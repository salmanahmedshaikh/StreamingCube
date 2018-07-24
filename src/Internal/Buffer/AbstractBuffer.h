//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../../Common/stdafx.h"
#include "../../Internal/Buffer/StreamPage.h"
#include <boost/shared_ptr.hpp>
#include "../../Common/Types.h"
#include "../../Internal/Buffer/BufferConstraint.h"
#include "../../Internal/Element/Element.h"
/* this is an abstract buffer, it contains pages allocated from the memory manager
each page is divided into chunks, one record is saved in one chunk.
The chunks are linked. The first 4 bytes in one chunk saves the memory position of next chunk.
After the 4 bytes, saves the element by an order of Timestamp, DocumentId, Mark, document
*/
class AbstractBuffer
{
private:

	int pageNumber;    // how many pages are allocated for this buffer
	std::vector<boost::shared_ptr<StreamPage> > pageVector; // the vector contains the pages
public:

	AbstractBuffer(void)
	{

		boost::shared_ptr<StreamPage>elementPage(new StreamPage);
		this->pageVector.push_back(elementPage);
		this->pageNumber = 1;
		MemoryPosition prevMemoryPosition;
		//NUMBER_OF_CHUNKS_PER_PAGE = number of chunks per page
		for(int j = NUMBER_OF_CHUNKS_PER_PAGE-1 ;j>=0; j--)
		{
			this->pageVector.at(0)->setPointerOfNextChunk(j,prevMemoryPosition);
			prevMemoryPosition.pageIndex = 0;
			prevMemoryPosition.chunkIndex = j;

		}

	}
	virtual ~AbstractBuffer(void){}
	inline bool isValidMemoryPosition(MemoryPosition& memoryPosition)  // if a memory position is valid in this buffer
	{

		return memoryPosition.pageIndex>=0&&memoryPosition.pageIndex<=this->pageNumber-1
			&& memoryPosition.chunkIndex>=0&&memoryPosition.chunkIndex<=NUMBER_OF_CHUNKS_PER_PAGE-1;
	}
	inline void getElementByMemoryPosition(Element&element,MemoryPosition& memoryPosition)
	{
		assert(isValidMemoryPosition(memoryPosition));
		this->pageVector.at(memoryPosition.pageIndex)->getValue(memoryPosition.chunkIndex,element);

	}
	inline void setElementByMemoryPosition(Element&element,MemoryPosition& memoryPosition)
	{
		assert(isValidMemoryPosition(memoryPosition));
		this->pageVector.at(memoryPosition.pageIndex)->setValue(memoryPosition.chunkIndex,element);
	}
	inline void setNextPosition(MemoryPosition& chunkMemoryPosition, MemoryPosition& nextFreeHeadPointerPosition)
	{
		this->pageVector.at(chunkMemoryPosition.pageIndex)->setPointerOfNextChunk(chunkMemoryPosition.chunkIndex,nextFreeHeadPointerPosition);
		//this->pageVector[chunkMemoryPosition.pageIndex]->setPointerOfNextChunk(chunkMemoryPosition.chunkIndex,nextFreeHeadPointerPosition);
	}
	inline void getNextPosition(MemoryPosition& chunkMemoryPosition, MemoryPosition& nextFreeHeadPointerPosition)
	{
		this->pageVector.at(chunkMemoryPosition.pageIndex)->getPointerOfNextChunk(chunkMemoryPosition.chunkIndex,nextFreeHeadPointerPosition);
	}
	inline bool allocateNewPage(void)//allocate a new page from memory manager
	{
		if(this->pageNumber == MAX_PAGE_NUMBER_IN_ONE_QUEUE_OR_SYNOPSIS)
		{
			return false;
		}
		boost::shared_ptr<StreamPage>elementPage(new StreamPage);
		this->pageVector.push_back(elementPage);
		this->pageNumber++;

		MemoryPosition prevMemoryPosition;
		for(int j = NUMBER_OF_CHUNKS_PER_PAGE-1 ;j>=0; j--)
		{
			this->pageVector.at(pageNumber-1)->setPointerOfNextChunk(j,prevMemoryPosition);
			prevMemoryPosition.pageIndex = pageNumber-1;
			prevMemoryPosition.chunkIndex = j;
		}

		MemoryPosition newMemoryPosition;
		newMemoryPosition.pageIndex = this->pageNumber-1;
		newMemoryPosition.chunkIndex = 0;
		this->pageVector.at(pageNumber-2)->setPointerOfNextChunk(NUMBER_OF_CHUNKS_PER_PAGE-1,newMemoryPosition);

		return true;
	}
	inline int getPageNumber(void)
	{
		return this->pageNumber;
	}
};

