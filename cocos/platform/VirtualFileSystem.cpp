#include "VirtualFileSystem.h"
#include "LeakDump.h"
#include "CCFileUtils.h"

#include <stack>

#include "base/CCData.h"
#include "base/ccMacros.h"
#include "base/CCDirector.h"
#include "platform/CCSAXParser.h"
#include "base/ccUtils.h"

#include "tinyxml2.h"
#ifdef MINIZIP_FROM_SYSTEM
#include <minizip/unzip.h>
#else // from our embedded sources
#include "unzip.h"
#endif
#include <sys/stat.h>


NS_CC_BEGIN

VirtualFileSystem* VirtualFileSystem::ms_instance;

VirtualFileSystem* VirtualFileSystem::getInstance()
{
	if (nullptr == VirtualFileSystem::ms_instance)
		VirtualFileSystem::ms_instance = New VirtualFileSystem;
	return VirtualFileSystem::ms_instance;
}

void VirtualFileSystem::destroyInstance()
{
	Delete VirtualFileSystem::ms_instance;
}

bool VirtualFileSystem::addZipFile(const std::string& zipPath)
{
	
	return false;
}

Data VirtualFileSystem::getDataFromFile(const std::string& path)
{
	return FileUtils::getInstance()->getDataFromFile(path);



#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)

#else
	
#endif

}

NS_CC_END