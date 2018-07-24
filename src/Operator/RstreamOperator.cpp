//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Operator/RstreamOperator.h"
#include "../Internal/Queue/QueueEntity.h"
#include "../Utility/TimeCounterUtility.h"
#include "../Utility/DocumentIdentifierGenerator.h"
#include "../Scheduler/Scheduler.h"
RstreamOperator::RstreamOperator(void)
{
	this->relationSynopsis.reset(new RelationSynopsis());
	this->lastInputTimestamp = 0;
	this->elementIterator.reset(new ElementIterator(this->relationSynopsis));
}


RstreamOperator::~RstreamOperator(void)
{
}


void RstreamOperator::execution()
{
#ifdef DEBUG
	std::cout<<"===================operator begin================="<<std::endl;
	std::cout<<"operatorid : "<<this->getId()<<std::endl;
#endif
	assert(this->getInputQueueList().size()==1);
	assert(this->getOutputQueueList().size()==1);
	boost::shared_ptr<QueueEntity>inputQueue = this->getInputQueueList().front();
	boost::shared_ptr<QueueEntity>outputQueue = this->getOutputQueueList().front();

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
		for(;lastInputTimestamp<inputElement.timestamp;lastInputTimestamp++)
		{
			produceOutput();
			assert(!outputQueue->isFull());
		}


		if(inputElement.mark==PLUS_MARK)
		{
			this->relationSynopsis->insertElement(inputElement);
		}
		else if(inputElement.mark==MINUS_MARK)
		{
			this->relationSynopsis->deleteElement(inputElement);
		}


	}


#ifdef DEBUG
	std::cout<<"===================operator over================="<<std::endl;
#endif
}
void RstreamOperator::produceOutput()
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
	}
	assert(!outputQueue->isFull());
}
