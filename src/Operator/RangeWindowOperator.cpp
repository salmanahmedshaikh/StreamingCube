//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Operator/RangeWindowOperator.h"
#include "../Internal/Queue/QueueEntity.h"
#include "../Utility/TimeCounterUtility.h"
#include "../Scheduler/Scheduler.h"
#include "../Utility/TimestampGenerator.h"

RangeWindowOperator::RangeWindowOperator(Timeunit timeRange):rangeWindowSize(timeRange)
{
	this->windowSynopsis.reset(new WindowSynopsis());
	this->lastInputTimestamp = 0;
	this->oldestElementTimestampInWinSynopsis = TimestampGenerator::getCurrentTime();
	//std::cout << "this->rangeWindowSize " << this->rangeWindowSize << std::endl;
}

RangeWindowOperator::~RangeWindowOperator(void)
{
}

Timeunit RangeWindowOperator::getWindowSize()
{
	return this->rangeWindowSize;
}

void RangeWindowOperator::execution()
{
#ifdef DEBUG
	std::cout<<"===================operator begin================="<<std::endl;
	std::cout<<"operatorid : "<<this->getId()<<std::endl;
#endif
	assert(this->getInputQueueList().size()==1);
	assert(this->getOutputQueueList().size()==1);
	boost::shared_ptr<QueueEntity>inputQueue = this->getInputQueueList().front();
	boost::shared_ptr<QueueEntity>outputQueue = this->getOutputQueueList().front();

	// Expire elements whose (currentTimestamp - ElementTimestamp) > rangeWindowSize
    if((TimestampGenerator::getCurrentTime() - this->oldestElementTimestampInWinSynopsis)/1000000.000000 >= this->rangeWindowSize)
    {
        //oldestElementTimestampInWinSynopsis = inputElement.timestamp;
        //std::cout << "time diff " << (TimestampGenerator::getCurrentTime() - this->oldestElementTimestampInWinSynopsis)/1000000.000000 << std::endl;
        expireElement();
    }

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
		//TimeCounterUtility::getInstance()->start();
		inputQueue->dequeue(inputElement);

		assert(inputElement.mark == PLUS_MARK);
		assert(lastInputTimestamp <= inputElement.timestamp);
		lastInputTimestamp = inputElement.timestamp;

		if(this->windowSynopsis->isFull())   // the window synopsis is not full
		{
			throw runtime_error("synopsis is full");
		}

		this->windowSynopsis->insertElement(inputElement);
		//std::cout << inputElement << std::endl;
		//std::cout << "windowSynopsis->getElementNumber " << this->windowSynopsis->getElementNumber() << std::endl;


		//std::cout << "TimestampGenerator::getCurrentTime() " << TimestampGenerator::getCurrentTime() << std::endl;
		//std::cout << "oldestElementTimestampInWinSynopsis " << this->oldestElementTimestampInWinSynopsis << std::endl;
		//std::cout << "(TimestampGenerator::getCurrentTime() - this->oldestElementTimestampInWinSynopsis)/1000000 " << (TimestampGenerator::getCurrentTime() - this->oldestElementTimestampInWinSynopsis)/1000000.000000 << std::endl;

        /*
		if(lastInputTimestamp >= this->rangeWindowSize)
		{  // generate oldest element out
			expireElement( lastInputTimestamp - rangeWindowSize, inputElement); //expire tuples with timestamp <= lastInputTimestamp - rangeWindowSize
		}
		*/

		assert(!outputQueue->isFull());
		outputQueue->enqueue(inputElement);//forward the same element
		//TimeCounterUtility::getInstance()->pause();
	}
#ifdef DEBUG
	std::cout<<"===================operator over================="<<std::endl;
#endif
}

//void RangeWindowOperator::expireElement(Timestamp ts, Element& inputElement)//expire elements with a timestamp older than ts
//void RangeWindowOperator::expireElement(Element& inputElement)//expire elements with a timestamp older than ts
void RangeWindowOperator::expireElement()
{
	boost::shared_ptr<QueueEntity>outputQueue = this->getOutputQueueList().front();
	Element elementInSynopsis;
	Element elementOutput;

	while(true)
	{
		if (this->windowSynopsis -> isEmpty())
		{
			return;
		}

		this->windowSynopsis->getOldestElement(elementInSynopsis);
		this->oldestElementTimestampInWinSynopsis = elementInSynopsis.timestamp;

        // If oldest element is not yet expired, return from the function
        if((TimestampGenerator::getCurrentTime() - elementInSynopsis.timestamp)/1000000.000000 < this->rangeWindowSize)
        {
            return;
        }

		assert(!outputQueue->isFull());
		elementOutput.document = elementInSynopsis.document;
		elementOutput.id = elementInSynopsis.id;
		elementOutput.mark = MINUS_MARK;
		elementOutput.timestamp = elementInSynopsis.timestamp + this->rangeWindowSize*1000000; // new timestamp is given to the minus element
		elementOutput.masterTag = false;
		outputQueue->enqueue(elementOutput);
		//std::cout << "Expired element" << std::endl;
		//std::cout << elementOutput << std::endl;
		windowSynopsis->deleteOldestElement();
	}

}
