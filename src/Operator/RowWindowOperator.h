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
class RowWindowOperator:public Operator
{
private:
	const int rowWindowSize;
	//int rowWindowSize;
	int currentRowNumberInSynopsis;
	boost::shared_ptr<WindowSynopsis> windowSynopsis;
	Timestamp lastInputTimestamp;
	Element blockedElement;

public:
	RowWindowOperator(int rowWindowSize);
	~RowWindowOperator(void);

    void execution();
	//void dealWithBlock(void);
	int getWindowSize();
};

