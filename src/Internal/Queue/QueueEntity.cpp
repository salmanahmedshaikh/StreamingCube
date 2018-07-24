//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../../Common/stdafx.h"
#include "../../Internal/Queue/QueueEntity.h"
#include "../../Scheduler/Scheduler.h"
int QueueEntity::total_dequeue_number = 0 ;

QueueEntity::QueueEntity()
{
	this->buffer.reset(new QueueBuffer());
}


QueueEntity::~QueueEntity(void)
{
}
bool QueueEntity::isEmpty(void)
{
	return this->buffer->isEmpty();
}
void QueueEntity::enqueue(Element& element)
{

#ifdef DEBUG
	std::cout<<"~~~~~~~~~~~~~~~queue begin~~~~~~~~~~"<<std::endl;
	std::cout<<"queueId : "<<this->getId()<<std::endl;
	std::cout<<"enqueue : "<<element<<std::endl;
#endif

	this->buffer->enqueue(element);

#ifdef DEBUG
	std::cout<<"~~~~~~~~~~~~~~~queue over~~~~~~~~~~"<<std::endl;
#endif

}
void QueueEntity::dequeue(Element& element)
{
    if(QueueEntity::firstDequeueCall)
    {
        QueueEntity::total_dequeue_number = 0;
        QueueEntity::firstDequeueCall = false;
    }

#ifdef DEBUG
	std::cout<<"---------------queue begin----------"<<std::endl;
	std::cout<<"queueId : "<<this->getId()<<std::endl;
#endif
	total_dequeue_number++;
	this->buffer->dequeue(element);
#ifdef DEBUG
	std::cout<<"dequeue : "<<element<<std::endl;
	std::cout<<"---------------queue over-----------"<<std::endl;
#endif
}
void QueueEntity::dequeue()
{
     if(QueueEntity::firstDequeueCall)
    {
        QueueEntity::total_dequeue_number = 0;
        QueueEntity::firstDequeueCall = false;
    }

#ifdef DEBUG
	std::cout<<"---------------queue begin----------"<<std::endl;
	std::cout<<"queueId : "<<this->getId()<<std::endl;
#endif
	total_dequeue_number++;
	this->buffer->dequeue();
#ifdef DEBUG
	std::cout<<"dequeue"<<std::endl;
	std::cout<<"---------------queue over-----------"<<std::endl;
#endif
}
void QueueEntity::peek(Element& element)
{
#ifdef DEBUG
	std::cout<<"---------------queue begin----------"<<std::endl;
	std::cout<<"queueId : "<<this->getId()<<std::endl;
#endif
	this->buffer->peek(element);
#ifdef DEBUG
	std::cout<<"pick : "<<element<<std::endl;
	std::cout<<"---------------queue over-----------"<<std::endl;
#endif
}



bool QueueEntity::isFull(void)
{
#ifdef DEBUG
	std::cout<<"---------------queue begin----------"<<std::endl;
	std::cout<<"queueId : "<<this->getId()<<std::endl;
#endif
	bool bl =  this->buffer->isFull();
#ifdef DEBUG
	if(bl== true)
	{
		std::cout<<"the queue is full"<<std::endl;
	}
	else
	{
		std::cout<<"the queue is not full"<<std::endl;
	}
	std::cout<<"---------------queue over-----------"<<std::endl;
#endif
	return bl;
}

void QueueEntity::setId(std::string id)
{
	this->id = id;
}
std::string QueueEntity::getId(void)
{
	return this->id;
}

void QueueEntity::setInputOperator(boost::shared_ptr<Operator> inputOperator)
{
	this->inputOperator = inputOperator;
}
void QueueEntity::setOutputOperator(boost::shared_ptr<Operator> outputOperator)
{
	this->outputOperator = outputOperator;
}
boost::shared_ptr<Operator> QueueEntity::getInputOperator()
{
	return this->inputOperator;
}
boost::shared_ptr<Operator> QueueEntity::getOutputOperator()
{
	return this->outputOperator;
}
