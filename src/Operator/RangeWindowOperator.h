//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../Operator/Operator.h"
#include "../Internal/Synopsis/WindowSynopsis.h"
#include "../Internal/Queue/QueueEntity.h"
#include <boost/shared_ptr.hpp>
class RangeWindowOperator:public Operator
{
private:
	const Timeunit rangeWindowSize;

	boost::shared_ptr<WindowSynopsis> windowSynopsis;
	Timestamp lastInputTimestamp;
	Timestamp oldestElementTimestampInWinSynopsis;

	//void expireElement(Timestamp timestamp, Element& inputElement);
	//void expireElement(Element& inputElement);
	void expireElement();

public:
	RangeWindowOperator(Timeunit timeRange);
	~RangeWindowOperator(void);

	Timeunit getWindowSize();

	void execution();
};

