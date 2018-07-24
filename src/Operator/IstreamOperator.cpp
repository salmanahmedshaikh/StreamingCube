//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Operator/IstreamOperator.h"
#include "../Operator/Operator.h"
#include "../Internal/Queue/QueueEntity.h"
#include "../Internal/Synopsis/LineageSynopsis.h"
#include "../Internal/Synopsis/RelationSynopsis.h"
#include "../Operator/Operator.h"
#include "../Utility/TimeCounterUtility.h"
#include "../Internal/Element/ElementIterator.h"
#include <boost/shared_ptr.hpp>
#include "../Scheduler/Scheduler.h"
IstreamOperator::IstreamOperator(void)
{
	this->relationSynopsis.reset(new RelationSynopsis());
	this->lastInputTimestamp = 0;
	this->elementIterator.reset(new ElementIterator(this->relationSynopsis));
}


IstreamOperator::~IstreamOperator(void)
{

}

void IstreamOperator::execution()
{

#ifdef DEBUG
	std::cout<<"===================operator begin================="<<std::endl;
	std::cout<<"operatorid : "<<this->getId()<<std::endl;
#endif
	assert(this->getInputQueueList().size()==1);
	assert(this->getOutputQueueList().size()==1);
	boost::shared_ptr<QueueEntity>inputQueue = this->getInputQueueList().front();

	Element inputElement;
	while(1)
	{
		if(inputQueue->isEmpty())
		{

			break;
		}

		TimeCounterUtility::getInstance()->start();
		inputQueue->dequeue(inputElement);

		//std::cout<<inputElement<<std::endl;

		if(lastInputTimestamp<inputElement.timestamp )
		{

		}
		if(inputElement.mark==PLUS_MARK)
		{
			this->relationSynopsis->insertElement(inputElement);
		}
		else if(inputElement.mark == MINUS_MARK)
		{
			this->relationSynopsis->deleteElement(inputElement);
		}
		if(inputElement.masterTag == true)
		{
			produceOutput();

		}
		lastInputTimestamp = inputElement.timestamp;
		TimeCounterUtility::getInstance()->pause();
	}

#ifdef DEBUG
	std::cout<<"===================operator over================="<<std::endl;
#endif
}
void IstreamOperator::produceOutput()
{

	this->elementIterator->initial();
	Element elementInSynopsis;
	while(this->elementIterator->getNext(elementInSynopsis))
	{
		Element outputElement;
		outputElement.document = elementInSynopsis.document;
		outputElement.id = elementInSynopsis.id;
		outputElement.mark = PLUS_MARK;
		outputElement.timestamp = lastInputTimestamp;
		outputElement.masterTag = elementInSynopsis.masterTag;
		output(outputElement);
		this->relationSynopsis->deleteElement(elementInSynopsis);
//		this->elementIterator->erase();
	}

}
