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
#include "../Operator/GateOperator.h"
#include "../Operator/SelectionOperator.h"
#include "../Operator/JoinOperator.h"
#include "../Operator/RowWindowOperator.h"
#include "../Operator/RangeWindowOperator.h"
#include "../Operator/SmartRowWindowOperator.h"
#include "../Operator/LeafOperator.h"
#include "../Query/QueryIntermediateRepresentationInterpreter.h"
#include "../Internal/Queue/QueueEntity.h"
#include "../IO/IStreamInput.h"
#include "QueryPlanTree.h"
#include <sstream>
#include <boost/shared_ptr.hpp>
#include <algorithm>
PlanManager* PlanManager::planManager = NULL;
PlanManager::PlanManager(void)
{
}

PlanManager::~PlanManager(void)
{
}

PlanManager* PlanManager::getInstance()
{

	if(planManager==NULL)
	{
		planManager = new PlanManager();
	}
	return planManager;
}

std::list<boost::shared_ptr<Operator> > PlanManager::getAllOperators(void)
{
	std::list<boost::shared_ptr<Operator> > resultOperatorList;
	// getting the previously merged query results
	resultOperatorList.insert(resultOperatorList.end(),this->previousResultOperatorList.begin(), this->previousResultOperatorList.end());
	//std::cout << "this->previousResultOperatorList " << this->previousResultOperatorList.size() << std::endl;

	std::list<boost::shared_ptr<QueryPlanTree> >::iterator it;
	static int queryMergeCounter = 0;

	//std::cout << "Call " << queryMergeCounter << std::endl;

    int i = 0;
    for(it = this->queryPlanTreeList.begin(); it != this-> queryPlanTreeList.end(); it++)
	{
        if(i == queryMergeCounter)
        {
            boost::shared_ptr<QueryPlanTree> queryPlanTree = *it;
            std::list<boost::shared_ptr<Operator> >opList = queryPlanTree->getAllOperators();

            //std::cout << "opList.size " << opList.size() << std::endl;
            //for (std::list<boost::shared_ptr<Operator> >::iterator opListIt=opList.begin(); opListIt != opList.end(); ++opListIt)
                //std::cout << *opListIt << std::endl;

            std::list<boost::shared_ptr<Operator> >toLeafOperatorList = getAllLeafOperators(resultOperatorList);
            resultOperatorList.insert(resultOperatorList.end(),opList.begin(), opList.end());

            std::list<boost::shared_ptr<Operator> >fromLeafOperatorList = getAllLeafOperators(opList);
            #ifndef SHARE_NO_OPERATOR
            mergeOperators(resultOperatorList,toLeafOperatorList,fromLeafOperatorList);
            #endif // SHARE_NO_OPERATOR
		}
		i++;
	}

	// Copying all the operators of the final operatorList to previousResultOperatorList
    this->previousResultOperatorList.erase(this->previousResultOperatorList.begin(),this->previousResultOperatorList.end());
    //std::cout << "this->previousResultOperatorList.size() " << this->previousResultOperatorList.size() << std::endl;
    this->previousResultOperatorList.insert(this->previousResultOperatorList.end(),resultOperatorList.begin(),resultOperatorList.end());

    //	std::cout<<"total operators before inserting gate operators : "<< resultOperatorList.size()<<std::endl;
        #ifndef SHARE_NO_OPERATOR
        insertGateOperators(resultOperatorList);
        #endif // SHARE_NO_OPERATOR
    //	std::cout<<"total operators in JsSpinner before sorting: "<< resultOperatorList.size()<<std::endl;
        //sort the list, leaf operators are first
        //printOperatorList(resultOperatorList);
        resultOperatorList = sortOperators(resultOperatorList);
        //printOperatorList(resultOperatorList);
    //	std::cout<<"total operators in JsSpinner after sorting: "<< resultOperatorList.size()<<std::endl;
        appendQueueId(resultOperatorList);

	//std::cout << "resultOperatorList.size " << resultOperatorList.size() << std::endl;
    //for (std::list<boost::shared_ptr<Operator> >::iterator opListItt=resultOperatorList.begin(); opListItt != resultOperatorList.end(); ++opListItt)
    //    std::cout << *opListItt << std::endl;

    queryMergeCounter++;
	return resultOperatorList;
}



void PlanManager::mergeOperators(std::list<boost::shared_ptr<Operator> >&resultOperatorList, std::list<boost::shared_ptr<Operator> >toOperatorList, std::list<boost::shared_ptr<Operator> >fromOperatorList)
{
	std::list<boost::shared_ptr<Operator> >outputOpofToOperator;
	std::list<boost::shared_ptr<Operator> >outputOpofFromOperator;
	std::list<boost::shared_ptr<Operator> >::iterator it;
	std::list<boost::shared_ptr<Operator> >::iterator itt;

	for(it = toOperatorList.begin(); it != toOperatorList.end(); it++)
	{
		for(itt = fromOperatorList.begin(); itt != fromOperatorList.end(); itt++)
		{
			boost::shared_ptr<Operator> toOp = *it;
			boost::shared_ptr<Operator> fromOp = *itt;

			if(typeid(*toOp) == typeid(LeafOperator)&&typeid(*fromOp) == typeid(LeafOperator))
			{
				boost::shared_ptr<LeafOperator> toLeafOp = boost::dynamic_pointer_cast<LeafOperator>(toOp);
				boost::shared_ptr<LeafOperator> fromLeafOp = boost::dynamic_pointer_cast<LeafOperator>(fromOp);
				if(isSameLeafOperatorLogic(toLeafOp,fromLeafOp))
				{
				//	std::cout<<"---------"<<std::endl;
				//	std::cout<<"two leaf operators are the same "<<std::endl;
					//std::cout<<"to leaf : " << toLeafOp<<std::endl;
					//std::cout<<"from leaf : "<< fromLeafOp<<std::endl;
					appendOutputOperators(outputOpofToOperator, toLeafOp);
					appendOutputOperators(outputOpofFromOperator, fromLeafOp);
					moveOutputQueues(toLeafOp,fromLeafOp);
					resultOperatorList.remove(fromLeafOp);

					/*
					std::list<boost::shared_ptr<QueryEntity> > queryEntityList = QueryManager::getInstance()->getQueryEntityList();
					for(queryEntityListIt = queryEntityList.begin(); queryEntityListIt!= queryEntityList.end(); queryEntityListIt++)
					{
						QueryEntity* queryEntity = *queryEntityListIt;
					}
					*/

					std::list<QueryEntity* >queryList = fromLeafOp->getRelatedQueries();
					std::list<QueryEntity* >::iterator ittt;

					//std::cout << "fromOp " << *itt << " : toOp " << *it << std::endl;
					//std::cout << "# fromLeafOp related quereis " << queryList.size() << std::endl;

					for(ittt = queryList.begin(); ittt!= queryList.end(); ittt++)
					{
						QueryEntity* queryEntity = *ittt;
						toLeafOp->addQuery(queryEntity);
						queryEntity->changeLeafOperator(fromLeafOp,toLeafOp);
					}
				}
			}
            #ifdef SHARE_RANGE_WINDOW
			else if(typeid(*toOp) == typeid(RangeWindowOperator)&&typeid(*fromOp) == typeid(RangeWindowOperator))
			{//if equal, the merged operator is a range window operator
			//	std::cout<<"---------"<<std::endl;
			//	std::cout<<"two operators are range window operators"<<std::endl;
				boost::shared_ptr<RangeWindowOperator> toRangeWindowOp = boost::dynamic_pointer_cast<RangeWindowOperator>(toOp);
				boost::shared_ptr<RangeWindowOperator> fromRangeWindowOp = boost::dynamic_pointer_cast<RangeWindowOperator>(fromOp);
				Timeunit toRangeWindowSize = toRangeWindowOp->getWindowSize();
				Timeunit fromRangeWindowSize = fromRangeWindowOp->getWindowSize();
				if(toRangeWindowSize == fromRangeWindowSize)
				{
			//		std::cout<<"---------"<<std::endl;
			//		std::cout<<"two common windows are the same "<<std::endl;
					appendOutputOperators(outputOpofToOperator, toRangeWindowOp);
					appendOutputOperators(outputOpofFromOperator, fromRangeWindowOp);
					moveOutputQueues(toRangeWindowOp,fromRangeWindowOp);
					resultOperatorList.remove(fromRangeWindowOp);
				}
			}
			#endif // SHARE_RANGE_WINDOW
			#ifdef SHARE_SMART_WINDOW
			else if(typeid(*toOp) == typeid(SmartRowWindowOperator) && typeid(*fromOp) == typeid(SmartRowWindowOperator))
			{//if equal, the merged operator is a smart window operator
				boost::shared_ptr<SmartRowWindowOperator> toSmartWindowOp = boost::dynamic_pointer_cast<SmartRowWindowOperator>(toOp);
				boost::shared_ptr<SmartRowWindowOperator> fromSmartWindowOp = boost::dynamic_pointer_cast<SmartRowWindowOperator>(fromOp);
				int toWindowSize = toSmartWindowOp->getWindowSize();
				int fromWindowSize = fromSmartWindowOp->getWindowSize();
				if(toWindowSize == fromWindowSize)
				{
				//	std::cout<<"---------"<<std::endl;
				//	std::cout<<"two smart row window operators are the same "<<std::endl;
					appendOutputOperators(outputOpofToOperator, toSmartWindowOp);
					appendOutputOperators(outputOpofFromOperator, fromSmartWindowOp);
					moveOutputQueues(toSmartWindowOp,fromSmartWindowOp);
					resultOperatorList.remove(fromSmartWindowOp);

					std::list<QueryEntity* >queryList = fromSmartWindowOp->getRelatedQueries();
					std::list<QueryEntity* >::iterator ittt;
					for(ittt = queryList.begin(); ittt!= queryList.end(); ittt++)
					{
						QueryEntity* queryEntity = *ittt;
						toSmartWindowOp->addQuery(queryEntity);
					}
				}
			}
			else if(typeid(*toOp) == typeid(RowWindowOperator)&&typeid(*fromOp) == typeid(SmartRowWindowOperator))
			{//if equal, the merged operator is a window operator
				boost::shared_ptr<RowWindowOperator> rowWindowOp = boost::dynamic_pointer_cast<RowWindowOperator>(toOp);
				boost::shared_ptr<SmartRowWindowOperator> smartRowWindowOp = boost::dynamic_pointer_cast<SmartRowWindowOperator>(fromOp);
				int rowWindowSize = rowWindowOp->getWindowSize();
				int smartRowWindowSize = smartRowWindowOp->getWindowSize();
				if(rowWindowSize == smartRowWindowSize)
				{
				//	std::cout<<"---------"<<std::endl;
				//	std::cout<<"a smart window and a common window are the same "<<std::endl;
					appendOutputOperators(outputOpofToOperator, rowWindowOp);
					appendOutputOperators(outputOpofFromOperator, smartRowWindowOp);
					moveOutputQueues(rowWindowOp,smartRowWindowOp);
					resultOperatorList.remove(smartRowWindowOp);

				}

			}
			else if(typeid(*toOp) == typeid(SmartRowWindowOperator)&&typeid(*fromOp) == typeid(RowWindowOperator))
			{//if equal, the merged operator is a window operator
				boost::shared_ptr<RowWindowOperator> rowWindowOp = boost::dynamic_pointer_cast<RowWindowOperator>(fromOp);
				boost::shared_ptr<SmartRowWindowOperator> smartRowWindowOp = boost::dynamic_pointer_cast<SmartRowWindowOperator>(toOp);
				int rowWindowSize = rowWindowOp->getWindowSize();
				int smartRowWindowSize = smartRowWindowOp->getWindowSize();
				if(rowWindowSize == smartRowWindowSize)
				{
				//	std::cout<<"---------"<<std::endl;
				//	std::cout<<"a smart window and a common window are the same "<<std::endl;
					appendOutputOperators(outputOpofToOperator, smartRowWindowOp);
					appendOutputOperators(outputOpofFromOperator, rowWindowOp);
					moveOutputQueues(rowWindowOp,smartRowWindowOp);
					resultOperatorList.remove(smartRowWindowOp);
				}

			}
			else if(typeid(*toOp) == typeid(RowWindowOperator)&&typeid(*fromOp) == typeid(RowWindowOperator))
			{//if equal, the merged operator is a row window operator
			//	std::cout<<"---------"<<std::endl;
			//	std::cout<<"two operators are row window operators"<<std::endl;
				boost::shared_ptr<RowWindowOperator> toRowWindowOp = boost::dynamic_pointer_cast<RowWindowOperator>(toOp);
				boost::shared_ptr<RowWindowOperator> fromRowWindowOp = boost::dynamic_pointer_cast<RowWindowOperator>(fromOp);
				int toRowWindowSize = toRowWindowOp->getWindowSize();
				int fromRowWindowSize = fromRowWindowOp->getWindowSize();
				if(toRowWindowSize == fromRowWindowSize)
				{
			//		std::cout<<"---------"<<std::endl;
			//		std::cout<<"two common windows are the same "<<std::endl;
					appendOutputOperators(outputOpofToOperator, toRowWindowOp);
					appendOutputOperators(outputOpofFromOperator, fromRowWindowOp);
					moveOutputQueues(toRowWindowOp,fromRowWindowOp);
					resultOperatorList.remove(fromRowWindowOp);
				}
			}
			#endif // SHARE_SMART_WINDOW
			else if(typeid(*toOp) == typeid(SelectionOperator)&&typeid(*fromOp) == typeid(SelectionOperator))
			{
			//		std::cout<<"---------"<<std::endl;
					boost::shared_ptr<SelectionOperator> toSelectionOp = boost::dynamic_pointer_cast<SelectionOperator>(toOp);
					boost::shared_ptr<SelectionOperator> fromSelectionOp = boost::dynamic_pointer_cast<SelectionOperator>(fromOp);
					if(1)// wangyan, needs to be done
					{
			//			std::cout<<"---------"<<std::endl;
			//			std::cout<<"two selection operators are the same "<<std::endl;

						appendOutputOperators(outputOpofToOperator, toSelectionOp);
						appendOutputOperators(outputOpofFromOperator, fromSelectionOp);
						moveOutputQueues(toSelectionOp,fromSelectionOp);
						resultOperatorList.remove(fromSelectionOp);
					}


			}
			else if(typeid(*toOp) == typeid(ProjectionOperator)&&typeid(*fromOp) == typeid(ProjectionOperator))
			{
			//		std::cout<<"---------"<<std::endl;
					boost::shared_ptr<ProjectionOperator> toOperator = boost::dynamic_pointer_cast<ProjectionOperator>(toOp);
					boost::shared_ptr<ProjectionOperator> fromOperator = boost::dynamic_pointer_cast<ProjectionOperator>(fromOp);
					if(1)// wangyan, needs to be done
					{
			//			std::cout<<"---------"<<std::endl;
			//			std::cout<<"two projection operators are the same "<<std::endl;

						appendOutputOperators(outputOpofToOperator, toOperator);
						appendOutputOperators(outputOpofFromOperator, fromOperator);
						moveOutputQueues(toOperator,fromOperator);
						resultOperatorList.remove(fromOperator);
					}
			}
			else if(typeid(*toOp) == typeid(JoinOperator)&&typeid(*fromOp) == typeid(JoinOperator))
			{
			//		std::cout<<"---------"<<std::endl;
					//need to be done , wangyan
			}

		}
	}
	if(outputOpofToOperator.size()>0)
	{
		mergeOperators(resultOperatorList,outputOpofToOperator,outputOpofFromOperator);
	}
}

void PlanManager::appendOutputOperators(std::list<boost::shared_ptr<Operator> >&opList, boost::shared_ptr<Operator>op)
{
	std::list<boost::shared_ptr<QueueEntity> >outputQueueList = op->getOutputQueueList();
	std::list<boost::shared_ptr<QueueEntity> >::iterator it ;
	for(it = outputQueueList.begin(); it != outputQueueList.end() ; it++)
	{
		boost::shared_ptr<QueueEntity> queueEntity = *it;
		boost::shared_ptr<Operator> outputOp = queueEntity->getOutputOperator();
		opList.push_back(outputOp);
	}
}

bool PlanManager::isSameLeafOperatorLogic(boost::shared_ptr<LeafOperator> op1, boost::shared_ptr<LeafOperator> op2)
{
	boost::shared_ptr<IStreamInput> streamInput1 = op1->getStreamInput();
	std::string streamId1 = streamInput1->getId();
	boost::shared_ptr<IStreamInput> streamInput2 = op2->getStreamInput();
	std::string streamId2 = streamInput2->getId();
	return streamId1.compare(streamId2)==0;
}

void PlanManager::insertGateOperators(std::list<boost::shared_ptr<Operator> >& operatorList)
{
	std::list<boost::shared_ptr<Operator> >::iterator it;
	for(it = operatorList.begin(); it != operatorList.end(); it++)
	{
		boost::shared_ptr<Operator> op = *it;

        // If gate operator not already appended to an operator, then may be appended
		if(std::find(gateOpAppendedToOpList.begin(), gateOpAppendedToOpList.end(), op) == gateOpAppendedToOpList.end())
		{
            int outputQueueNumber = op->getOutputQueueList().size();
            //std::cout << "outputQueueNumber in PlanManager " << outputQueueNumber << std::endl;
            if(outputQueueNumber > 1)
            {
                //std::cout << "op to append gate " << op << std::endl;
                appendGateOperator(op, operatorList);
            }
		}
	}
}

void PlanManager::appendGateOperator(boost::shared_ptr<Operator> op,std::list<boost::shared_ptr<Operator> >& operatorList)
{
	std::list<boost::shared_ptr<QueueEntity> >outputQueueList = op->getOutputQueueList();
	std::list<boost::shared_ptr<QueueEntity> >::iterator it;
	std::list<boost::shared_ptr<RootOperator> >rootOperatorList;
	std::list<boost::shared_ptr<LeafOperator> >leafOperatorList;
	std::list<boost::shared_ptr<RootOperator> >::iterator itt;
//	std::cout<<"&&&&&&&&&&&&&"<<std::endl;
	for(it = outputQueueList.begin() ; it != outputQueueList.end(); it ++)
	{
		boost::shared_ptr<QueueEntity> queueEntity = *it;
		boost::shared_ptr<Operator> outputOperator = queueEntity->getOutputOperator();
		boost::shared_ptr<Operator> gateOperator(new GateOperator());
		boost::shared_ptr<QueueEntity> newQueueEntity (new QueueEntity());
		rootOperatorList = getRootOperatorList (outputOperator);
		leafOperatorList = getLeafOperatorList(op);
		//
//		std::cout<<"~~~~~~~~~~~"<<std::endl;
		std::set<QueryEntity*> queryEntitySet;
		for(itt = rootOperatorList.begin(); itt!= rootOperatorList.end(); itt++)
		{
			boost::shared_ptr<RootOperator>rootOperator = *itt;
			queryEntitySet.insert(rootOperator->getQueryEntity());
//			std::cout<<"root operator related query : " <<rootOperator->getQueryEntity()<<std::endl;
		}


		bool isAllLeafOperatorMaster = true;
		std::list<boost::shared_ptr<LeafOperator> >::iterator leafIt;
		for(leafIt = leafOperatorList.begin() ; leafIt != leafOperatorList.end() ; leafIt++)
		{
			boost::shared_ptr<LeafOperator> leafOp = *leafIt;
//			std::cout<<"a leaf operator " << leafOp->getId()<<std::endl;
			std::list<QueryEntity* > relatedQueries = leafOp->getRelatedQueries();
			std::list<QueryEntity* >::iterator queryIt;
			for(queryIt = relatedQueries.begin(); queryIt != relatedQueries.end() ; queryIt++)
			{
				QueryEntity* queryEntity = *queryIt;
//				std::cout<<"a query related with leaf " << queryEntity<<std::endl;
				if(queryEntitySet.find(queryEntity)!= queryEntitySet.end())
				{
					bool masterTag = queryEntity->getMasterTag(&*leafOp);
//					std::cout<<"its master mark " <<masterTag<<std::endl;
					if(masterTag == false)
					{
						isAllLeafOperatorMaster = false;
//						std::cout<<"not satisfied"<<std::endl;
						break;
					}
				}
			}
			if(isAllLeafOperatorMaster == false)
			{
//				std::cout<<"not satisfied"<<std::endl;
				break;
			}
		}
		if(isAllLeafOperatorMaster == true)
		{
//			std::cout<<"ignore inserting one gate operator "<<std::endl;
			continue;
		}
		else
		{
			//std::cout<<"a gate operato is inserted"<<std::endl;
		}

		queueEntity->setOutputOperator(gateOperator);
		gateOperator->addInputQueue(queueEntity);

		gateOperator->addOutputQueue(newQueueEntity);
		newQueueEntity->setInputOperator(gateOperator);

		newQueueEntity->setOutputOperator(outputOperator);
		outputOperator->removeInputQueue(queueEntity);
		outputOperator->addInputQueue(newQueueEntity);
		operatorList.push_back(gateOperator);
		gateOpAppendedToOpList.push_back(op);
		//std::cout << "gateOperator " << gateOperator << std::endl;
		for(itt = rootOperatorList.begin(); itt!= rootOperatorList.end(); itt++)
		{
			boost::shared_ptr<RootOperator>rootOperator = *itt;
			boost::shared_ptr<GateOperator>gateOp = boost::dynamic_pointer_cast<GateOperator>(gateOperator);
		        gateOp->addQueryEntity(rootOperator->getQueryEntity());

		}
	}
}
std::list<boost::shared_ptr<LeafOperator> > PlanManager::getLeafOperatorList(boost::shared_ptr<Operator> op)
{
	std::list<boost::shared_ptr<LeafOperator> >resultList;
	std::list<boost::shared_ptr<QueueEntity> >queueEntityList = op->getInputQueueList();
	std::list<boost::shared_ptr<QueueEntity> >::iterator it;
	for(it = queueEntityList.begin(); it != queueEntityList.end(); it++)
	{
		boost::shared_ptr<QueueEntity > queueEntity = *it;
		boost::shared_ptr<Operator> op = queueEntity->getInputOperator();
		if(typeid(*op) == typeid(LeafOperator))
		{
			boost::shared_ptr<LeafOperator> leafOp = boost::dynamic_pointer_cast<LeafOperator>(op);
			resultList.push_back(leafOp);
		}
		else
		{
			std::list<boost::shared_ptr<LeafOperator> > opList = getLeafOperatorList(op);
			resultList.insert(resultList.end(),opList.begin(), opList.end());
		}
	}

	return resultList;

}
std::list<boost::shared_ptr<RootOperator> > PlanManager::getRootOperatorList(boost::shared_ptr<Operator> op)
{
	std::list<boost::shared_ptr<RootOperator> >resultList;
	std::list<boost::shared_ptr<QueueEntity> >queueEntityList = op->getOutputQueueList();
	std::list<boost::shared_ptr<QueueEntity> >::iterator it;
	for(it = queueEntityList.begin(); it != queueEntityList.end(); it++)
	{
		boost::shared_ptr<QueueEntity > queueEntity = *it;
		boost::shared_ptr<Operator> op = queueEntity->getOutputOperator();
		if(typeid(*op) == typeid(RootOperator))
		{
			boost::shared_ptr<RootOperator> rootOp = boost::dynamic_pointer_cast<RootOperator>(op);
			resultList.push_back(rootOp);
		}
		else
		{
			std::list<boost::shared_ptr<RootOperator> > opList = getRootOperatorList(op);
			resultList.insert(resultList.end(),opList.begin(), opList.end());
		}
	}

	return resultList;
}
std::list<boost::shared_ptr<Operator> >PlanManager::getAllLeafOperators(std::list<boost::shared_ptr<Operator> >operatorList)
{
	std::list<boost::shared_ptr<Operator> >resultOperatorList;
	std::list<boost::shared_ptr<Operator> >::iterator it;
	for(it = operatorList.begin(); it!= operatorList.end(); it++)
	{
		boost::shared_ptr<Operator> op = *it;
		if(typeid(*op) == typeid(LeafOperator) )
		{
			resultOperatorList.push_back(op);
		}
	}
	return resultOperatorList;
}
void PlanManager::moveOutputQueues(boost::shared_ptr<Operator>  operatorTo, boost::shared_ptr<Operator> operatorFrom)
{

	std::list<boost::shared_ptr<QueueEntity> >outputQueueList = operatorFrom->getOutputQueueList();
	std::list<boost::shared_ptr<QueueEntity> >::iterator it ;
	for(it = outputQueueList.begin(); it != outputQueueList.end() ; it++)
	{
		boost::shared_ptr<QueueEntity> queueEntity = *it;
		operatorTo->addOutputQueue(queueEntity);
		queueEntity->setInputOperator(operatorTo);
	}
	std::list<boost::shared_ptr<QueueEntity> >inputQueueList = operatorFrom->getInputQueueList();
	for(it = inputQueueList.begin(); it != inputQueueList.end(); it++)
	{
		boost::shared_ptr<QueueEntity> queueEntity = *it;
		boost::shared_ptr<Operator> op = queueEntity->getInputOperator();
		op->removeOutputQueue(queueEntity);
	}
}
//std::list<boost::shared_ptr<Operator> > PlanManager::getAllOperators(void)
//{
//	std::list<boost::shared_ptr<Operator> > operatorList;
//	std::list<boost::shared_ptr<QueryPlanTree> >::iterator it;
//	for(it = this->queryPlanTreeList.begin(); it != this->queryPlanTreeList.end(); it++)
//	{
//		boost::shared_ptr<QueryPlanTree> queryPlanTree = *it;
//		std::list<boost::shared_ptr<Operator> >opList = queryPlanTree->getAllOperators();
//		operatorList.splice(operatorList.end(), opList);
//	}
//	//sharing leaf operators, replace the new leaf operators with the existing one
//	if(this->queryPlanTreeList.size() > 1)
//	{
//		std::map<std::string, boost::shared_ptr<LeafOperator> >leafOperatorMap;
//		std::list<boost::shared_ptr<QueryPlanTree> >::iterator it;
//		for(it = this->queryPlanTreeList.begin(); it != this->queryPlanTreeList.end(); it++)
//		{
//			boost::shared_ptr<QueryPlanTree> queryPlanTree = *it;
//			std::list<boost::shared_ptr<Operator> >opList = queryPlanTree->getAllOperators();
//			std::list<boost::shared_ptr<Operator> >::iterator itt;
//			for (itt = opList.begin(); itt != opList.end() ; )
//			{
//				boost::shared_ptr<Operator> op = *itt;
//				if(typeid(*op) == typeid(LeafOperator))
//				{
//					boost::shared_ptr<LeafOperator> leafOperator = boost::dynamic_pointer_cast<LeafOperator>(op);
//					boost::shared_ptr<IStreamInput> streamInput = leafOperator->getStreamInput();
//					std::string streamInputId= streamInput->getId();
//					std::map<std::string, boost::shared_ptr<LeafOperator> >::iterator ittt;
//					ittt = leafOperatorMap.find(streamInputId);
//					if(ittt != leafOperatorMap.end())
//					{//replace with exist operator
//						boost::shared_ptr<LeafOperator> existLeafOperator = ittt->second;
//						existLeafOperator->addOutputQueue(leafOperator->getOutputQueueList().front());
//						leafOperator->getOutputQueueList().front()->setInputOperator(existLeafOperator);
//						std::list<QueryEntity* >queryList = leafOperator->getRelatedQueries();
//						std::list<QueryEntity* >::iterator itttt;
//						for(itttt = queryList.begin(); itttt!= queryList.end(); itttt++)
//						{
//							QueryEntity* queryEntity = *itttt;
//
//							existLeafOperator->addQuery(queryEntity);
//							queryEntity->changeLeafOperator(leafOperator,existLeafOperator);
//						}
//						itt = opList.erase(itt);
//						operatorList.remove(leafOperator);
//					}
//					else
//					{
//						itt++;
//						leafOperatorMap.insert(make_pair(streamInputId,leafOperator));
//					}
//				}
//				else
//				{
//					itt++;
//				}
//			}
//		}
//
//	}
//
//	//std::cout<<"total operators in the system : "  << operatorList.size()<<std::endl;
//	return operatorList;
//}
void PlanManager::createQueryPlan(boost::shared_ptr<QueryEntity>queryEntity)
{

	boost::shared_ptr<RootOperator> rootOperator = queryEntity->getRootOperator();

	boost::shared_ptr<QueryPlanTree> queryPlanTree(new QueryPlanTree(rootOperator));
	this->queryPlanTreeList.push_back(queryPlanTree);

	stringstream ss;
	ss<<this->queryPlanTreeList.size();
	queryPlanTree->setOperatorIdAndQueueId(ss.str()+"_");


}
std::list<boost::shared_ptr<Operator> > PlanManager::sortOperators(std::list<boost::shared_ptr<Operator> >operatorList)
{
	std::list<boost::shared_ptr<Operator> > rootOperatorList;
	std::list<boost::shared_ptr<Operator> >::iterator it ;
	for(it = operatorList.begin(); it != operatorList.end(); it++)
	{
		boost::shared_ptr<Operator> op = *it;
		if(typeid(*op) == typeid(RootOperator))
		{
			rootOperatorList.push_back(op);
		}
	}
	std::list<boost::shared_ptr<Operator> >resultOperatorList;
	for(it = rootOperatorList.begin(); it != rootOperatorList.end(); it++)
	{
		std::list<boost::shared_ptr<Operator> > opList = traversePostOrder(*it);
		std::list<boost::shared_ptr<Operator> >::iterator beginIt;
		std::list<boost::shared_ptr<Operator> >::iterator endIt;
		beginIt = opList.begin();
		for(endIt = opList.begin(); endIt != opList.end(); )
		{
			boost::shared_ptr<Operator> currentOperator = *endIt;
			std::list<boost::shared_ptr<Operator> >::iterator resultIt;
			resultIt = find (resultOperatorList.begin(), resultOperatorList.end(), currentOperator);
			if(resultIt != resultOperatorList.end())
			{
				resultOperatorList.insert(resultIt,beginIt,endIt);
				endIt++;
				beginIt = endIt;

			}
			else
			{
				endIt++;
			}
		}
		resultOperatorList.insert(resultOperatorList.end(), beginIt, endIt);

	}
	return resultOperatorList;
}
void PlanManager::printOperatorList(std::list<boost::shared_ptr<Operator> >operatorList)
{
//	std::cout<<"--------------"<<std::endl;
	std::list<boost::shared_ptr<Operator> >::iterator it;
	for( it = operatorList.begin(); it!= operatorList.end() ; it++)
	{
		//std::cout<<(*it)->getId()<<"  ";
	}
//	std::cout<<std::endl;
//	std::cout<<"--------------"<<std::endl;
}
std::list<boost::shared_ptr<Operator> > PlanManager::traversePostOrder(boost::shared_ptr<Operator> op)
{
	std::list<boost::shared_ptr<Operator> > opList;
	std::list<boost::shared_ptr<QueueEntity> > queueList;
	queueList = op->getInputQueueList();
	std::list<boost::shared_ptr<QueueEntity> >::iterator it;
	for(it = queueList.begin(); it != queueList.end(); it++)
	{
		boost::shared_ptr<QueueEntity> queueEntity = *it;
		boost::shared_ptr<Operator> tempOp = queueEntity->getInputOperator();
		std::list<boost::shared_ptr<Operator> > tempOpList = traversePostOrder(tempOp);
		opList.insert(opList.end(), tempOpList.begin(), tempOpList.end());
	}
	opList.push_back(op);
	return opList;


}
void PlanManager::appendQueueId(std::list<boost::shared_ptr<Operator> >operatorList)
{
	int queueNumber = 1;
	std::list<boost::shared_ptr<Operator> >::iterator it;
	for(it = operatorList.begin(); it!= operatorList.end(); it++)
	{
		boost::shared_ptr<Operator> op = *it;
		std::list<boost::shared_ptr<QueueEntity> >outputQueueList = op->getOutputQueueList();
		std::list<boost::shared_ptr<QueueEntity> >::iterator itt;
		for(itt = outputQueueList.begin(); itt!= outputQueueList.end(); itt++)
		{
			boost::shared_ptr<QueueEntity> queueEntity = *itt;
			queueEntity->setId("" );
		}
	}
	for(it = operatorList.begin(); it!= operatorList.end(); it++)
	{
		boost::shared_ptr<Operator> op = *it;
		std::list<boost::shared_ptr<QueueEntity> >outputQueueList = op->getOutputQueueList();
		std::list<boost::shared_ptr<QueueEntity> >::iterator itt;
		for(itt = outputQueueList.begin(); itt!= outputQueueList.end(); itt++)
		{
			boost::shared_ptr<QueueEntity> queueEntity = *itt;
			stringstream ss;
			ss<<queueNumber++;
			queueEntity->setId("queue" + ss.str());
		}
	}
}
