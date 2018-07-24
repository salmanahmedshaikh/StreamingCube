//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../../Common/stdafx.h"
#include "../../Internal/Memory/MemoryManager.h"
#include "../../Internal/Buffer/BufferConstraint.h"
MemoryManager* MemoryManager::memoryManager = NULL;
MemoryManager::MemoryManager(void)
{
	initial();
}


MemoryManager::~MemoryManager(void)
{
}

MemoryManager* MemoryManager::getInstance()
{
	if(memoryManager==NULL)
	{
		memoryManager = new MemoryManager();
	}
	return memoryManager;
}
void MemoryManager::initial()
{
	boost::mutex::scoped_lock lock(m_oMutex);
	this->buffer = (char*)malloc(BUFFER_MAX_SIZE);
	assert(BUFFER_MAX_SIZE % PAGE_SIZE == 0 );

	for(int i = 0; i< PAGE_NUMBER ;i++)
	{
		freePageList.push_back(i);
	}
	lock.unlock();


}
char* MemoryManager::getPageByIndex(int index)
{
	return this->buffer + index * PAGE_SIZE;
}
char* MemoryManager::getOnePage(void)
{
	boost::mutex::scoped_lock lock(m_oMutex);
	if(freePageList.size()==0)
	{
		throw std::runtime_error("out of memory");
	}
	int pageIndex = freePageList.front();
	freePageList.pop_front();
	lock.unlock();
	return getPageByIndex(pageIndex);
}
void MemoryManager::releaseOnePage(char* buffer)
{
	boost::mutex::scoped_lock lock(m_oMutex);
	assert((buffer - this->buffer)%PAGE_SIZE == 0);
	int index = (buffer - this->buffer)/PAGE_SIZE;
	freePageList.push_front(index);
	lock.unlock();
}

int MemoryManager::getPageSize(void)
{
	return PAGE_SIZE;
}
