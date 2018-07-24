//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Operator/SmartRangeWindowOperator.h"
#include "../Internal/Synopsis/WindowSynopsis.h"
#include "../Internal/Queue/QueueEntity.h"
#include "../Utility/TimeCounterUtility.h"
#include "../Scheduler/Scheduler.h"

int SmartRangeWindowOperator::directlyDeletedTuple = 0;
SmartRangeWindowOperator::SmartRangeWindowOperator(Timeunit timeRange):rangeWindowSize(timeRange)
{
    // range window variables
    this->windowSynopsis.reset(new WindowSynopsis());
	this->lastInputTimestamp = -1;
	this->oldestElementTimestampInWinSynopsis = TimestampGenerator::getCurrentTime();
	this->oldestBufferedElementTimestampInWinSynopsis = -1;
	this->latestBufferedElementTimestampInWinSynopsis = -1;

    // variables specific to smart range window
	// this->currentRowNumberInSynopsis = 0;
	this->lastOutputBufferedTupleTimestamp = -1;
	this->bufferedTupleNumber = 0;
	this->isFirstBufferedElement = true;
}

SmartRangeWindowOperator::~SmartRangeWindowOperator(void)
{
}

//int SmartRangeWindowOperator::getWindowSize()
//{
//	return this->rowWindowSize;
//}

std::list<QueryEntity*> SmartRangeWindowOperator::getRelatedQueries()
{
	return this->queryEntityList;
}

void SmartRangeWindowOperator::execution()
{
#ifdef DEBUG
	std::cout<<"===================operator begin================="<<std::endl;
	std::cout<<"operatorid : "<<this->getId()<<std::endl;
#endif
	bool hasMasterStreamTupleArrived = false;


	std::list<QueryEntity*>::iterator it;
	//std::cout<<this->queryEntityList.size()<<std::endl;
	for( it = this->queryEntityList.begin(); it != this->queryEntityList.end(); it++)
	{
		QueryEntity * queryEntity = *it;
		// If the activationTimestamp of any of the query associated with this operator is greater than the lastOutputBufferedTupleTimestamp; it means the
		// master tuple arrived and the buffered tuples must be sent out; as it is done below
		if(this->lastOutputBufferedTupleTimestamp < queryEntity->getLastActivatedTimestamp())
		{
			this->lastOutputBufferedTupleTimestamp = queryEntity->getLastActivatedTimestamp();
			hasMasterStreamTupleArrived = true;
		}
	}

    // Output all the bufferd tuples in the synopsis when the query becomes active due the arrival of master tuple
	if(hasMasterStreamTupleArrived == true)
	{
		outputBufferedElement();
		this->isFirstBufferedElement = true;
	}

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
		//TimeCounterUtility::getInstance()->start();
		inputQueue->dequeue(inputElement);

		assert(inputElement.mark == PLUS_MARK);
		assert(lastInputTimestamp <= inputElement.timestamp);
		lastInputTimestamp = inputElement.timestamp;
        // Master tag of the elements arriving during the query active duration are set to true.
        // Elemets with the master tag set contribute to query result output.
		bool masterTag = inputElement.masterTag;
		//std::cout<<masterTag<<std::endl;
		if(masterTag == true)
		{
			this->executionInEagerMode(inputElement);
		}
		else
		{
			this->executionInWaitingMode(inputElement);
		}
		//TimeCounterUtility::getInstance()->pause();
	}


#ifdef DEBUG
	std::cout<<"===================operator over================="<<std::endl;
#endif
}

void SmartRangeWindowOperator::executionInWaitingMode(Element& inputElement)
{
	if(this->windowSynopsis->isFull())   // the window synopsis is not full
	{
		throw std::runtime_error("synopsis full");
	}

    // No p-tuple is generated for the tuples inserted in executionInWaitingMode
	this->windowSynopsis->insertElement(inputElement);
	//this->currentRowNumberInSynopsis++;
	this->bufferedTupleNumber++;
	this->latestBufferedElementTimestampInWinSynopsis = inputElement.timestamp;

	if(this->isFirstBufferedElement)
	{
        this->oldestBufferedElementTimestampInWinSynopsis = inputElement.timestamp;
        this->isFirstBufferedElement = false;
    }

	//outputQueue->enqueue(inputElement);//forward the same element
	//std::cout<<currentRowNumberInSynopsis<<std::endl;
	Element outputElement;

	if((TimestampGenerator::getCurrentTime() - this->oldestElementTimestampInWinSynopsis)/1000000.000000 > this->rangeWindowSize)
	{
        while(true)
        {
            if (this->windowSynopsis -> isEmpty())
            {
                break;
            }

            // Check if buffered element is ready to expire or non-buffered element
            if((TimestampGenerator::getCurrentTime() - this->oldestBufferedElementTimestampInWinSynopsis)/1000000.000000 > this->rangeWindowSize)
            {
                //we haven't output it (no p-tuple was generated for it), delete it directly
                //this tuple is a buffered tuple, minus the buffered tuple number
                Element oldestElement;

                this->windowSynopsis->deleteOldestElement();
                this->bufferedTupleNumber--;
                directlyDeletedTuple++;
                std::cout << "directlyDeletedTuple " << directlyDeletedTuple << std::endl;

                this->windowSynopsis->getOldestElement(oldestElement);

                if(this->latestBufferedElementTimestampInWinSynopsis >= oldestElement.timestamp)
                {
                    this->oldestBufferedElementTimestampInWinSynopsis = oldestElement.timestamp;
                }
                else
                {
                    std::cout << "SmartRangeWindowOperator::executionInWaitingMode assert" << std::endl;
                    assert(false);

                }
            }
            else
            {
                this->windowSynopsis->getOldestElement(outputElement);
                this->oldestElementTimestampInWinSynopsis = outputElement.timestamp;

                if((TimestampGenerator::getCurrentTime() - this->oldestElementTimestampInWinSynopsis)/1000000.000000 > this->rangeWindowSize)
                {
                    // We have already sent a p-tuple to the downstream operators. Hence we must generate a m-tuple to delete it
                    // from the downstream operators
                    this->windowSynopsis->deleteOldestElement();
                    outputElement.mark = MINUS_MARK;
                    outputElement.timestamp = inputElement.timestamp;
                    outputElement.masterTag = inputElement.masterTag;
                    //if output queue is full, change the state to blocked and buffer the element;
                    output(outputElement);
                }
                else
                {
                    //exit the while loop, there is no element to delete
                    break;
                }
            }
        }
	}


    /*
	if(this->currentRowNumberInSynopsis > this->rowWindowSize)
	{  // generate oldest element out
	//	std::cout<<"tuple deleted directly"<<std::endl;
		this->windowSynopsis->getOldestElement(outputElement);
		this->windowSynopsis->deleteOldestElement();
		this->currentRowNumberInSynopsis--;

		if(this->bufferedTupleNumber  > rowWindowSize)
		{//we haven't output it, delete it directly
			//this tuple is a buffered tuple, minus the buffered tuple number
			this->bufferedTupleNumber--;
			directlyDeletedTuple++;
		}
		else
		{// has already been output
			outputElement.mark = MINUS_MARK;
			outputElement.timestamp = inputElement.timestamp;
			outputElement.masterTag = inputElement.masterTag;
			//if output queue is full, change the state to blocked and buffer the element;
			output(outputElement);
		}
	}
	*/

}
void SmartRangeWindowOperator::executionInEagerMode(Element& inputElement)
{
	if(this->windowSynopsis->isFull())   // the window synopsis is not full
	{
		throw std::runtime_error("synopsis full");
	}

	this->windowSynopsis->insertElement(inputElement);
	//this->currentRowNumberInSynopsis++;

	if((TimestampGenerator::getCurrentTime() - this->oldestElementTimestampInWinSynopsis)/1000000.000000 > this->rangeWindowSize)
    {
        //oldestElementTimestampInWinSynopsis = inputElement.timestamp;
        expireElement(inputElement);
	}

	output(inputElement);//forward the same plus(p) element

	/*
	Element outputElement;
	if(this->currentRowNumberInSynopsis>this->rowWindowSize)
	{  // generate oldest element out
		this->windowSynopsis->getOldestElement(outputElement);
		this->windowSynopsis->deleteOldestElement();

		outputElement.mark = MINUS_MARK;
		outputElement.timestamp = inputElement.timestamp;
		outputElement.masterTag = inputElement.masterTag;
		//if output queue is full, change the state to blocked and buffer the element;
		output(outputElement);
	}
	*/
}

void SmartRangeWindowOperator::expireElement(Element& inputElement)//expire elements with a timestamp older than ts
{
	//boost::shared_ptr<QueueEntity>outputQueue = this->getOutputQueueList().front();
	Element elementInSynopsis;
	Element elementOutput;

	while(true)
	{
		if (this->windowSynopsis -> isEmpty())
		{
			return;
		}

        //fetch the oldest element to see if gets expired
		this->windowSynopsis->getOldestElement(elementInSynopsis);
		this->oldestElementTimestampInWinSynopsis = elementInSynopsis.timestamp;

        // If oldest element is not yet expired, return from the function
        if((TimestampGenerator::getCurrentTime() - elementInSynopsis.timestamp)/1000000.000000 < this->rangeWindowSize)
        {
            return;
        }

        // To expire an element, send its equivalent MINUS tuple to the downstream operators
		//assert(!outputQueue->isFull());
		elementOutput.document = elementInSynopsis.document;
		elementOutput.id = elementInSynopsis.id;
		elementOutput.mark = MINUS_MARK;
		elementOutput.timestamp = inputElement.timestamp; // new timestamp is given to the minus element
		elementOutput.masterTag = inputElement.masterTag;
		output(elementOutput);
		//outputQueue->enqueue(elementOutput);
		//std::cout << "Expired element" << std::endl;
		//std::cout << elementOutput << std::endl;
		windowSynopsis->deleteOldestElement();
		//this->currentRowNumberInSynopsis--;
	}

}

void SmartRangeWindowOperator::outputBufferedElement()
{
	//scan the window synopsis from the oldest, and find the one has a same
	std::list<Element> elementList;

	// if there is no buffered tuples, return
	if(this->bufferedTupleNumber==0)
	{
		return;
	}

	//TimeCounterUtility::getInstance()->start();
	// These elements arrive during the query inactive duration and therefore were buffered
	this->windowSynopsis->peekNewestElementList(elementList,this->bufferedTupleNumber);
	for(std::list<Element>::iterator it = elementList.begin();it!=elementList.end();it++)
	{
		Element& outputElement = *it;
		outputElement.mark = PLUS_MARK;
		//if output queue is full, change the state to blocked and buffer the element;
		//std::cout<<outputElement<<std::endl;
		outputElement.document.getOwned();
		output(outputElement);
	}

	this->bufferedTupleNumber = 0;
	//TimeCounterUtility::getInstance()->pause();
}

void SmartRangeWindowOperator::addQuery(QueryEntity* queryEntity)
{
	this->queryEntityList.push_back(queryEntity);
}
