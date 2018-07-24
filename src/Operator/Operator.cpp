//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Operator/Operator.h"
#include "../Internal/Queue/QueueEntity.h"
#include <boost/shared_ptr.hpp>

Operator::Operator(void)
{

	this->operatorState = SUSPENDED;
	this->totalOutputTuples = 0;
}

Operator::~Operator(void)
{

}

void Operator::setId(std::string id)
{
	this->id = id;
}
std::string Operator::getId(void)
{
	return this->id;
}

void Operator::addInputQueue(boost::shared_ptr<QueueEntity> queueEntity)
{
	this->inputQueueList.push_back(queueEntity);
}
void Operator::removeInputQueue(boost::shared_ptr<QueueEntity> queueEntity)
{
	this->inputQueueList.remove(queueEntity);
}
std::list<boost::shared_ptr<QueueEntity> >& Operator::getInputQueueList(void)
{
	return this->inputQueueList;
}
void Operator::addOutputQueue(boost::shared_ptr<QueueEntity> queueEntity)
{
	this->outputQueueList.push_back(queueEntity);
}
void Operator::removeOutputQueue(boost::shared_ptr<QueueEntity> queueEntity)
{
	this->outputQueueList.remove(queueEntity);

}
std::list<boost::shared_ptr<QueueEntity> >& Operator::getOutputQueueList(void)
{
	return this->outputQueueList;
}

OperatorState Operator::getOperatorState(void)
{
	return this->operatorState;
}
void Operator::setOperatorState(OperatorState operatorState)
{
	this->operatorState = operatorState;
}
void Operator::output(Element& element)
{
	std::list<boost::shared_ptr<QueueEntity> >::iterator it;
	for( it = this->outputQueueList.begin(); it != this->outputQueueList.end(); it ++ )
	{
		boost::shared_ptr<QueueEntity> queueEntity = *it;
		assert(!queueEntity->isFull());
		queueEntity->enqueue(element);
		totalOutputTuples++;
	}
}
