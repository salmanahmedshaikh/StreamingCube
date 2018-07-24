//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Operator/DstreamOperator.h"
#include "../Operator/Operator.h"
#include "../Internal/Queue/QueueEntity.h"
#include "../Internal/Synopsis/LineageSynopsis.h"
#include "../Internal/Synopsis/RelationSynopsis.h"
#include "../Operator/Operator.h"
#include "../Internal/Element/ElementIterator.h"
#include <boost/shared_ptr.hpp>
#include "../Scheduler/Scheduler.h"
#include "../Utility/TimeCounterUtility.h"
DstreamOperator::DstreamOperator(void)
{
	this->relationSynopsis.reset(new RelationSynopsis());
	this->lastInputTimestamp = 0;
	this->elementIterator.reset(new ElementIterator(this->relationSynopsis));
}


DstreamOperator::~DstreamOperator(void)
{
}


void DstreamOperator::execution()
{

#ifdef DEBUG
	std::cout<<"===================operator begin================="<<std::endl;
	std::cout<<"operatorid : "<<this->getId()<<std::endl;
#endif
	assert(this->getInputQueueList().size()==1);
	assert(this->getOutputQueueList().size()==1);
	boost::shared_ptr<QueueEntity>inputQueue = this->getInputQueueList().front();
	boost::shared_ptr<QueueEntity>outputQueue = this->getOutputQueueList().front();
	std::cout<<"executing dstream operator"<<std::endl;
	Element inputElement;
	while(1)
	{
		if(outputQueue->isFull())
		{
			break;
		}
		if(inputQueue->isEmpty())
		{

			break;
		}
		inputQueue->dequeue(inputElement);
		if(lastInputTimestamp<inputElement.timestamp)
		{
			produceOutput();
		}

		if(inputElement.mark==MINUS_MARK)
		{
			this->relationSynopsis->insertElement(inputElement);
		}
		lastInputTimestamp = inputElement.timestamp;
	}

#ifdef DEBUG
	std::cout<<"===================operator over================="<<std::endl;
#endif
}
void DstreamOperator::produceOutput()
{

	boost::shared_ptr<QueueEntity>outputQueue = this->getOutputQueueList().front();
	this->elementIterator->initial();
	Element elementInSynopsis;
	while((!outputQueue->isFull())&&this->elementIterator->getNext(elementInSynopsis))
	{
		Element outputElement;
		outputElement.document = elementInSynopsis.document;
		outputElement.id = elementInSynopsis.id;
		outputElement.mark = PLUS_MARK;
		outputElement.timestamp = lastInputTimestamp;
		outputQueue->enqueue(outputElement);
		this->relationSynopsis->deleteElement(elementInSynopsis);
//		this->elementIterator->erase();
	}

	assert(!outputQueue->isFull());

}
