//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Operator/RowWindowOperator.h"
#include "../Internal/Synopsis/WindowSynopsis.h"
#include "../Utility/TimeCounterUtility.h"
#include "../Internal/Queue/QueueEntity.h"
#include "../Scheduler/Scheduler.h"
RowWindowOperator::RowWindowOperator(int rowWindowSize):rowWindowSize(rowWindowSize)
{
    //this->rowWindowSize = rowWindowSize;
    //std::cout << "rowWindowSize " << this->rowWindowSize << std::endl;
	this->windowSynopsis.reset(new WindowSynopsis());
	this->currentRowNumberInSynopsis = 0;
	this->lastInputTimestamp = 0;
}


RowWindowOperator::~RowWindowOperator(void)
{
}

int RowWindowOperator::getWindowSize()
{
	return this->rowWindowSize;
}

void RowWindowOperator::execution()
{
#ifdef DEBUG
	std::cout<<"===================operator begin================="<<std::endl;
	std::cout<<"operatorid : "<<this->getId()<<std::endl;
#endif
	assert(this->getInputQueueList().size()==1);
	assert(this->getOutputQueueList().size()==1);
	boost::shared_ptr<QueueEntity>inputQueue = this->getInputQueueList().front();
	boost::shared_ptr<QueueEntity>outputQueue = this->getOutputQueueList().front();

	while(1)
	{
		Element inputElement;

		if(inputQueue->isEmpty())   // the input queue is not empty
		{
			break;
		}
		if(outputQueue->isFull())   // the output queue is not full
		{
			break;
		}
		TimeCounterUtility::getInstance()->start();
		inputQueue->dequeue(inputElement);
		//std::cout<<inputElement<<std::endl;

		assert(inputElement.mark==PLUS_MARK);
		assert(lastInputTimestamp<= inputElement.timestamp);

		lastInputTimestamp = inputElement.timestamp;

		if(this->windowSynopsis->isFull())   // the window synopsis is not full
		{
			throw std::runtime_error("synopsis full");
		}

		this->windowSynopsis->insertElement(inputElement);
		this->currentRowNumberInSynopsis++;
		assert(!outputQueue->isFull());
		outputQueue->enqueue(inputElement);//forward the same element

		Element outputElement;
		if(this->currentRowNumberInSynopsis > this->rowWindowSize)
		{   // generate oldest element out
			this->windowSynopsis->getOldestElement(outputElement);
			this->windowSynopsis->deleteOldestElement();
			this->currentRowNumberInSynopsis--;
			outputElement.mark = MINUS_MARK; // give MINUS_MARK to the element to be deleted
			outputElement.timestamp = inputElement.timestamp; // give new timestamp to the element to be deleted
			outputElement.masterTag = inputElement.masterTag;
			//if output queue is full, change the state to blocked and buffer the element;

			assert(!outputQueue->isFull());
			outputQueue->enqueue(outputElement);
		}
		TimeCounterUtility::getInstance()->pause();
	}
#ifdef DEBUG
	std::cout<<"===================operator over================="<<std::endl;
#endif
}
