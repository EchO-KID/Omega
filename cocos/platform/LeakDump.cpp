#include "LeakDump.h"

#ifdef LeakDump


KAllocInfo::KAllocInfo()
	:ptr(NULL)
	,file("")
	,line(-1)
	,bytes(-1)
	,bArray(false)
	,bFree(true)
{

}

KAllocInfo::KAllocInfo(void* ptr, const char* file, int line, size_t bytes, bool bArray)
{
	this->ptr     = ptr;
	this->file    = file;
	this->line    = line;
	this->bytes   = bytes;
	this->bArray  = bArray;
	bFree = false;
}

//KAllocRegistry
//////////////////////////////////////KAllocRegistry////////////////////////////////////
KAllocRegistry::KAllocRegistry()
	:m_allocCount(0)
	,m_allocBytes(0)
	,m_nonMonitoredBytes(0)
	,m_nonMonitoredCount(0)
{

}

KAllocRegistry& KAllocRegistry::getInstance()
{
	static KAllocRegistry allocRegistry;
	return allocRegistry;
}

KAllocRegistry::~KAllocRegistry()
{
	dump("LeakDump.log");
}

void KAllocRegistry::allocate(KAllocInfo info)
{
	++m_allocCount;

	m_allocBytes += info.bytes;
	unsigned hashCode = reinterpret_cast<unsigned>(info.ptr)%maxAllocs;

	for ( int i=hashCode; i<maxAllocs; ++i )
	{
		if ( m_allocs[i].bFree )
		{
			m_allocs[i] = info;
			return ;
		}
	}

	for ( int i=0; i<hashCode; ++i )
	{
		if ( m_allocs[i].bFree )
		{
			m_allocs[i] = info;
			return ;
		}
	}

	++m_nonMonitoredCount;
	m_nonMonitoredBytes += info.bytes;
}

void KAllocRegistry::deallocate(void* ptr, bool bArray)
{
	unsigned hashCode = reinterpret_cast<unsigned>(ptr)%maxAllocs;

	for ( int i=hashCode; i<maxAllocs; ++i )
	{
		if ( !m_allocs[i].bFree && m_allocs[i].ptr && m_allocs[i].bArray == bArray )
		{
			m_allocs[i].bFree = true;
			return ;
		}
	}

	for( int i=0; i<hashCode; ++i )
	{
		if ( !m_allocs[i].bFree && m_allocs[i].ptr && m_allocs[i].bFree==bArray )
		{
			m_allocs[i].bFree = true;
			return ;
		}
	}

}

void KAllocRegistry::reset()
{
	for ( int i=0; i<maxAllocs; ++i )
	{
		m_allocs[i] = KAllocInfo();
	}
}

void KAllocRegistry::dump(const char* path)
{
	FILE * file	     = fopen(path, "wt");
	int    leakCount = 0;
	size_t leakBytes = 0;

	fprintf(file, "Memory leak dump\n\n");

	for ( int i=0; i<maxAllocs; ++i )
	{
		if ( !m_allocs[i].bFree )
		{
			leakBytes += m_allocs[i].bytes;
			fprintf(file, "%d.\tfile:%s, line:%d, bytes:%d, array:%d\n", ++leakCount, m_allocs[i].file, m_allocs[i].line, m_allocs[i].bytes, m_allocs[i].bArray);
		}
	}

	fprintf(file, "\nTotal leaks:%d, %d bytes\n", leakCount, leakBytes);
	fprintf(file, "Total allocations:%d, %d bytes\n", m_allocCount, m_allocBytes);
	fprintf(file, "Not monitored allocations:%d, %d bytes\n", m_nonMonitoredCount, m_nonMonitoredBytes);

	fclose(file);
}

//////////////////////////////////////KAllocRegistry end////////////////////////////////

#endif