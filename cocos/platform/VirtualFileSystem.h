#ifndef _CC_VFS_H_
#define _CC_VFS_H_

#include <string>
#include <vector>
#include <unordered_map>

#include "platform/CCPlatformMacros.h"
#include "base/ccTypes.h"
#include "base/CCValue.h"
#include "base/CCData.h"

NS_CC_BEGIN

class CC_DLL VirtualFileSystem
{
public:
	static VirtualFileSystem* getInstance();
	static void destroyInstance();

	bool addZipFile(const std::string& zipPath);                //! initialise virtual file system with zip file.
	Data getDataFromFile(const std::string& path);	

protected:

	static VirtualFileSystem* ms_instance;
};


NS_CC_END

#endif