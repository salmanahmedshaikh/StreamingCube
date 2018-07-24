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
#include <boost/shared_ptr.hpp>
#include "../../Internal/Buffer/RelationBuffer.h"
#include "../../Internal/Buffer/BufferConstraint.h"
#include "../../Internal/Synopsis/Synopsis.h"
class ElementIterator;
class SetElementIterator;
class RelationSynopsis: public Synopsis
{
private:
	boost::shared_ptr<RelationBuffer>buffer;

public:
	RelationSynopsis(void);
	virtual ~RelationSynopsis(void);
	void insertElement(Element& element);
	bool deleteElement(Element& element);
	void findElementByElementId(Element& element, DocumentId id);
	void clear(void);
	bool isFull(void);
	friend class ElementIterator;
	friend class SetElementIterator;
};
