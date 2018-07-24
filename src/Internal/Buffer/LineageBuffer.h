//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../../Common/stdafx.h"
#include <boost/shared_ptr.hpp>
#include "../../Internal/Buffer/StreamPage.h"
#include "../../Internal/Buffer/BufferConstraint.h"
#include "../../Internal/Element/Element.h"
#include "../../Internal/Buffer/AbstractBuffer.h"
#include <boost/unordered_map.hpp>
/* a buffer to saves the lineage information
when insert, user should provide (lineage, element), "lineage" is the identifier of the element help generate the "element"
then element is saved in the buffer
some one can use lineage to search for element
*/
class LineageBuffer:public AbstractBuffer
{
private:
	const int lineageNumber; // the number of lineage document in one record.
	MemoryPosition freePositionHead;  //the head memory position of free memory chunks

	// map the lineage to memory position, it is used to search the element by lineage easily
	std::map<Lineage, MemoryPosition>lineagePositionMap;

public:
	LineageBuffer( int lineageNumber):lineageNumber(lineageNumber)
	{


		this->freePositionHead.pageIndex=0;
		this->freePositionHead.chunkIndex=0;
	}
	~LineageBuffer(void){}
	inline void insertLineage(Lineage& lineage,Element& outputElement)
	{
		bool bl = isFull();
		assert(bl==false);
		if(bl==true)
		{
			std::cout<<"memory not enough"<<std::endl;
			exit(0);
		}


		this->setElementByMemoryPosition(outputElement,this->freePositionHead);
		if(this->lineagePositionMap.find(lineage)!=this->lineagePositionMap.end())//this lineage is already exists
		{
			return;
		}
		this->lineagePositionMap.insert(make_pair(lineage,this->freePositionHead));
		this->getNextPosition(this->freePositionHead,this->freePositionHead);


	}

	inline bool getAndDeleteElement(Lineage& lineage, Element& outputElement)
	{
		std::map<Lineage, MemoryPosition>::iterator it;
		it = this->lineagePositionMap.find(lineage);
		if(it==this->lineagePositionMap.end())
		{
			return false;
		}

		MemoryPosition elementPosition = it->second;
		this->getElementByMemoryPosition(outputElement,elementPosition);
		this->setNextPosition(elementPosition,this->freePositionHead);
		this->freePositionHead = elementPosition;
		this->lineagePositionMap.erase(it);
		return true;
	}

	inline bool isFull(void)
	{
		bool ret;
		if(!isValidMemoryPosition(freePositionHead))
		{
			ret = this->allocateNewPage();
			if(ret == false)
			{
				return true;
			}
			this->freePositionHead.pageIndex = this->getPageNumber()-1;
			this->freePositionHead.chunkIndex = 0;

		}

		return false;
	}

};

