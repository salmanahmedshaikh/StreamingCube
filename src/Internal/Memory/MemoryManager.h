//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../../Common/stdafx.h"
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
/*
   allocate a large size of continuous memory when initials
   divide it into pages
   manage the allocation and release of the pages
 */
class MemoryManager:private boost::noncopyable
{

	MemoryManager(void);
	static MemoryManager* memoryManager;
	char* buffer;
	void initial(void );
	boost::mutex m_oMutex;

	std::list<int>freePageList;
	char* getPageByIndex(int index);

public:
	static MemoryManager* getInstance(void);
	~MemoryManager(void);
	char* getOnePage(void);
	void releaseOnePage(char*);
	static int getPageSize(void);

};

