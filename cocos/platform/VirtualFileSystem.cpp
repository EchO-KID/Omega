#include "VirtualFileSystem.h"
#include "LeakDump.h"
#include "VirtualFileSystem.h"

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


// Implement DictMaker

#if (CC_TARGET_PLATFORM != CC_PLATFORM_IOS) && (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)

typedef enum
{
	SAX_NONE = 0,
	SAX_KEY,
	SAX_DICT,
	SAX_INT,
	SAX_REAL,
	SAX_STRING,
	SAX_ARRAY
}SAXState;

typedef enum
{
	SAX_RESULT_NONE = 0,
	SAX_RESULT_DICT,
	SAX_RESULT_ARRAY
}SAXResult;

class DictMaker : public SAXDelegator
{
public:
	SAXResult _resultType;
	ValueMap _rootDict;
	ValueVector _rootArray;

	std::string _curKey;   ///< parsed key
	std::string _curValue; // parsed value
	SAXState _state;

	ValueMap*  _curDict;
	ValueVector* _curArray;

	std::stack<ValueMap*> _dictStack;
	std::stack<ValueVector*> _arrayStack;
	std::stack<SAXState>  _stateStack;

public:
	DictMaker()
		: _resultType(SAX_RESULT_NONE)
	{
	}

	~DictMaker()
	{
	}


	ValueMap dictionaryWithDataOfFile(const char* filedata, int filesize)
	{
		_resultType = SAX_RESULT_DICT;
		SAXParser parser;

		CCASSERT(parser.init("UTF-8"), "The file format isn't UTF-8");
		parser.setDelegator(this);
		parser.parse(filedata, filesize);
		return _rootDict;
	}


	void startElement(void *ctx, const char *name, const char **atts)
	{
		CC_UNUSED_PARAM(ctx);
		CC_UNUSED_PARAM(atts);
		const std::string sName(name);
		if (sName == "dict")
		{
			if (_resultType == SAX_RESULT_DICT && _rootDict.empty())
			{
				_curDict = &_rootDict;
			}

			_state = SAX_DICT;

			SAXState preState = SAX_NONE;
			if (!_stateStack.empty())
			{
				preState = _stateStack.top();
			}

			if (SAX_ARRAY == preState)
			{
				// add a new dictionary into the array
				_curArray->push_back(Value(ValueMap()));
				_curDict = &(_curArray->rbegin())->asValueMap();
			}
			else if (SAX_DICT == preState)
			{
				// add a new dictionary into the pre dictionary
				CCASSERT(!_dictStack.empty(), "The state is wrong!");
				ValueMap* preDict = _dictStack.top();
				(*preDict)[_curKey] = Value(ValueMap());
				_curDict = &(*preDict)[_curKey].asValueMap();
			}

			// record the dict state
			_stateStack.push(_state);
			_dictStack.push(_curDict);
		}
		else if (sName == "key")
		{
			_state = SAX_KEY;
		}
		else if (sName == "integer")
		{
			_state = SAX_INT;
		}
		else if (sName == "real")
		{
			_state = SAX_REAL;
		}
		else if (sName == "string")
		{
			_state = SAX_STRING;
		}
		else if (sName == "array")
		{
			_state = SAX_ARRAY;

			if (_resultType == SAX_RESULT_ARRAY && _rootArray.empty())
			{
				_curArray = &_rootArray;
			}
			SAXState preState = SAX_NONE;
			if (!_stateStack.empty())
			{
				preState = _stateStack.top();
			}

			if (preState == SAX_DICT)
			{
				(*_curDict)[_curKey] = Value(ValueVector());
				_curArray = &(*_curDict)[_curKey].asValueVector();
			}
			else if (preState == SAX_ARRAY)
			{
				CCASSERT(!_arrayStack.empty(), "The state is wrong!");
				ValueVector* preArray = _arrayStack.top();
				preArray->push_back(Value(ValueVector()));
				_curArray = &(_curArray->rbegin())->asValueVector();
			}
			// record the array state
			_stateStack.push(_state);
			_arrayStack.push(_curArray);
		}
		else
		{
			_state = SAX_NONE;
		}
	}

	void endElement(void *ctx, const char *name)
	{
		CC_UNUSED_PARAM(ctx);
		SAXState curState = _stateStack.empty() ? SAX_DICT : _stateStack.top();
		const std::string sName((char*)name);
		if (sName == "dict")
		{
			_stateStack.pop();
			_dictStack.pop();
			if (!_dictStack.empty())
			{
				_curDict = _dictStack.top();
			}
		}
		else if (sName == "array")
		{
			_stateStack.pop();
			_arrayStack.pop();
			if (!_arrayStack.empty())
			{
				_curArray = _arrayStack.top();
			}
		}
		else if (sName == "true")
		{
			if (SAX_ARRAY == curState)
			{
				_curArray->push_back(Value(true));
			}
			else if (SAX_DICT == curState)
			{
				(*_curDict)[_curKey] = Value(true);
			}
		}
		else if (sName == "false")
		{
			if (SAX_ARRAY == curState)
			{
				_curArray->push_back(Value(false));
			}
			else if (SAX_DICT == curState)
			{
				(*_curDict)[_curKey] = Value(false);
			}
		}
		else if (sName == "string" || sName == "integer" || sName == "real")
		{
			if (SAX_ARRAY == curState)
			{
				if (sName == "string")
					_curArray->push_back(Value(_curValue));
				else if (sName == "integer")
					_curArray->push_back(Value(atoi(_curValue.c_str())));
				else
					_curArray->push_back(Value(utils::atof(_curValue.c_str())));
			}
			else if (SAX_DICT == curState)
			{
				if (sName == "string")
					(*_curDict)[_curKey] = Value(_curValue);
				else if (sName == "integer")
					(*_curDict)[_curKey] = Value(atoi(_curValue.c_str()));
				else
					(*_curDict)[_curKey] = Value(utils::atof(_curValue.c_str()));
			}

			_curValue.clear();
		}

		_state = SAX_NONE;
	}

	void textHandler(void *ctx, const char *ch, int len)
	{
		CC_UNUSED_PARAM(ctx);
		if (_state == SAX_NONE)
		{
			return;
		}

		SAXState curState = _stateStack.empty() ? SAX_DICT : _stateStack.top();
		const std::string text = std::string((char*)ch, len);

		switch (_state)
		{
		case SAX_KEY:
			_curKey = text;
			break;
		case SAX_INT:
		case SAX_REAL:
		case SAX_STRING:
		{
						   if (curState == SAX_DICT)
						   {
							   CCASSERT(!_curKey.empty(), "key not found : <integer/real>");
						   }

						   _curValue.append(text);
		}
			break;
		default:
			break;
		}
	}
};
#else


#endif /* (CC_TARGET_PLATFORM != CC_PLATFORM_IOS) && (CC_TARGET_PLATFORM != CC_PLATFORM_MAC) */


VirtualFileSystem* VirtualFileSystem::ms_instance;

//VirtualFileSystem* VirtualFileSystem::getInstance()
//{
//
////	if (nullptr == VirtualFileSystem::ms_instance)
////		VirtualFileSystem::ms_instance = New VirtualFileSystem;
////	return VirtualFileSystem::ms_instance;
//}

void VirtualFileSystem::destroyInstance()
{
	Delete VirtualFileSystem::ms_instance;
}

bool VirtualFileSystem::init()
{
	_searchPathArray.push_back(_defaultResRootPath);
	_searchResolutionsOrderArray.push_back("");
	return true;
}

ValueMap VirtualFileSystem::getValueMapFromFile(const std::string& file)
{
	Data data = getFileData(file);
	if (!data.isNull())
	{
		DictMaker tMaker;
		return tMaker.dictionaryWithDataOfFile((const char*)data.getBytes(), data.getSize());
	}
}

std::string VirtualFileSystem::fullPathFromRelativeFile(const std::string &filename, const std::string &relativeFile)
{
	return relativeFile.substr(0, relativeFile.rfind('/') + 1) + filename;
}

const std::vector<std::string>& VirtualFileSystem::getSearchResolutionsOrder() const
{
	return _searchResolutionsOrderArray;
}

const std::vector<std::string>& VirtualFileSystem::getSearchPaths() const
{
	return _searchPathArray;
}

void VirtualFileSystem::purgeCachedEntries()
{
	_fullPathCache.clear();
}

void VirtualFileSystem::setWritablePath(const std::string& writablePath)
{
	_writablePath = writablePath;
}

void VirtualFileSystem::setDefaultResourceRootPath(const std::string& path)
{
	_defaultResRootPath = path;
}

void VirtualFileSystem::setSearchPaths(const std::vector<std::string>& searchPaths)
{
	bool existDefaultRootPath = false;

	_fullPathCache.clear();
	_searchPathArray.clear();
	for (const auto& iter : searchPaths)
	{
		std::string prefix;
		std::string path;

		if (!isAbsolutePath(iter))
		{ // Not an absolute path
			prefix = _defaultResRootPath;
		}
		path = prefix + (iter);
		if (!path.empty() && path[path.length() - 1] != '/')
		{
			path += "/";
		}
		if (!existDefaultRootPath && path == _defaultResRootPath)
		{
			existDefaultRootPath = true;
		}
		_searchPathArray.push_back(path);
	}

	if (!existDefaultRootPath)
	{
		//CCLOG("Default root path doesn't exist, adding it.");
		_searchPathArray.push_back(_defaultResRootPath);
	}
}

void VirtualFileSystem::addSearchPath(const std::string &searchpath, const bool front)
{
	std::string prefix;
	if (!isAbsolutePath(searchpath))
		prefix = _defaultResRootPath;

	std::string path = prefix + searchpath;
	if (!path.empty() && path[path.length() - 1] != '/')
	{
		path += "/";
	}
	if (front) {
		_searchPathArray.insert(_searchPathArray.begin(), path);
	}
	else {
		_searchPathArray.push_back(path);
	}
}

std::string VirtualFileSystem::getSuitableFOpen(const std::string& filenameUtf8) const   //! 是否要做转换
{
	return filenameUtf8;
}

bool VirtualFileSystem::isFileExist(const std::string& filename) const
{
	if (isAbsolutePath(filename))
	{
		return isFileExistInternal(filename);
	}
	else
	{
		std::string fullpath = fullPathForFilename(filename);
		if (fullpath.empty())
			return false;
		else
			return true;
	}
}

std::string VirtualFileSystem::getFileExtension(const std::string& filePath) const
{
	std::string fileExtension;
	size_t pos = filePath.find_last_of('.');
	if (pos != std::string::npos)
	{
		fileExtension = filePath.substr(pos, filePath.length());

		std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), ::tolower);
	}

	return fileExtension;
}


bool VirtualFileSystem::isAbsolutePath(const std::string& path) const
{
	return (path[0] == '/');
}


std::string VirtualFileSystem::fullPathForFilename(const std::string &filename) const
{
	if (filename.empty())
	{
		return "";
	}

	if (isAbsolutePath(filename))
	{
		return filename;
	}

	// Already Cached ?
	auto cacheIter = _fullPathCache.find(filename);
	if (cacheIter != _fullPathCache.end())
	{
		return cacheIter->second;
	}

	std::string fullpath;

	for (const auto& searchIt : _searchPathArray)
	{
		for (const auto& resolutionIt : _searchResolutionsOrderArray)
		{
			fullpath = this->getPathForFilename(filename, resolutionIt, searchIt);

			if (!fullpath.empty())
			{
				// Using the filename passed in as key.
				_fullPathCache.insert(std::make_pair(filename, fullpath));
				return fullpath;
			}

		}
	}

	CCLOG("No file found at %s. Possible missing file.", filename.c_str());

	// The file wasn't found, return empty string.
	return "";
}

bool VirtualFileSystem::addZipFile(const std::string& zipPath)
{
	
	return false;
}

Data VirtualFileSystem::getFileData(const std::string& filename, bool forString)
{
	if (filename.empty())
	{
		return Data::Null;
	}

	Data ret;
	unsigned char* buffer = nullptr;
	size_t size = 0;
	size_t readsize;
	const char* mode = nullptr;

	if (forString)
		mode = "rt";
	else
		mode = "rb";

	do
	{
		// Read the file from hardware
		std::string fullPath = fullPathForFilename(filename);
		FILE *fp = fopen(getSuitableFOpen(fullPath).c_str(), mode);
		CC_BREAK_IF(!fp);
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		if (forString)
		{
			buffer = (unsigned char*)malloc(sizeof(unsigned char)* (size + 1));
			buffer[size] = '\0';
		}
		else
		{
			buffer = (unsigned char*)malloc(sizeof(unsigned char)* size);
		}

		readsize = fread(buffer, sizeof(unsigned char), size, fp);
		fclose(fp);

		if (forString && readsize < size)
		{
			buffer[readsize] = '\0';
		}
	} while (0);

	if (nullptr == buffer || 0 == readsize)
	{
		CCLOG("Get data from file %s failed", filename.c_str());
	}
	else
	{
		ret.fastSet(buffer, readsize);
	}

	return ret;


#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)

#else
	
#endif

}

std::string VirtualFileSystem::getPathForFilename(const std::string& filename, const std::string& resolutionDirectory, const std::string& searchPath) const
{
	std::string file = filename;
	std::string file_path = "";
	size_t pos = filename.find_last_of("/");
	if (pos != std::string::npos)
	{
		file_path = filename.substr(0, pos + 1);
		file = filename.substr(pos + 1);
	}

	// searchPath + file_path + resourceDirectory
	std::string path = searchPath;
	path += file_path;
	path += resolutionDirectory;

	path = getFullPathForDirectoryAndFilename(path, file);

	//CCLOG("getPathForFilename, fullPath = %s", path.c_str());
	return path;
}

std::string VirtualFileSystem::getFullPathForDirectoryAndFilename(const std::string& directory, const std::string& filename) const
{
	// get directory+filename, safely adding '/' as necessary
	std::string ret = directory;
	if (directory.size() && directory[directory.size() - 1] != '/'){
		ret += '/';
	}
	ret += filename;

	// if the file doesn't exist, return an empty string
	if (!isFileExistInternal(ret)) {
		ret = "";
	}
	return ret;
}



NS_CC_END