//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include <sstream>
#include "../Operator/GateOperator.h"

#include "../Operator/ProjectionOperator.h"
#include "../Utility/TimeCounterUtility.h"
#include "../Internal/Queue/QueueEntity.h"
#include "../Utility/DocumentIdentifierGenerator.h"
#include "../Scheduler/Scheduler.h"
#include "../Query/QueryProjectionAbstract.h"
#include "../Internal/Element/ElementIterator.h"
int GateOperator::gateOperatorNumber = 0;
int GateOperator::deletedNumber = 0;
int GateOperator::totalPlusNumber = 0;
GateOperator::GateOperator()
{
	this->relationSynopsis.reset(new RelationSynopsis());
	gateOperatorNumber++;
	std::stringstream ss;
	ss<<gateOperatorNumber;
	setId ("gateOperator"+ss.str());
}


GateOperator::~GateOperator()
{
}


void GateOperator::execution()
{
#ifdef DEBUG
	std::cout<<"===================operator begin================="<<std::endl;
	std::cout<<"operatorid : "<<this->getId()<<std::endl;
#endif
	assert(this->getInputQueueList().size()==1);
	assert(this->getOutputQueueList().size()==1);
	boost::shared_ptr<QueueEntity>inputQueue = this->getInputQueueList().front();
	bool ret;
	while(1)
	{
		Element inputElement;
		if(inputQueue->isEmpty())
		{
			break;
		}
		TimeCounterUtility::getInstance()->start();
		inputQueue->dequeue(inputElement);

		if(inputElement.mark == PLUS_MARK)
		{
			totalPlusNumber++;
			relationSynopsis->insertElement(inputElement);
		}
		else if(inputElement.mark  == MINUS_MARK)
		{
			if((relationSynopsis->deleteElement(inputElement) )== false)
			{
				output(inputElement);
			}
			else
			{
				deletedNumber++;
			}
		}
		bool isQueryActive = false;
		std::list<QueryEntity* >::iterator it;
		for( it = this->queryEntityList.begin(); it != this->queryEntityList.end(); it++)
		{
			QueryEntity* queryEntity = *it;
			if (queryEntity->isActive(inputElement.timestamp))
			{
				isQueryActive = true;
			}
		}
		if(isQueryActive == true)
		{
			ElementIterator it(this->relationSynopsis);
			it.initial();
			Element element;
			if(it.getNext(element))
			{
				output(element);
			}
			this->relationSynopsis->clear();
		}
		TimeCounterUtility::getInstance()->pause();
	}
#ifdef DEBUG
	std::cout<<"===================operator over================="<<std::endl;
#endif

}
void GateOperator::addQueryEntity(QueryEntity* queryEntity)
{
	this->queryEntityList.push_back(queryEntity);
}
