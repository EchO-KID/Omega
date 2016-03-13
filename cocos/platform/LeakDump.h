#ifndef LeakDump_H
#define LeakDump_H

#define LeakDump

#ifdef LeakDump


#include <cstdlib>
#include <cstdio>
#include "platform/CCPlatformMacros.h"

struct  KAllocInfo
{
	int			line;
	const char* file;
	size_t		bytes;
	void *		ptr;
	bool        bFree;
	bool        bArray;

	KAllocInfo();
	KAllocInfo(void* ptr, const char* file, int line, size_t bytes, bool bArray);
};

class CC_DLL KAllocRegistry
{
private:
	static const unsigned maxAllocs = 40000;

private:
	KAllocRegistry();

public:
	static KAllocRegistry& getInstance();

	~KAllocRegistry();

	void allocate(KAllocInfo info);
	void deallocate(void* ptr, bool bArray);
	void reset();
	void dump(const char* path);

private:
	KAllocInfo m_allocs[maxAllocs];
	int		   m_allocCount;
	size_t	   m_allocBytes;
	int	       m_nonMonitoredCount;
	size_t     m_nonMonitoredBytes;
};

//if an allocation ocurrs in a file where "leaks_dumper.h" is not included
//this operator new is called and file and line will be unknown
inline void * operator new (size_t bytes){
	void *ptr= malloc(bytes);
	KAllocRegistry::getInstance().allocate(KAllocInfo(ptr, "unknown", 0, bytes, false));
	return ptr;
}

inline void operator delete(void *ptr){
	KAllocRegistry::getInstance().deallocate(ptr, false);
	free(ptr);
}

inline void * operator new[](size_t bytes){
	void *ptr= malloc(bytes);
	KAllocRegistry::getInstance().allocate(KAllocInfo(ptr, "unknown", 0, bytes, true));
	return ptr;
}

inline void operator delete [](void *ptr){
	KAllocRegistry::getInstance().deallocate(ptr, true);
	free(ptr);
}

//if an allocation ocurrs in a file where "leaks_dumper.h" is included 
//this operator new is called and file and line will be known
inline void * operator new (size_t bytes, char* file, int line){
	void *ptr= malloc(bytes);
	KAllocRegistry::getInstance().allocate(KAllocInfo(ptr, file, line, bytes, false));
	return ptr;
}

inline void operator delete(void *ptr, char* file, int line){
	KAllocRegistry::getInstance().deallocate(ptr, false);
	free(ptr);
}

inline void * operator new[](size_t bytes, char* file, int line){
	void *ptr= malloc(bytes);
	KAllocRegistry::getInstance().allocate(KAllocInfo(ptr, file, line, bytes, true));
	return ptr;
}

inline void operator delete [](void *ptr, char* file, int line){
	KAllocRegistry::getInstance().deallocate(ptr, true);
	free(ptr);
}

#define New new(__FILE__, __LINE__)
#define Delete delete

#endif

#endif