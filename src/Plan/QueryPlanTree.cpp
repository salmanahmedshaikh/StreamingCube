//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Plan/PlanManager.h"
#include "../Operator/Operator.h"
#include "../Operator/RootOperator.h"
#include "../IO/IStreamOutput.h"
#include "../Query/QueryManager.h"
#include "../Operator/DstreamOperator.h"
#include "../Operator/IstreamOperator.h"
#include "../Operator/RstreamOperator.h"
#include "../Operator/ProjectionOperator.h"
#include "../Operator/SelectionOperator.h"
#include "../Operator/JoinOperator.h"
#include "../Operator/RowWindowOperator.h"
#include "../Operator/RangeWindowOperator.h"
#include "../Operator/SmartRowWindowOperator.h"
#include "../Operator/LeafOperator.h"
#include "../Operator/GroupAggregationOperator.h"
#include "../Query/QueryIntermediateRepresentationInterpreter.h"
#include "../Internal/Queue/QueueEntity.h"

QueryPlanTree::QueryPlanTree(boost::shared_ptr<Operator>rootOperator)
{
	std::list<boost::shared_ptr<Operator> >operatorList;
	addOperator(rootOperator,operatorList);

#ifdef SMART
	 //change rowwindow operator to smart rowwindow operator for non-master streams
	 std::list<boost::shared_ptr<Operator> >::iterator it;
	 for(it = operatorList.begin(); it != operatorList.end(); it++)
	 {
		 boost::shared_ptr<Operator> op = *it;
		 if(typeid(*op) == typeid(LeafOperator))
		 {
			boost::shared_ptr<LeafOperator> leafOperator =boost::dynamic_pointer_cast<LeafOperator>(op);
			bool masterTag = leafOperator->getRelatedQueries().front()->getMasterTag(&*leafOperator);
			QueryEntity * queryEntity = leafOperator->getRelatedQueries().front();

			//if the query stream source is non-master, ordinary window can be converted into smart window
			if(masterTag == false)
			{
				boost::shared_ptr<QueueEntity> inputQueue = leafOperator->getOutputQueueList().front();
				boost::shared_ptr<Operator> winOperator = inputQueue->getOutputOperator();

				//boost::shared_ptr<RangeWindowOperator> rangeWindowOperator = boost::dynamic_pointer_cast<RangeWindowOperator>(winOperator);
				//std::cout << "RangeWindowOperator " << rangeWindowOperator << std::endl;

				boost::shared_ptr<RowWindowOperator> rowWindowOperator = boost::dynamic_pointer_cast<RowWindowOperator>(winOperator);
				//std::cout << "rowWindowOperator " << rowWindowOperator << std::endl;

				int windowSize = rowWindowOperator->getWindowSize();
				boost::shared_ptr<QueueEntity> outputQueue = winOperator->getOutputQueueList().front();
				boost::shared_ptr<SmartRowWindowOperator> smartRowWindowOperator (new SmartRowWindowOperator(windowSize));
				smartRowWindowOperator->addQuery(queryEntity);
				smartRowWindowOperator->addInputQueue(inputQueue);
				smartRowWindowOperator->addOutputQueue(outputQueue);
				inputQueue->setOutputOperator(smartRowWindowOperator);
				outputQueue->setInputOperator(smartRowWindowOperator);
				it++; // by default, the window operator is after the leaf operator in the list
				*it = smartRowWindowOperator;
			}
		 }
	 }
#endif
	 //std::cout<<operatorList.size()<<std::endl;
	 this->operatorList = operatorList;
}
QueryPlanTree::~QueryPlanTree()
{
}


void QueryPlanTree::addOperator(boost::shared_ptr<Operator> op,std::list<boost::shared_ptr<Operator> >&operatorList)
{
	std::list<boost::shared_ptr<QueueEntity> > inputQueueList = op->getInputQueueList();
	std::list<boost::shared_ptr<QueueEntity> > ::iterator it;
	for(it= inputQueueList.begin();it!=inputQueueList.end();it++)
	{
		boost::shared_ptr<QueueEntity> queueEntity = *it;
		boost::shared_ptr<Operator> inputOperator = queueEntity->getInputOperator();
		addOperator(inputOperator,operatorList);
	}
	operatorList.push_back(op);
}

void QueryPlanTree::setOperatorIdAndQueueId(std::string prefix)
{
	int leafOperatorNumber = 0;
	int rowwindowOperatorNumber = 0;
	int smartRowWindowOperatorNumber = 0;
	int rangewindowOperatorNumber = 0;
	int selectionOperatorNumber = 0;
	int projectionOperatorNumber = 0;
	int joinOperatorNumber = 0;
	int dstreamOperatorNumber = 0;
	int istreamOperatorNumber = 0;
	int rstreamOperatorNumber = 0;
	int rootOperatorNumber = 0;
	int groupAggregationOperatorOperatorNumber = 0;
	int queueNumber = 0;
	std::list<boost::shared_ptr<Operator> >operatorList = this->operatorList;
	std::list<boost::shared_ptr<Operator> >::iterator it;
	for(it = operatorList.begin();it!=operatorList.end();it++)
	{
		boost::shared_ptr<Operator> op = *it;
		if(typeid(*op)==typeid(LeafOperator))
		{
			stringstream ss;
			leafOperatorNumber++;
			ss<<leafOperatorNumber;
			op->setId(prefix + QUERY_LEAF + "operator" +  ss.str());
		}
		else if(typeid(*op)==typeid(RowWindowOperator))
		{
			stringstream ss;
			rowwindowOperatorNumber++;
			ss<<rowwindowOperatorNumber;
			op->setId(prefix + QUERY_ROWWINDOW  + "operator" +  ss.str());
		}
		else if(typeid(*op)==typeid(SmartRowWindowOperator))
		{
			stringstream ss;
			smartRowWindowOperatorNumber++;
			ss<<smartRowWindowOperatorNumber;
			op->setId(prefix + QUERY_SMART_ROW_WINDOW  + "operator" +  ss.str());
		}
		else if(typeid(*op)==typeid(RangeWindowOperator))
		{
			stringstream ss;
			rangewindowOperatorNumber++;
			ss<<rangewindowOperatorNumber;
			op->setId(prefix + QUERY_RANGEWINDOW  + "operator" +  ss.str());
		}
		else if(typeid(*op)==typeid(SelectionOperator))
		{
			stringstream ss;
			selectionOperatorNumber++;
			ss<<selectionOperatorNumber;
			op->setId(prefix + QUERY_SELECTION  + "operator" +  ss.str());
		}
		else if(typeid(*op)==typeid(ProjectionOperator))
		{
			stringstream ss;
		    projectionOperatorNumber++;
			ss<<projectionOperatorNumber;
			op->setId(prefix + QUERY_PROJECTION  + "operator" +  ss.str());
		}
		else if(typeid(*op)==typeid(JoinOperator))
		{
			stringstream ss;
			joinOperatorNumber++;
			ss<<joinOperatorNumber;
			op->setId(prefix + QUERY_JOIN  + "operator" +  ss.str());
		}
		else if(typeid(*op)==typeid(IstreamOperator))
		{
			stringstream ss;
			istreamOperatorNumber++;
			ss<<istreamOperatorNumber;
			op->setId(prefix + QUERY_ISTREAM  + "operator" +  ss.str());
		}
		else if(typeid(*op)==typeid(DstreamOperator))
		{
			stringstream ss;
			dstreamOperatorNumber++;
			ss<<dstreamOperatorNumber;
			op->setId(prefix + QUERY_DSTREAM  + "operator" +  ss.str());
		}
		else if(typeid(*op)==typeid(RstreamOperator))
		{
			stringstream ss;
			rstreamOperatorNumber++;
			ss<<rstreamOperatorNumber;
			op->setId(prefix + QUERY_RSTREAM  + "operator" +  ss.str());
		}
		else if(typeid(*op)==typeid(RootOperator))
		{
			stringstream ss;
			rootOperatorNumber++;
			ss<<rootOperatorNumber;
			op->setId(prefix + QUERY_ROOT  + "operator" +  ss.str());
		}
		else if(typeid(*op)==typeid(GroupAggregationOperator))
		{
			stringstream ss;
			groupAggregationOperatorOperatorNumber++;
			ss<<groupAggregationOperatorOperatorNumber;
			op->setId(prefix + QUERY_GROUPBY_AGGREGATION  + "operator" +  ss.str());
		}
		std::list<boost::shared_ptr<QueueEntity> > outputQueueList;
		std::list<boost::shared_ptr<QueueEntity> >::iterator itt;
		for(itt = outputQueueList.begin();itt!=outputQueueList.end();itt++)
		{
			boost::shared_ptr<QueueEntity> queueEntity = *itt;
			queueNumber++;
			stringstream ss;
			ss<<queueNumber;
			queueEntity->setId(prefix + "queue" + ss.str());
		}
	}
}

std::list<boost::shared_ptr<Operator> > QueryPlanTree::getAllOperators()
{
	return this->operatorList;
}
