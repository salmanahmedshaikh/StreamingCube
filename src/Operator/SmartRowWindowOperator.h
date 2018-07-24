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
#include "../Query/QueryEntity.h"
class SmartRowWindowOperator:public Operator
{
private:
	const int rowWindowSize;
	int currentRowNumberInSynopsis;
	boost::shared_ptr<WindowSynopsis> windowSynopsis;
	Timestamp lastInputTimestamp;
	Timestamp lastOutputBufferedTupleTimestamp;
	void executionInWaitingMode(Element& element);
	void executionInEagerMode(Element& element);
	void outputBufferedElement();
	int bufferedTupleNumber; // the number of tuples in the synopsis that haven't generate plus tuples
	std::list<QueryEntity*> queryEntityList;
public:
	SmartRowWindowOperator(int rowWindowSize);
	~SmartRowWindowOperator(void);
	void addQuery(QueryEntity* queryEntity);
	void execution();
	static int directlyDeletedTuple;
	int getWindowSize();
	std::list<QueryEntity*> getRelatedQueries();

};

