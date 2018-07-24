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
#include "../../Internal/Buffer/LineageBuffer.h"
#include "../../Internal/Buffer/BufferConstraint.h"
#include "../../Internal/Element/Element.h"
#include <boost/shared_ptr.hpp>
#include "../../Internal/Synopsis/Synopsis.h"
class LineageSynopsis:public Synopsis
{
private:
	boost::shared_ptr<LineageBuffer>buffer;
	const int lineageNumber;
public:
	LineageSynopsis(int lineageNumber);
	virtual ~LineageSynopsis(void);

	void insertLineage(Lineage& lineage,Element& outputElement);
	bool getAndDeleteElement(Lineage& lineage, Element& outputElement);
	bool isFull(void);
};

