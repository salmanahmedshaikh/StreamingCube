//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../../Common/stdafx.h"
#include "../../Internal/Element/Element.h"
#include "../../Internal/Buffer/StreamPage.h"
#include "../../Internal/Buffer/BufferConstraint.h"
#include <boost/shared_ptr.hpp>
#include <map>
#include "../../Internal/Buffer/AbstractBuffer.h"
/* a buffer to saves the queue information
* elements are saved in this buffer
* this buffer acts as a queue, elements comes first will go out first.
*/
class QueueBuffer : public AbstractBuffer
{
private:
	MemoryPosition freePositionHead;
	std::list<MemoryPosition> elementQueue;
	int elementNumber; // how many elements are buffered

public:
	QueueBuffer(void)
	{

		this->freePositionHead.pageIndex=0;
		this->freePositionHead.chunkIndex=0;

		this->elementNumber = 0;
	}
	~QueueBuffer(void){}
	inline void enqueue(Element& element)
	{
		bool bl = isFull();
		assert(bl==false);
		if(bl==true)
		{
			std::cout<<"memory not enough"<<std::endl;
			exit(0);
		}
		this->setElementByMemoryPosition(element,freePositionHead);
		this->elementQueue.push_back(freePositionHead);
		this->getNextPosition(freePositionHead,freePositionHead);
		this->elementNumber++;
	}
	inline void dequeue(Element& element)
	{
		assert(this->elementNumber>0);
		assert(elementQueue.size()>0);
		MemoryPosition dequePosition = this->elementQueue.front();
		this->getElementByMemoryPosition(element,dequePosition);
		this->elementNumber--;

		this->setNextPosition(dequePosition,freePositionHead);
		freePositionHead = dequePosition;
		this->elementQueue.pop_front();
	}
	inline void dequeue(void)
	{
		assert(this->elementNumber>0);
		assert(elementQueue.size()>0);
		MemoryPosition dequePosition = this->elementQueue.front();
		this->elementNumber--;

		this->setNextPosition(dequePosition,freePositionHead);
		freePositionHead = dequePosition;
		this->elementQueue.pop_front();
	}
	inline void peek(Element& element)
	{
		assert(this->elementNumber>0);

		MemoryPosition dequePosition = this->elementQueue.front();
		this->getElementByMemoryPosition(element,dequePosition);

	}
	inline void peekNewestElementList(std::list<Element>&elementList, int number)
	{
		assert(this->elementNumber>=number);
		for(std::list<MemoryPosition>::reverse_iterator it = this->elementQueue.rbegin();it!=this->elementQueue.rend();it++)
		{
			if(number==0)
			{
				break;
			}
			Element element;
			MemoryPosition& position = *it;
			this->getElementByMemoryPosition(element,position);
			elementList.push_front(element);
			number--;
		}


	}
	inline void peekAllElements(std::list<Element>&elementList)
	{
		for(std::list<MemoryPosition>::reverse_iterator it = this->elementQueue.rbegin();it!=this->elementQueue.rend();it++)
		{
			Element element;
			MemoryPosition& position = *it;
			this->getElementByMemoryPosition(element,position);
			elementList.push_front(element);
		}


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
	inline bool isEmpty(void)
	{
		return this->elementNumber==0;
	}
	inline int getElementNumber(void)
	{
		return this->elementNumber;
	}
};

