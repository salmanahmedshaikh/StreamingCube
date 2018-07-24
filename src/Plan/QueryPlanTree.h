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
class QueryPlanTree
{
private:
	std::list<boost::shared_ptr<Operator> >operatorList;
	void addOperator(boost::shared_ptr<Operator> op,std::list<boost::shared_ptr<Operator> >&operatorList);
public:
	QueryPlanTree(boost::shared_ptr<Operator>rootOperator);
	~QueryPlanTree();
	std::list<boost::shared_ptr<Operator> >getAllOperators();
	void setOperatorIdAndQueueId(std::string prefix);
};

