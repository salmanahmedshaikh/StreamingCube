//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Operator/JoinOperator.h"
#include "../Internal/Synopsis/LineageSynopsis.h"
#include "../Internal/Queue/QueueEntity.h"
#include "../Utility/DocumentIdentifierGenerator.h"
#include "../Internal/Synopsis/RelationSynopsis.h"
#include "../Internal/Element/ElementIterator.h"
#include "../Scheduler/Scheduler.h"
#include "../Utility/TimeCounterUtility.h"
#include "../Query/QueryUtility.h"
JoinOperator::JoinOperator(void)
{
	int lineageNumber = 2;
	this->lineageSynopsis.reset(new LineageSynopsis(lineageNumber));
	this->leftRelationSynopsis.reset(new RelationSynopsis());
	this->rightRelationSynopsis.reset(new RelationSynopsis());
//	this->lastLeftTimestamp = 0;
//	this->lastRightTimestamp = 0;
	this->leftElementIterator.reset(new ElementIterator(this->leftRelationSynopsis));
	this->rightElementIterator.reset(new ElementIterator(this->rightRelationSynopsis));
	this->leftOuter = false;
	this->rightOuter = false;

	this->outputTuplesCount = 0;

	/*
	Document a,b;
	if(!(a.getField("243")==b.getField("dsg")))
	{
		//continue;
	}
	*/
}


JoinOperator::~JoinOperator(void)
{
}

void JoinOperator::setLeftJoinAttribute(boost::shared_ptr<QueryAttribute> leftJoinAttribute)
{
	this->leftJoinAttribute = leftJoinAttribute;
}
void JoinOperator::setRightJoinAttribute(boost::shared_ptr<QueryAttribute> rightJoinAttribute)
{
	this->rightJoinAttribute = rightJoinAttribute;
}
void JoinOperator::setLeftOuter(bool bl)
{
	this->leftOuter = bl;
}
void JoinOperator::setRightOuter(bool bl)
{
	this->rightOuter = bl;
}
void JoinOperator::setResultQueryProjection(boost::shared_ptr<QueryProjectionAbstract> resultQueryProjection)
{
	this->resultQueryProjection = resultQueryProjection;
}

void JoinOperator::execution()
{
#ifdef DEBUG
	std::cout<<"===================operator begin================="<<std::endl;
	std::cout<<"operatorid : "<<this->getId()<<std::endl;
#endif
	assert(this->getInputQueueList().size()==2);
	assert(this->getOutputQueueList().size()==1);
	boost::shared_ptr<QueueEntity>leftInputQueue = this->getInputQueueList().front();
	boost::shared_ptr<QueueEntity>rightInputQueue = this->getInputQueueList().back();
	boost::shared_ptr<QueueEntity>outputQueue = this->getOutputQueueList().front();

	Timestamp minimumLeftQueueLastTimestamp = -1;
	Timestamp minimumRightQueueLastTimestamp = -1;
	//minimumLeftQueueLastTimestamp = lastLeftTimestamp;
	//minimumRightQueueLastTimestamp = lastRightTimestamp;
    //std::cout<<"Join operator"<<std::endl;

	while(1)
	{
        //std::cout << "join output count " << outputTuplesCount << std::endl;
		Element leftElement;
		Element rightElement;
		if(outputQueue->isFull())
		{
			break;
		}
		if(!leftInputQueue->isEmpty())
		{
			leftInputQueue->peek(leftElement);
			minimumLeftQueueLastTimestamp = leftElement.timestamp;
		}
		if(!rightInputQueue->isEmpty())
		{
			rightInputQueue->peek(rightElement);
			minimumRightQueueLastTimestamp = rightElement.timestamp;
		}
		if(leftElement.timestamp==-1 && rightElement.timestamp==-1)
		{
			break;
		}

		if((!leftInputQueue->isEmpty())&&(!rightInputQueue->isEmpty())&&minimumLeftQueueLastTimestamp<minimumRightQueueLastTimestamp)
		{
			assert(!leftInputQueue->isEmpty());

			TimeCounterUtility::getInstance()->start();
			leftInputQueue->dequeue();
			//lastLeftTimestamp = leftElement.timestamp;
			if(leftElement.mark == PLUS_MARK)
			{
				processLeftPlus(leftElement);
			}
			else if(leftElement.mark == MINUS_MARK)
			{
				processLeftMinus(leftElement);
			}
			TimeCounterUtility::getInstance()->pause();
		}
		else if ((!leftInputQueue->isEmpty())&&(!rightInputQueue->isEmpty())&&minimumLeftQueueLastTimestamp > minimumRightQueueLastTimestamp)
		{
			assert(!rightInputQueue->isEmpty());

			TimeCounterUtility::getInstance()->start();
			rightInputQueue -> dequeue();
			//lastRightTimestamp = rightElement.timestamp;
			if(rightElement.mark==PLUS_MARK)
			{
				processRightPlus(rightElement);
			}
			else if(rightElement.mark == MINUS_MARK)
			{
				processRightMinus(rightElement);
			}
			TimeCounterUtility::getInstance()->pause();
		}
		else if(!leftInputQueue->isEmpty())
		{
			TimeCounterUtility::getInstance()->start();
			leftInputQueue->dequeue();
			//lastLeftTimestamp = leftElement.timestamp;
			if(leftElement.mark==PLUS_MARK)
			{
				processLeftPlus(leftElement);
			}
			else if(leftElement.mark == MINUS_MARK)
			{
				processLeftMinus(leftElement);
			}
			TimeCounterUtility::getInstance()->pause();
		}
		else if(!rightInputQueue->isEmpty())
		{
			TimeCounterUtility::getInstance()->start();
			rightInputQueue -> dequeue();
			//lastRightTimestamp = rightElement.timestamp;
			if(rightElement.mark==PLUS_MARK)
			{
				processRightPlus(rightElement);
			}
			else if(rightElement.mark == MINUS_MARK)
			{
				processRightMinus(rightElement);
			}
			TimeCounterUtility::getInstance()->pause();
		}
	}
#ifdef DEBUG
	std::cout<<"===================operator over================="<<std::endl;
#endif
}

void JoinOperator::processLeftPlus (Element& leftElement)
{

	boost::shared_ptr<QueueEntity>outputQueue = this->getOutputQueueList().front();

	Document& leftDocument = leftElement.document;

	if(this->leftRelationSynopsis->isFull())
	{
		throw runtime_error("left synopsis is full");
	}

	this->leftRelationSynopsis->insertElement(leftElement);

	this->rightElementIterator->initial();
	Element rightElement;
	while((!outputQueue->isFull())&&this->rightElementIterator->getNext(rightElement))
	{
		Document& rightDocument = rightElement.document;
		boost::any leftValue = this->leftJoinAttribute->getValue(leftDocument);
		boost::any rightValue = this->rightJoinAttribute->getValue(rightDocument);
		//wangyan
		if(!QueryUtility::compareEqual(leftValue,rightValue))
		{
			//salman
			#ifdef TOKYO_PEOPLE_FLOW_DEMO
            int pid;
            leftElement.document.getField("pid").Val(pid);

            if(pid%100000 == -99999)
            {
                if(outputQueue->isFull())
                {
                    throw std::runtime_error("output queue is full");
                }
                outputQueue->enqueue(leftElement);
            }
            #endif // TOKYO_PEOPLE_FLOW_DEMO
            //~salman

			continue;
		}

		boost::any retDocument =  this->resultQueryProjection->performProjection(leftDocument,rightDocument);
		assert(retDocument.type() == typeid( Document));
		Document newDocument = boost::any_cast<Document>(retDocument);
		Element newElement;
		newElement.id = DocumentIdentifierGenerator::generateNewDocumentIdentifier();
		newElement.document = newDocument;
		newElement.mark = PLUS_MARK;
		newElement.timestamp = leftElement.timestamp;
		newElement.masterTag = leftElement.masterTag;
		//insert into lineage synopsis
		if(this->lineageSynopsis->isFull())
		{
			throw runtime_error("lineage synopsis is full");
		}
		Lineage lineage;
		lineage.lineageNumber = 2;
		lineage.lineageDocumentId[0] = leftElement.id;
		lineage.lineageDocumentId[1] = rightElement.id;
		this->lineageSynopsis->insertLineage(lineage,newElement);

		//generate new output to the output queue
		if(outputQueue->isFull())
		{
			throw std::runtime_error("output queue is full");
		}
		outputQueue->enqueue(newElement);

		//std::cout << "joined element 1 " << newElement << std::endl;
		outputTuplesCount++;
	}
	assert(!outputQueue->isFull());
}

void JoinOperator::processLeftMinus (Element& leftElement)
{
	boost::shared_ptr<QueueEntity>outputQueue = this->getOutputQueueList().front();
	Document& leftDocument = leftElement.document;
	this->leftRelationSynopsis->deleteElement(leftElement);
	this->rightElementIterator->initial();
	Element rightElement;

	while((!outputQueue->isFull())&&this->rightElementIterator->getNext(rightElement))
	{
		Document& rightDocument = rightElement.document;
		//std::cout<<"left document : "<<leftDocument<<std::endl;
		//std::cout<<"right document : "<<rightDocument<<std::endl;
		//if satisfied join condition
		boost::any leftValue = this->leftJoinAttribute->getValue(leftDocument);
		boost::any rightValue = this->rightJoinAttribute->getValue(rightDocument);
	//	//wangyan
		if(!QueryUtility::compareEqual(leftValue,rightValue))
		{
			continue;
		}
		Lineage lineage;
		lineage.lineageNumber = 2;
		lineage.lineageDocumentId[0] = leftElement.id;
		lineage.lineageDocumentId[1] = rightElement.id;
		Element elementInSynopsis;
		this->lineageSynopsis->getAndDeleteElement(lineage,elementInSynopsis);

		//generate output element
		Element outputElement;
		outputElement.mark = MINUS_MARK;
		outputElement.id = elementInSynopsis.id;
		outputElement.timestamp = leftElement.timestamp;
		outputElement.document = elementInSynopsis.document;
		outputElement.masterTag = leftElement.masterTag;
		//output element
		if(outputQueue->isFull())
		{
			throw std::runtime_error("output queue is full");
		}
		outputQueue->enqueue(outputElement);

		//std::cout << "joined element 2 " << outputElement << std::endl;
		outputTuplesCount++;
	}
	assert(!outputQueue->isFull());
}
void JoinOperator::processRightPlus (Element& rightElement)
{
	boost::shared_ptr<QueueEntity>outputQueue = this->getOutputQueueList().front();
	Document rightDocument = rightElement.document;
	if(this->rightRelationSynopsis->isFull())
	{
		throw runtime_error("right synopsis is full");
	}
	this->rightRelationSynopsis->insertElement(rightElement);

	this->leftElementIterator->initial();
	Element leftElement;
	while((!outputQueue->isFull())&&this->leftElementIterator->getNext(leftElement))
	{

		Document leftDocument = leftElement.document;

		boost::any leftValue = this->leftJoinAttribute->getValue(leftDocument);
		boost::any rightValue = this->rightJoinAttribute->getValue(rightDocument);
	//	//wangyan
		if(!QueryUtility::compareEqual(leftValue,rightValue))
		{
            //salman
            #ifdef TOKYO_PEOPLE_FLOW_DEMO
            int pid;
            rightElement.document.getField("pid").Val(pid);

            if(pid%100000 == -99999)
            {
                if(outputQueue->isFull())
                {
                    throw std::runtime_error("output queue is full");
                }
                outputQueue->enqueue(rightElement);
            }

            #endif // TOKYO_PEOPLE_FLOW_DEMO
            //~salman
			continue;
		}
		boost::any retDocument =  this->resultQueryProjection->performProjection(leftDocument,rightDocument);
		assert(retDocument.type() == typeid( Document));
		Document newDocument = boost::any_cast<Document>(retDocument);
		//cout << "the rigth plus: " << newDocument << endl;
		Element newElement;
		newElement.id = DocumentIdentifierGenerator::generateNewDocumentIdentifier();
		newElement.document = newDocument;
		newElement.mark = PLUS_MARK;
		newElement.timestamp = rightElement.timestamp;
		newElement.masterTag = rightElement.masterTag;
		//insert into lineage synopsis
		if(this->lineageSynopsis->isFull())
		{
			throw runtime_error("lineage synopsis is full");
		}
		Lineage lineage;
		lineage.lineageNumber = 2;
		lineage.lineageDocumentId[0] = leftElement.id;
		lineage.lineageDocumentId[1] = rightElement.id;
		this->lineageSynopsis->insertLineage(lineage,newElement);

		//generate new output to the output queue
		if(outputQueue->isFull())
		{
			throw std::runtime_error("output queue is full");
		}
		outputQueue->enqueue(newElement);

		//std::cout << "joined element 3 " << newElement << std::endl;
		outputTuplesCount++;
	}
	assert(!outputQueue->isFull());
}
void JoinOperator::processRightMinus (Element& rightElement)
{
	boost::shared_ptr<QueueEntity>outputQueue = this->getOutputQueueList().front();
	Document& rightDocument = rightElement.document;
	this->rightRelationSynopsis->deleteElement(rightElement);
	this->leftElementIterator->initial();
	Element leftElement;
	while((!outputQueue->isFull())&&this->leftElementIterator->getNext(leftElement))
	{

		Document& leftDocument = leftElement.document;
		//if satisfied join condition
		//std::cout<<"left document : "<<leftDocument<<std::endl;
		//std::cout<<"right document : "<<rightDocument<<std::endl;
		boost::any leftValue = this->leftJoinAttribute->getValue(leftDocument);
		boost::any rightValue = this->rightJoinAttribute->getValue(rightDocument);
	//	//wangyan
		if(!QueryUtility::compareEqual(leftValue,rightValue))
		{
			continue;
		}
		Lineage lineage;
		lineage.lineageNumber = 2;
		lineage.lineageDocumentId[0] = leftElement.id;
		lineage.lineageDocumentId[1] = rightElement.id;
		Element elementInSynopsis;
		this->lineageSynopsis->getAndDeleteElement(lineage,elementInSynopsis);
		//cout << "the rigth minus: " << elementInSynopsis.document << endl;
		//generate output element
		Element outputElement;
		outputElement.mark = MINUS_MARK;
		outputElement.id = elementInSynopsis.id;
		outputElement.timestamp = rightElement.timestamp;
		outputElement.document = elementInSynopsis.document;
		outputElement.masterTag = rightElement.masterTag;
		//output element
		if(outputQueue->isFull())
		{
			throw std::runtime_error("output queue is full");
		}
		outputQueue->enqueue(outputElement);
		outputTuplesCount++;

		//std::cout << "joined element 4 " << outputElement << std::endl;
	}
	assert(!outputQueue->isFull());
}
