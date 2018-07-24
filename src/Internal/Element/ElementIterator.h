//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma  once
#include "../../Common/stdafx.h"
#include "../../Internal/Buffer/RelationBuffer.h"
#include "../../Internal/Synopsis/RelationSynopsis.h"
#include "../../Common/Types.h"
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
//this is used to iterate the elements one by one in the relation buffer
class ElementIterator
{
private:
	boost::shared_ptr<RelationBuffer> relationBuffer;
	boost::unordered_map<DocumentId, MemoryPosition>::iterator it;
public:
	//std::set<DocumentId>
	ElementIterator(boost::shared_ptr<RelationSynopsis> relationSynopsis)
	{
		this->relationBuffer = relationSynopsis->buffer;
	}

	void initial(void)
	{
		it = relationBuffer->idPositionMap.begin();
	}

	bool getNext(Element& element)
	{
		if(it==relationBuffer->idPositionMap.end())
		{
			return false;
		}
		MemoryPosition& memoryPosition = it->second;
		relationBuffer->getElementByMemoryPosition(element,memoryPosition);
		it++;
		return true;
	}
	//void erase() // erase the current element pointed by the iterator from the buffer
	//{
	//	if(it==relationBuffer->idPositionMap.end())
	//	{
	//		return;
	//	}
	//	MemoryPosition elementPosition = it->second;
	//	this->relationBuffer->setNextPosition(elementPosition,this->relationBuffer->freePositionHead);
	//	this->relationBuffer->freePositionHead = elementPosition;
	//	it = relationBuffer->idPositionMap.erase(it);
	//}
};

//iterator the elements in the relation synopsis by a set of ducumentid
class SetElementIterator
{
private:
	boost::shared_ptr<RelationBuffer> relationBuffer;
	std::set<DocumentId> documentIdSet;
	std::set<DocumentId>::iterator it;
public:
	//std::set<DocumentId>
	SetElementIterator(boost::shared_ptr<RelationSynopsis> relationSynopsis,std::set<DocumentId>documentIdSet)
	{
		this->relationBuffer = relationSynopsis->buffer;
		this->documentIdSet = documentIdSet;
	}
	void initial(void)
	{
		it = documentIdSet.begin();
	}
	bool getNext(Element& element)
	{
		if(it==documentIdSet.end())
		{
			return false;
		}
		DocumentId id = *it;
		boost::unordered_map<DocumentId, MemoryPosition>::iterator itt = relationBuffer->idPositionMap.find(id);
		assert(itt!=relationBuffer->idPositionMap.end());
		MemoryPosition& memoryPosition = itt->second;
		relationBuffer->getElementByMemoryPosition(element,memoryPosition);
		it++;

		return true;
	}

	bool getThis(Element& element) {
			if (it == documentIdSet.end()) {
				return false;
			}
			DocumentId id = *it;
			boost::unordered_map<DocumentId, MemoryPosition>::iterator itt =
					relationBuffer->idPositionMap.find(id);
			assert(itt != relationBuffer->idPositionMap.end());
			MemoryPosition& memoryPosition = itt->second;
			relationBuffer->getElementByMemoryPosition(element, memoryPosition);

			return true;
		}

};
