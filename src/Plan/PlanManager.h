//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../Query/QueryIntermediateRepresentation.h"
#include "../Operator/Operator.h"
#include "../Operator/RootOperator.h"
#include "../Query/QueryEntity.h"
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "QueryPlanTree.h"
/*
 generate query plan tree, initialize the operators, the queues
*/
class PlanManager:private boost::noncopyable
{
private:
	static PlanManager* planManager;
	std::list<boost::shared_ptr<Operator> > previousResultOperatorList;
	std::list<boost::shared_ptr<Operator> > gateOpAppendedToOpList;
	PlanManager(void);
	std::list<boost::shared_ptr<QueryPlanTree> >queryPlanTreeList;
	void moveOutputQueues(boost::shared_ptr<Operator> operatorTo, boost::shared_ptr<Operator> operatorFrom);
	void mergeOperators(std::list<boost::shared_ptr<Operator> >&resultOperatorList, std::list<boost::shared_ptr<Operator> >toOperatorList, std::list<boost::shared_ptr<Operator> >fromOperatorList);
	bool isSameLeafOperatorLogic(boost::shared_ptr<LeafOperator> op1, boost::shared_ptr<LeafOperator> op2);
	void appendOutputOperators(std::list<boost::shared_ptr<Operator> >&opList, boost::shared_ptr<Operator>op);
	void insertGateOperators(std::list<boost::shared_ptr<Operator> >&operatorList);
	std::list<boost::shared_ptr<Operator> >getAllLeafOperators(std::list<boost::shared_ptr<Operator> >operatorList);
	std::list<boost::shared_ptr<LeafOperator> >getLeafOperatorList(boost::shared_ptr<Operator> op);
	void appendGateOperator(boost::shared_ptr<Operator> op,std::list<boost::shared_ptr<Operator> >& operatorList);
	std::list<boost::shared_ptr<RootOperator> >getRootOperatorList(boost::shared_ptr<Operator> op);
	std::list<boost::shared_ptr<Operator> > sortOperators(std::list<boost::shared_ptr<Operator> >operatorList);
	void printOperatorList(std::list<boost::shared_ptr<Operator> >operatorList);
	std::list<boost::shared_ptr<Operator> > traversePostOrder(boost::shared_ptr<Operator> op);
	void appendQueueId(std::list<boost::shared_ptr<Operator> >operatorList);
public:
	~PlanManager(void);
	static PlanManager* getInstance();
	void createQueryPlan(boost::shared_ptr<QueryEntity>queryEntity);
//	std::list<boost::shared_ptr<Operator> > getAllMergedOperators(void);
	std::list<boost::shared_ptr<Operator> > getAllOperators(void);
};
