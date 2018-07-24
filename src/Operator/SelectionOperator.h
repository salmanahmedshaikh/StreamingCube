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
#include "../Internal/Queue/QueueEntity.h"
#include "../Query/QueryConditionAbstract.h"
#include <boost/shared_ptr.hpp>

class SelectionOperator:public Operator
{
private:
	boost::shared_ptr<QueryConditionAbstract> condition;
	bool isSatisfySelectionCondition(Element& element);
public:
	SelectionOperator(void);
	~SelectionOperator(void);
	void execution();
	void setSelectionCondition(boost::shared_ptr<QueryConditionAbstract> condition);
};

