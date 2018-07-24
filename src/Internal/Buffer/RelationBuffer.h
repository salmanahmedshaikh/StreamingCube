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
#include "../../Common/Types.h"
#include "../../Internal/Buffer/BufferConstraint.h"
#include <boost/shared_ptr.hpp>
#include "../../Internal/Element/Element.h"
#include "../../Internal/Buffer/AbstractBuffer.h"
#include <boost/unordered_map.hpp>

/* a buffer to saves the relation information
 insert element in, and delete element by id
 */
class ElementIterator;
class SetElementIterator;
class RelationBuffer: public AbstractBuffer {
private:

	boost::unordered_map<DocumentId, MemoryPosition> idPositionMap;
	MemoryPosition freePositionHead;

public:
	RelationBuffer(void){
		this->freePositionHead.pageIndex = 0;
		this->freePositionHead.chunkIndex = 0;
	}

	~RelationBuffer(void) {
	}
	inline void insertElement(Element& element) {

		bool bl = isFull();
		assert(bl==false);
		if (bl == true)
		{
			std::cout << "RelationBuffer memory not enough" << std::endl;
			bool bl = isFull();
			exit(1);
		}
		MemoryPosition elementPosition = this->freePositionHead;

		this->setElementByMemoryPosition(element, elementPosition);
		this->idPositionMap.insert(make_pair(element.id, elementPosition));
		this->getNextPosition(elementPosition, freePositionHead);

	}
	inline bool deleteElement(Element& element) {
		bool bl = false;
		DocumentId id = element.id;
		boost::unordered_map<DocumentId, MemoryPosition>::iterator it;
		it = this->idPositionMap.find(id);
		//assert(it!=this->idPositionMap.end());
		if (it != this->idPositionMap.end()) {
			bl = true;
			MemoryPosition elementPosition = it->second;
			this->setNextPosition(elementPosition, freePositionHead);
			this->freePositionHead = elementPosition;
			this->idPositionMap.erase(it);
		} else {
			//the element is not in the relation synopsis
		}
		return bl;
	}
	inline bool isFull(void) {
		bool ret;
		if (!isValidMemoryPosition(freePositionHead)) {
			ret = this->allocateNewPage();
			if (ret == false) {
				return true;
			}
			this->freePositionHead.pageIndex = this->getPageNumber() - 1;
			this->freePositionHead.chunkIndex = 0;

		}

		return false;
	}
	inline void clear(void) {
		boost::unordered_map<DocumentId, MemoryPosition>::iterator it;
		for (it = idPositionMap.begin(); it != idPositionMap.end();) {
			MemoryPosition elementPosition = it->second;
			this->setNextPosition(elementPosition, freePositionHead);
			this->freePositionHead = elementPosition;
			it = this->idPositionMap.erase(it);

		}
	}

	void findElementByElementId(Element& element, DocumentId id) {
		boost::unordered_map<DocumentId, MemoryPosition>::iterator it;
		it = this->idPositionMap.find(id);
		//assert(it!=this->idPositionMap.end());
		if (it != this->idPositionMap.end()) {
			MemoryPosition elementPosition = it->second;
			this->getElementByMemoryPosition(element, elementPosition);
		} else {
			//the element is not in the relation synopsis
		}
	}

	//  void getAllElements(std::list<Element>&elementList);
	friend class ElementIterator;
	friend class SetElementIterator;
};
