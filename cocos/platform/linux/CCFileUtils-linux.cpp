/****************************************************************************
Copyright (c) 2011      Laschweinski
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
#if CC_TARGET_PLATFORM == CC_PLATFORM_LINUX

#include "VirtualFileSystem-linux.h"
#include "CCApplication-linux.h"
#include "platform/CCCommon.h"
#include "base/ccMacros.h"
#include "deprecated/CCString.h"
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

#ifndef CC_RESOURCE_FOLDER_LINUX
#define CC_RESOURCE_FOLDER_LINUX ("/Resources/")
#endif

using namespace std;

NS_CC_BEGIN

VirtualFileSystem* VirtualFileSystem::getInstance()
{
    if (s_sharedVirtualFileSystem == NULL)
    {
        s_sharedVirtualFileSystem = new VirtualFileSystemLinux();
        if(!s_sharedVirtualFileSystem->init())
        {
          delete s_sharedVirtualFileSystem;
          s_sharedVirtualFileSystem = NULL;
          CCLOG("ERROR: Could not init VirtualFileSystemLinux");
        }
    }
    return s_sharedVirtualFileSystem;
}

VirtualFileSystemLinux::VirtualFileSystemLinux()
{}

bool VirtualFileSystemLinux::init()
{
    // get application path
    char fullpath[256] = {0};
    ssize_t length = readlink("/proc/self/exe", fullpath, sizeof(fullpath)-1);

    if (length <= 0) {
        return false;
    }

    fullpath[length] = '\0';
    std::string appPath = fullpath;
    _defaultResRootPath = appPath.substr(0, appPath.find_last_of("/"));
    _defaultResRootPath += CC_RESOURCE_FOLDER_LINUX;

    // Set writable path to $XDG_CONFIG_HOME or ~/.config/<app name>/ if $XDG_CONFIG_HOME not exists.
    const char* xdg_config_path = getenv("XDG_CONFIG_HOME");
    std::string xdgConfigPath;
    if (xdg_config_path == NULL) {
        xdgConfigPath = getenv("HOME");
        xdgConfigPath += "/.config";
    } else {
        xdgConfigPath  = xdg_config_path;
    }
    _writablePath = xdgConfigPath;
    _writablePath += appPath.substr(appPath.find_last_of("/"));
    _writablePath += "/";

    return VirtualFileSystem::init();
}

string VirtualFileSystemLinux::getWritablePath() const
{
    struct stat st;
    stat(_writablePath.c_str(), &st);
    if (!S_ISDIR(st.st_mode)) {
        mkdir(_writablePath.c_str(), 0744);
    }

    return _writablePath;
}

bool VirtualFileSystemLinux::isFileExistInternal(const std::string& strFilePath) const
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

    struct stat sts;
    return (stat(strPath.c_str(), &sts) != -1) ? true : false;
}

NS_CC_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_LINUX
