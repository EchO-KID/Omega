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

	bool init();

	ValueMap getValueMapFromFile(const std::string& file);

	std::string fullPathFromRelativeFile(const std::string &filename, const std::string &relativeFile);
	const std::unordered_map<std::string, std::string>& getFullPathCache() const { return _fullPathCache; }
	const std::vector<std::string>& getSearchResolutionsOrder() const;
	const std::vector<std::string>& getSearchPaths() const;
	
	void purgeCachedEntries();
	void setWritablePath(const std::string& writablePath);
	void setDefaultResourceRootPath(const std::string& path);
	void setSearchPaths(const std::vector<std::string>& searchPaths);
	void addSearchPath(const std::string &searchpath, const bool front);

	virtual std::string getWritablePath() const = 0;

	virtual std::string getSuitableFOpen(const std::string& filenameUtf8) const;   //! 是否要做转换
	std::string getFileExtension(const std::string& filePath) const;
	bool isFileExist(const std::string& filename) const;                   //! 要判断是否在zip里面
	virtual bool isAbsolutePath(const std::string& path) const;            //! 存在于zip即绝对路径。
	std::string fullPathForFilename(const std::string &filename) const;    //! 要判断是否在zip里面
		
	bool addZipFile(const std::string& zipPath);            //! initialise virtual file system with zip file.
	Data getFileData(const std::string& path, bool forString=false);	            //! 未实现

protected:
	std::string getPathForFilename(const std::string& filename, const std::string& resolutionDirectory, const std::string& searchPath) const;
	std::string getFullPathForDirectoryAndFilename(const std::string& directory, const std::string& filename) const;
	/**
	*  Checks whether a file exists without considering search paths and resolution orders.
	*  @param filename The file (with absolute path) to look up for
	*  @return Returns true if the file found at the given absolute path, otherwise returns false
	*/
	virtual bool isFileExistInternal(const std::string& filename) const = 0;

protected:

	/**
	*  The vector contains resolution folders.
	*  The lower index of the element in this vector, the higher priority for this resolution directory.
	*/
	std::vector<std::string> _searchResolutionsOrderArray;

	/**
	* The vector contains search paths.
	* The lower index of the element in this vector, the higher priority for this search path.
	*/
	std::vector<std::string> _searchPathArray;

	/**
	*  The default root path of resources.
	*  If the default root path of resources needs to be changed, do it in the `init` method of FileUtils's subclass.
	*  For instance:
	*  On Android, the default root path of resources will be assigned with "assets/" in FileUtilsAndroid::init().
	*  Similarly on Blackberry, we assign "app/native/Resources/" to this variable in FileUtilsBlackberry::init().
	*/
	std::string _defaultResRootPath;

	/**
	*  The full path cache. When a file is found, it will be added into this cache.
	*  This variable is used for improving the performance of file search.
	*/
	mutable std::unordered_map<std::string, std::string> _fullPathCache;

	/**
	* Writable path.
	*/
	std::string _writablePath;
protected:

	static VirtualFileSystem* ms_instance;
};


NS_CC_END

#endif