//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Operator/SmartRowWindowOperator.h"
#include "../Internal/Synopsis/WindowSynopsis.h"
#include "../Internal/Queue/QueueEntity.h"
#include "../Utility/TimeCounterUtility.h"
#include "../Scheduler/Scheduler.h"
int SmartRowWindowOperator::directlyDeletedTuple = 0;
SmartRowWindowOperator::SmartRowWindowOperator(int rowWindowSize):rowWindowSize(rowWindowSize)
{
	this->windowSynopsis.reset(new WindowSynopsis());
	this->currentRowNumberInSynopsis = 0;
	this->lastInputTimestamp = -1;
	this->lastOutputBufferedTupleTimestamp = -1;
	this->bufferedTupleNumber = 0;
}


SmartRowWindowOperator::~SmartRowWindowOperator(void)
{
}

int SmartRowWindowOperator::getWindowSize()
{
	return this->rowWindowSize;
}

std::list<QueryEntity*> SmartRowWindowOperator::getRelatedQueries()
{
	return this->queryEntityList;
}

void SmartRowWindowOperator::execution()
{
#ifdef DEBUG
	std::cout<<"===================operator begin================="<<std::endl;
	std::cout<<"operatorid : "<<this->getId()<<std::endl;
#endif
	int hasMasterStreamTupleArrived = false;
	std::list<QueryEntity*>::iterator it;
	//std::cout<<this->queryEntityList.size()<<std::endl;
	for( it = this->queryEntityList.begin(); it != this->queryEntityList.end(); it++)
	{
		QueryEntity * queryEntity = *it;
		if(this->lastOutputBufferedTupleTimestamp < queryEntity->getLastActivatedTimestamp())
		{
			this->lastOutputBufferedTupleTimestamp = queryEntity->getLastActivatedTimestamp();
			hasMasterStreamTupleArrived = true;
		}
	}
	if(hasMasterStreamTupleArrived == true)
	{
		outputBufferedElement();
	}
	assert(this->getInputQueueList().size()==1);
	assert(this->getOutputQueueList().size()==1);
	boost::shared_ptr<QueueEntity>inputQueue = this->getInputQueueList().front();


	while(1)
	{
		Element inputElement;

		if(inputQueue->isEmpty())   // the input queue is not empty
		{
			break;
		}
		TimeCounterUtility::getInstance()->start();
		inputQueue->dequeue(inputElement);
		//std::cout<<inputElement<<std::endl;
		assert(inputElement.mark==PLUS_MARK);
		assert(lastInputTimestamp<= inputElement.timestamp);
		lastInputTimestamp = inputElement.timestamp;
		//Master tag of the elements arriving during the query active duration are set to true.
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
		TimeCounterUtility::getInstance()->pause();
	}
#ifdef DEBUG
	std::cout<<"===================operator over================="<<std::endl;
#endif
}
void SmartRowWindowOperator::executionInWaitingMode(Element& inputElement)
{
	if(this->windowSynopsis->isFull())   // the window synopsis is not full
	{
		throw std::runtime_error("synopsis full");
	}

	//std::cout << "executing in waiting mode: directlyDeletedTuple "  << directlyDeletedTuple << std::endl;

	this->windowSynopsis->insertElement(inputElement);
	this->currentRowNumberInSynopsis++;
	this->bufferedTupleNumber++;
	//outputQueue->enqueue(inputElement);//forward the same element
	//std::cout<<currentRowNumberInSynopsis<<std::endl;
	Element outputElement;

	if(this->currentRowNumberInSynopsis>this->rowWindowSize)
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
}
void SmartRowWindowOperator::executionInEagerMode(Element& inputElement)
{

	if(this->windowSynopsis->isFull())   // the window synopsis is not full
	{
		throw std::runtime_error("synopsis full");
	}

	this->windowSynopsis->insertElement(inputElement);
	this->currentRowNumberInSynopsis++;

	output(inputElement);//forward the same element

	Element outputElement;
	if(this->currentRowNumberInSynopsis>this->rowWindowSize)
	{  // generate oldest element out
		this->windowSynopsis->getOldestElement(outputElement);
		this->windowSynopsis->deleteOldestElement();
		this->currentRowNumberInSynopsis--;
		outputElement.mark = MINUS_MARK;
		outputElement.timestamp = inputElement.timestamp;
		outputElement.masterTag = inputElement.masterTag;
		//if output queue is full, change the state to blocked and buffer the element;
		output(outputElement);


	}
}
void SmartRowWindowOperator::outputBufferedElement()
{
	//scan the window synopsis from the oldest, and find the one has a same
	std::list<Element> elementList;
	if(this->bufferedTupleNumber==0)
	{
		return;
	}
	TimeCounterUtility::getInstance()->start();
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
	TimeCounterUtility::getInstance()->pause();
}
void SmartRowWindowOperator::addQuery(QueryEntity* queryEntity)
{
	this->queryEntityList.push_back(queryEntity);
}
