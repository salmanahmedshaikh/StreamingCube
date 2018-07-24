//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../../Common/stdafx.h"
#include "../../Common/Types.h"

/*
 * this file specifies constraints about buffer
 */

//random generate configuration
#ifndef TOKYO_PEOPLE_FLOW_DEMO
const int BUFFER_MAX_SIZE = (1 << 20) * 1024;//1024MB, the max size of the whole buffer owns by the memory manager
const int PAGE_SIZE = 4096;                // the size of one page
const int PAGE_NUMBER = BUFFER_MAX_SIZE/PAGE_SIZE; // how many pages the memory manager holds
const int CHUNK_SIZE = 512;                // the size of chunk, each record is saved in one chunk
const int ROW_CHUNK_SIZE = CHUNK_SIZE/2;                // the size of chunk, each record is saved in one chunk
const int NUMBER_OF_CHUNKS_PER_PAGE = PAGE_SIZE/CHUNK_SIZE; // how many chunks in one page
const int NUMBER_OF_ROW_CHUNKS_PER_PAGE = PAGE_SIZE/ROW_CHUNK_SIZE; // how many chunks in one page
const int MAX_PAGE_NUMBER_IN_ONE_QUEUE_OR_SYNOPSIS = 8*4096;  //1024; // a queue or synopsis can allocate how many pages at most
#endif // TOKYO_PEOPLE_FLOW_DEMO

/*
#ifndef TOKYO_PEOPLE_FLOW_DEMO
const int BUFFER_MAX_SIZE = (1 << 20) * 256;//256MB, the max size of the whole buffer owns by the memory manager
const int PAGE_SIZE = 4096;                // the size of one page
const int PAGE_NUMBER = BUFFER_MAX_SIZE/PAGE_SIZE; // how many pages the memory manager holds
const int CHUNK_SIZE = 256;                // the size of chunk, each record is saved in one chunk
const int NUMBER_OF_CHUNKS_PER_PAGE = PAGE_SIZE/CHUNK_SIZE; // how many chunks in one page
const int MAX_PAGE_NUMBER_IN_ONE_QUEUE_OR_SYNOPSIS = 8*4096;  //1024; // a queue or synopsis can allocate how many pages at most
#endif // TOKYO_PEOPLE_FLOW_DEMO
*/

//twitter && RSS configuration
//const int BUFFER_MAX_SIZE = (1 << 20) * 64;//64MB, the max size of the whole buffer owns by the memory manager
//const int PAGE_SIZE = 4096*32;                // the size of one page
//const int PAGE_NUMBER = BUFFER_MAX_SIZE/PAGE_SIZE; // how many pages the memory manager holds
//const int CHUNK_SIZE = 8192;                // the size of chunk, each record is saved in one chunk
//const int NUMBER_OF_CHUNKS_PER_PAGE = PAGE_SIZE/CHUNK_SIZE; // how many chunks in one page
//const int MAX_PAGE_NUMBER_IN_ONE_QUEUE_OR_SYNOPSIS = 4; // a queue or synopsis can allocate how many pages at most

//people flow configuration
#ifdef TOKYO_PEOPLE_FLOW_DEMO
const int BUFFER_MAX_SIZE = (1 << 20) * 256;//64MB, the max size of the whole buffer owns by the memory manager
const int PAGE_SIZE = 4096;                // the size of one page
const int PAGE_NUMBER = BUFFER_MAX_SIZE/PAGE_SIZE; // how many pages the memory manager holds
const int CHUNK_SIZE = 512;                // the size of chunk, each record is saved in one chunk
const int NUMBER_OF_CHUNKS_PER_PAGE = PAGE_SIZE/CHUNK_SIZE; // how many chunks in one page
const int MAX_PAGE_NUMBER_IN_ONE_QUEUE_OR_SYNOPSIS = 4; // a queue or synopsis can allocate how many pages at most
#endif // TOKYO_PEOPLE_FLOW_DEMO

typedef short PAGE_INDEX_TYPE;    // page  numbers in buffer is no more than 1<<16 = 65535.
typedef short CHUNK_INDEX_TYPE;   // chunk numbers in memory is no more than 1<<16 = 65535.

const int CHUNK_HEADER_MAX_LENGTH = sizeof(PAGE_INDEX_TYPE)+sizeof(CHUNK_INDEX_TYPE);

/* MemoryPosition can specify one location in the memory in some buffer
   one buffer contains some pages
   pageIndex indicates the page index in the buffer
   chunkIndex indicates the chunk index in the page
*/
typedef struct _MemoryPosition
{
	PAGE_INDEX_TYPE pageIndex;
	CHUNK_INDEX_TYPE chunkIndex;
	bool operator ==(_MemoryPosition& temp)
	{
		return this->pageIndex == temp.pageIndex && this->chunkIndex == temp.chunkIndex;
	}
	_MemoryPosition()
	{
		pageIndex = -1;
		chunkIndex = -1;
	}

}MemoryPosition;


