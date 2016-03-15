/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2014 Chukong Technologies Inc.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include "platform/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32

#include "VirtualFileSystem-win32.h"
#include "platform/CCCommon.h"
#include "platform/LeakDump.h"
#include <Shlobj.h>
#include <cstdlib>
#include <regex>

using namespace std;

NS_CC_BEGIN

#define CC_MAX_PATH  512

// The root path of resources, the character encoding is UTF-8.
// UTF-8 is the only encoding supported by cocos2d-x API.
static std::string s_resourcePath = "";

// D:\aaa\bbb\ccc\ddd\abc.txt --> D:/aaa/bbb/ccc/ddd/abc.txt
static inline std::string convertPathFormatToUnixStyle(const std::string& path)
{
    std::string ret = path;
    int len = ret.length();
    for (int i = 0; i < len; ++i)
    {
        if (ret[i] == '\\')
        {
            ret[i] = '/';
        }
    }
    return ret;
}

static std::wstring StringUtf8ToWideChar(const std::string& strUtf8)
{
    std::wstring ret;
    if (!strUtf8.empty())
    {
        int nNum = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, nullptr, 0);
        if (nNum)
        {
            WCHAR* wideCharString = new WCHAR[nNum + 1];
            wideCharString[0] = 0;

            nNum = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, wideCharString, nNum + 1);

            ret = wideCharString;
            delete[] wideCharString;
        }
        else
        {
            CCLOG("Wrong convert to WideChar code:0x%x", GetLastError());
        }
    }
    return ret;
}

static std::string StringWideCharToUtf8(const std::wstring& strWideChar)
{
    std::string ret;
    if (!strWideChar.empty())
    {
        int nNum = WideCharToMultiByte(CP_UTF8, 0, strWideChar.c_str(), -1, nullptr, 0, nullptr, FALSE);
        if (nNum)
        {
            char* utf8String = new char[nNum + 1];
            utf8String[0] = 0;

            nNum = WideCharToMultiByte(CP_UTF8, 0, strWideChar.c_str(), -1, utf8String, nNum + 1, nullptr, FALSE);

            ret = utf8String;
            delete[] utf8String;
        }
        else
        {
            CCLOG("Wrong convert to Utf8 code:0x%x", GetLastError());
        }
    }

    return ret;
}

static std::string UTF8StringToMultiByte(const std::string& strUtf8)
{
    std::string ret;
    if (!strUtf8.empty())
    {
        std::wstring strWideChar = StringUtf8ToWideChar(strUtf8);
        int nNum = WideCharToMultiByte(CP_ACP, 0, strWideChar.c_str(), -1, nullptr, 0, nullptr, FALSE);
        if (nNum)
        {
            char* ansiString = new char[nNum + 1];
            ansiString[0] = 0;

            nNum = WideCharToMultiByte(CP_ACP, 0, strWideChar.c_str(), -1, ansiString, nNum + 1, nullptr, FALSE);

            ret = ansiString;
            delete[] ansiString;
        }
        else
        {
            CCLOG("Wrong convert to Ansi code:0x%x", GetLastError());
        }
    }

    return ret;
}

static void _checkPath()
{
    if (s_resourcePath.empty())
    {
        WCHAR *pUtf16ExePath = nullptr;
        _get_wpgmptr(&pUtf16ExePath);

        // We need only directory part without exe
        WCHAR *pUtf16DirEnd = wcsrchr(pUtf16ExePath, L'\\');

        char utf8ExeDir[CC_MAX_PATH] = { 0 };
        int nNum = WideCharToMultiByte(CP_UTF8, 0, pUtf16ExePath, pUtf16DirEnd-pUtf16ExePath+1, utf8ExeDir, sizeof(utf8ExeDir), nullptr, nullptr);

        s_resourcePath = convertPathFormatToUnixStyle(utf8ExeDir);
    }
}

VirtualFileSystem* VirtualFileSystem::getInstance()
{
	if (VirtualFileSystem::ms_instance == nullptr)
    {
		VirtualFileSystem::ms_instance = New VirtualFileSystemWin32();
		if (!VirtualFileSystem::ms_instance->init())
        {
			Delete VirtualFileSystem::ms_instance;
			VirtualFileSystem::ms_instance = nullptr;
          CCLOG("ERROR: Could not init VirtualFileSystemWin32");
        }
    }
	return VirtualFileSystem::ms_instance;
}

VirtualFileSystemWin32::VirtualFileSystemWin32()
{
}

bool VirtualFileSystemWin32::init()
{
    _checkPath();
    _defaultResRootPath = s_resourcePath;
    return VirtualFileSystem::init();
}

bool VirtualFileSystemWin32::isDirectoryExistInternal(const std::string& dirPath) const
{
    unsigned long fAttrib = GetFileAttributes(StringUtf8ToWideChar(dirPath).c_str());
    if (fAttrib != INVALID_FILE_ATTRIBUTES &&
        (fAttrib & FILE_ATTRIBUTE_DIRECTORY))
    {
        return true;
    }
    return false;
}

std::string VirtualFileSystemWin32::getSuitableFOpen(const std::string& filenameUtf8) const
{
    return UTF8StringToMultiByte(filenameUtf8);
}

long VirtualFileSystemWin32::getFileSize(const std::string &filepath)
{
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesEx(StringUtf8ToWideChar(filepath).c_str(), GetFileExInfoStandard, &fad))
    {
        return 0; // error condition, could call GetLastError to find out more
    }
    LARGE_INTEGER size;
    size.HighPart = fad.nFileSizeHigh;
    size.LowPart = fad.nFileSizeLow;
    return (long)size.QuadPart;
}

bool VirtualFileSystemWin32::isFileExistInternal(const std::string& strFilePath) const
{
    if (strFilePath.empty())
    {
        return false;
    }

    std::string strPath = strFilePath;
    if (!isAbsolutePath(strPath))
    { // Not absolute path, add the default root path at the beginning.
        strPath.insert(0, _defaultResRootPath);
    }

    DWORD attr = GetFileAttributesW(StringUtf8ToWideChar(strPath).c_str());
    if(attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY))
        return false;   //  not a file
    return true;
}

bool VirtualFileSystemWin32::isAbsolutePath(const std::string& strPath) const
{
	if (VirtualFileSystem::isAbsolutePath(strPath))
		return true;


    if (   (strPath.length() > 2
        && ( (strPath[0] >= 'a' && strPath[0] <= 'z') || (strPath[0] >= 'A' && strPath[0] <= 'Z') )
        && strPath[1] == ':') || (strPath[0] == '/' && strPath[1] == '/'))
    {
        return true;
    }
    return false;
}

// Because windows is case insensitive, so we should check the file names.
static bool checkFileName(const std::string& fullPath, const std::string& filename)
{
    std::string tmpPath=convertPathFormatToUnixStyle(fullPath);
    size_t len = tmpPath.length();
    size_t nl = filename.length();
    std::string realName;

    while (tmpPath.length() >= len - nl && tmpPath.length()>2)
    {
        //CCLOG("%s", tmpPath.c_str());
        WIN32_FIND_DATAA data;
        HANDLE h = FindFirstFileA(tmpPath.c_str(), &data);
        FindClose(h);
        if (h != INVALID_HANDLE_VALUE)
        {
            int fl = strlen(data.cFileName);
            if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                realName = "/" + realName;
            }
            realName = data.cFileName + realName;
            if (0 != strcmp(&tmpPath.c_str()[tmpPath.length() - fl], data.cFileName))
            {
                std::string msg = "File path error: \"";
                msg.append(filename).append("\" the real name is: ").append(realName);

                CCLOG("%s", msg.c_str());
                return false;
            }
        }
        else
        {
            break;
        }

        do
        {
            tmpPath = tmpPath.substr(0, tmpPath.rfind("/"));
        } while (tmpPath.back() == '.');
    }
    return true;
}

static Data getData(const std::string& filename, bool forString)
{
    if (filename.empty())
    {
        return Data::Null;
    }

    unsigned char *buffer = nullptr;

    size_t size = 0;
    do
    {
        // read the file from hardware
        std::string fullPath = VirtualFileSystem::getInstance()->fullPathForFilename(filename);

        // check if the filename uses correct case characters
        checkFileName(fullPath, filename);

        HANDLE fileHandle = ::CreateFile(StringUtf8ToWideChar(fullPath).c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, nullptr);
        CC_BREAK_IF(fileHandle == INVALID_HANDLE_VALUE);

        size = ::GetFileSize(fileHandle, nullptr);

        if (forString)
        {
            buffer = (unsigned char*) malloc(size + 1);
            buffer[size] = '\0';
        }
        else
        {
            buffer = (unsigned char*) malloc(size);
        }
        DWORD sizeRead = 0;
        BOOL successed = FALSE;
        successed = ::ReadFile(fileHandle, buffer, size, &sizeRead, nullptr);
        ::CloseHandle(fileHandle);

        if (!successed)
        {
            // should determine buffer value, or it will cause memory leak
            if (buffer)
            {
                free(buffer);
                buffer = nullptr;
            }
        }
    } while (0);

    Data ret;

    if (buffer == nullptr || size == 0)
    {
        std::string msg = "Get data from file(";
        // Gets error code.
        DWORD errorCode = ::GetLastError();
        char errorCodeBuffer[20] = {0};
        snprintf(errorCodeBuffer, sizeof(errorCodeBuffer), "%d", errorCode);

        msg = msg + filename + ") failed, error code is " + errorCodeBuffer;
        CCLOG("%s", msg.c_str());

        if (buffer)
            free(buffer);
    }
    else
    {
        ret.fastSet(buffer, size);
    }
    return ret;
}

std::string VirtualFileSystemWin32::getStringFromFile(const std::string& filename)
{
    Data data = getData(filename, true);
    if (data.isNull())
    {
        return "";
    }

    std::string ret((const char*)data.getBytes());
    return ret;
}

Data VirtualFileSystemWin32::getDataFromFile(const std::string& filename)
{
    return getData(filename, false);
}

std::string VirtualFileSystemWin32::getPathForFilename(const std::string& filename, const std::string& resolutionDirectory, const std::string& searchPath) const
{
    std::string unixFileName = convertPathFormatToUnixStyle(filename);
    std::string unixResolutionDirectory = convertPathFormatToUnixStyle(resolutionDirectory);
    std::string unixSearchPath = convertPathFormatToUnixStyle(searchPath);

    return VirtualFileSystem::getPathForFilename(unixFileName, unixResolutionDirectory, unixSearchPath);
}

std::string VirtualFileSystemWin32::getFullPathForDirectoryAndFilename(const std::string& strDirectory, const std::string& strFilename) const
{
    std::string unixDirectory = convertPathFormatToUnixStyle(strDirectory);
    std::string unixFilename = convertPathFormatToUnixStyle(strFilename);

    return VirtualFileSystem::getFullPathForDirectoryAndFilename(unixDirectory, unixFilename);
}

string VirtualFileSystemWin32::getWritablePath() const
{
    if (_writablePath.length())
    {
        return _writablePath;
    }

    // Get full path of executable, e.g. c:\Program Files (x86)\My Game Folder\MyGame.exe
    WCHAR full_path[CC_MAX_PATH + 1] = { 0 };
    ::GetModuleFileName(nullptr, full_path, CC_MAX_PATH + 1);

    // Debug app uses executable directory; Non-debug app uses local app data directory
//#ifndef _DEBUG
    // Get filename of executable only, e.g. MyGame.exe
    WCHAR *base_name = wcsrchr(full_path, '\\');
    wstring retPath;
    if(base_name)
    {
        WCHAR app_data_path[CC_MAX_PATH + 1];

        // Get local app data directory, e.g. C:\Documents and Settings\username\Local Settings\Application Data
        if (SUCCEEDED(SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, app_data_path)))
        {
            wstring ret(app_data_path);

            // Adding executable filename, e.g. C:\Documents and Settings\username\Local Settings\Application Data\MyGame.exe
            ret += base_name;

            // Remove ".exe" extension, e.g. C:\Documents and Settings\username\Local Settings\Application Data\MyGame
            ret = ret.substr(0, ret.rfind(L"."));

            ret += L"\\";

            // Create directory
            if (SUCCEEDED(SHCreateDirectoryEx(nullptr, ret.c_str(), nullptr)))
            {
                retPath = ret;
            }
        }
    }
    if (retPath.empty())
//#endif // not defined _DEBUG
    {
        // If fetching of local app data directory fails, use the executable one
        retPath = full_path;

        // remove xxx.exe
        retPath = retPath.substr(0, retPath.rfind(L"\\") + 1);
    }

    return convertPathFormatToUnixStyle(StringWideCharToUtf8(retPath));
}

bool VirtualFileSystemWin32::renameFile(const std::string &oldfullpath, const std::string& newfullpath)
{
    CCASSERT(!oldfullpath.empty(), "Invalid path");
    CCASSERT(!newfullpath.empty(), "Invalid path");

    std::wstring _wNew = StringUtf8ToWideChar(newfullpath);
    std::wstring _wOld = StringUtf8ToWideChar(oldfullpath);

    if (VirtualFileSystem::getInstance()->isFileExist(newfullpath))
    {
        if (!DeleteFile(_wNew.c_str()))
        {
            CCLOGERROR("Fail to delete file %s !Error code is 0x%x", newfullpath.c_str(), GetLastError());
        }
    }

    if (MoveFile(_wOld.c_str(), _wNew.c_str()))
    {
        return true;
    }
    else
    {
        CCLOGERROR("Fail to rename file %s to %s !Error code is 0x%x", oldfullpath.c_str(), newfullpath.c_str(), GetLastError());
        return false;
    }
}

bool VirtualFileSystemWin32::renameFile(const std::string &path, const std::string &oldname, const std::string &name)
{
    CCASSERT(!path.empty(), "Invalid path");
    std::string oldPath = path + oldname;
    std::string newPath = path + name;

    std::regex pat("\\/");
    std::string _old = std::regex_replace(oldPath, pat, "\\");
    std::string _new = std::regex_replace(newPath, pat, "\\");

    return renameFile(_old, _new);
}


bool VirtualFileSystemWin32::removeFile(const std::string &filepath)
{
    std::regex pat("\\/");
    std::string win32path = std::regex_replace(filepath, pat, "\\");

    if (DeleteFile(StringUtf8ToWideChar(win32path).c_str()))
    {
        return true;
    }
    else
    {
        CCLOGERROR("Fail remove file %s !Error code is 0x%x", filepath.c_str(), GetLastError());
        return false;
    }
}


NS_CC_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_WIN32

