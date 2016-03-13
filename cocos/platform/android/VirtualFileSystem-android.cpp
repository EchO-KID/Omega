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
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

#include "VirtualFileSystem-android.h"
#include "platform/CCCommon.h"
#include "platform/LeakDump.h"
#include "jni/Java_org_cocos2dx_lib_Cocos2dxHelper.h"
#include <stdlib.h>
#include <sys/stat.h>

#define  LOG_TAG    "VirtualFileSystem-android.cpp"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

using namespace std;

NS_CC_BEGIN

AAssetManager* VirtualFileSystemAndroid::assetmanager = nullptr;

void VirtualFileSystemAndroid::setassetmanager(AAssetManager* a) {
    if (nullptr == a) {
        LOGD("setassetmanager : received unexpected nullptr parameter");
        return;
    }

    cocos2d::VirtualFileSystemAndroid::assetmanager = a;
}


VirtualFileSystem* VirtualFileSystem::getInstance()
{
    if (VirtualFileSystem::ms_instance == nullptr)
    {
        VirtualFileSystem::ms_instance = New VirtualFileSystemAndroid();
        if (!VirtualFileSystem::ms_instance->init())
        {
            Delete VirtualFileSystem::ms_instance;
            VirtualFileSystem::ms_instance = nullptr;
          CCLOG("ERROR: Could not init CCVirtualFileSystemWin32");
        }
    }
    return VirtualFileSystem::ms_instance;
}

VirtualFileSystemAndroid::VirtualFileSystemAndroid()
{
}

VirtualFileSystemAndroid::~VirtualFileSystemAndroid()
{
}

bool VirtualFileSystemAndroid::init()
{
    _defaultResRootPath = "assets/";
    return VirtualFileSystem::init();
}

bool VirtualFileSystemAndroid::isFileExistInternal(const std::string& strFilePath) const
{
    if (strFilePath.empty())
    {
        return false;
    }

    if (cocosplay::isEnabled() && !cocosplay::isDemo())
    {
        return cocosplay::fileExists(strFilePath);
    }

    bool bFound = false;

    // Check whether file exists in apk.
    if (strFilePath[0] != '/')
    {
        const char* s = strFilePath.c_str();

        // Found "assets/" at the beginning of the path and we don't want it
        if (strFilePath.find(_defaultResRootPath) == 0) s += strlen("assets/");

        if (VirtualFileSystemAndroid::assetmanager) {
            AAsset* aa = AAssetManager_open(VirtualFileSystemAndroid::assetmanager, s, AASSET_MODE_UNKNOWN);
            if (aa)
            {
                bFound = true;
                AAsset_close(aa);
            } else {
                // CCLOG("[AssetManager] ... in APK %s, found = false!", strFilePath.c_str());
            }
        }
    }
    else
    {
        FILE *fp = fopen(strFilePath.c_str(), "r");
        if(fp)
        {
            bFound = true;
            fclose(fp);
        }
    }
    return bFound;
}

bool VirtualFileSystemAndroid::isDirectoryExistInternal(const std::string& dirPath) const
{
    if (dirPath.empty())
    {
        return false;
    }

    const char* s = dirPath.c_str();
    bool startWithAssets = (dirPath.find("assets/") == 0);
    int lenOfAssets = 7;

    std::string tmpStr;
    if (cocosplay::isEnabled() && !cocosplay::isDemo())
    {
        // redirect assets/*** path to cocosplay resource dir
        tmpStr.append(_defaultResRootPath);
        if ('/' != tmpStr[tmpStr.length() - 1])
        {
            tmpStr += '/';
        }
        tmpStr.append(s + lenOfAssets);
    }

    // find absolute path in flash memory
    if (s[0] == '/')
    {
        CCLOG("find in flash memory dirPath(%s)", s);
        struct stat st;
        if (stat(s, &st) == 0)
        {
            return S_ISDIR(st.st_mode);
        }
    }

    // find it in apk's assets dir
    // Found "assets/" at the beginning of the path and we don't want it
    CCLOG("find in apk dirPath(%s)", s);
    if (startWithAssets)
    {
        s += lenOfAssets;
    }
    if (VirtualFileSystemAndroid::assetmanager)
    {
        AAssetDir* aa = AAssetManager_openDir(VirtualFileSystemAndroid::assetmanager, s);
        if (aa && AAssetDir_getNextFileName(aa))
        {
            AAssetDir_close(aa);
            return true;
        }
    }
    return false;
}

bool VirtualFileSystemAndroid::isAbsolutePath(const std::string& strPath) const
{
    if ( VirtualFileSystem::isAbsolutePath(strPath) )
        return true;

    // On Android, there are two situations for full path.
    // 1) Files in APK, e.g. assets/path/path/file.png
    // 2) Files not in APK, e.g. /data/data/org.cocos2dx.hellocpp/cache/path/path/file.png, or /sdcard/path/path/file.png.
    // So these two situations need to be checked on Android.
    if (strPath[0] == '/' || strPath.find(_defaultResRootPath) == 0)
    {
        return true;
    }
    return false;
}

Data VirtualFileSystemAndroid::getData(const std::string& filename, bool forString)
{
    if (filename.empty())
    {
        return Data::Null;
    }

    unsigned char* data = nullptr;
    ssize_t size = 0;
    string fullPath = fullPathForFilename(filename);
    cocosplay::updateAssets(fullPath);

    if (fullPath[0] != '/')
    {
        string relativePath = string();

        size_t position = fullPath.find("assets/");
        if (0 == position) {
            // "assets/" is at the beginning of the path and we don't want it
            relativePath += fullPath.substr(strlen("assets/"));
        } else {
            relativePath += fullPath;
        }
        CCLOGINFO("relative path = %s", relativePath.c_str());

        if (nullptr == FileUtilsAndroid::assetmanager) {
            LOGD("... FileUtilsAndroid::assetmanager is nullptr");
            return Data::Null;
        }

        // read asset data
        AAsset* asset =
            AAssetManager_open(FileUtilsAndroid::assetmanager,
                               relativePath.c_str(),
                               AASSET_MODE_UNKNOWN);
        if (nullptr == asset) {
            LOGD("asset is nullptr");
            return Data::Null;
        }

        off_t fileSize = AAsset_getLength(asset);

        if (forString)
        {
            data = (unsigned char*) malloc(fileSize + 1);
            data[fileSize] = '\0';
        }
        else
        {
            data = (unsigned char*) malloc(fileSize);
        }

        int bytesread = AAsset_read(asset, (void*)data, fileSize);
        size = bytesread;

        AAsset_close(asset);
    }
    else
    {
        do
        {
            // read rrom other path than user set it
            //CCLOG("GETTING FILE ABSOLUTE DATA: %s", filename);
            const char* mode = nullptr;
            if (forString)
                mode = "rt";
            else
                mode = "rb";

            FILE *fp = fopen(fullPath.c_str(), mode);
            CC_BREAK_IF(!fp);

            long fileSize;
            fseek(fp,0,SEEK_END);
            fileSize = ftell(fp);
            fseek(fp,0,SEEK_SET);
            if (forString)
            {
                data = (unsigned char*) malloc(fileSize + 1);
                data[fileSize] = '\0';
            }
            else
            {
                data = (unsigned char*) malloc(fileSize);
            }
            fileSize = fread(data,sizeof(unsigned char), fileSize,fp);
            fclose(fp);

            size = fileSize;
        } while (0);
    }

    Data ret;
    if (data == nullptr || size == 0)
    {
        std::string msg = "Get data from file(";
        msg.append(filename).append(") failed!");
        CCLOG("%s", msg.c_str());
    }
    else
    {
        ret.fastSet(data, size);
        cocosplay::notifyFileLoaded(fullPath);
    }

    return ret;
}


string VirtualFileSystemAndroid::getWritablePath() const
{
    // Fix for Nexus 10 (Android 4.2 multi-user environment)
    // the path is retrieved through Java Context.getCacheDir() method
    string dir("");
    string tmp = getFileDirectoryJNI();

    if (tmp.length() > 0)
    {
        dir.append(tmp).append("/");

        return dir;
    }
    else
    {
        return "";
    }
}

NS_CC_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
