//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Operator/RootOperator.h"
#include "../Utility/TimeCounterUtility.h"
#include "../Scheduler/Scheduler.h"
#include "../IO/csvOutput.h"

int RootOperator::rootOutputSize = 0;
RootOperator::RootOperator(void)
{
}

RootOperator::~RootOperator(void)
{
}

void RootOperator::execution()
{
#ifdef DEBUG
	std::cout<<"===================operator begin================="<<std::endl;
	std::cout<<"operatorid : "<<this->getId()<<std::endl;
#endif
	assert(this->getInputQueueList().size()==1);
	boost::shared_ptr<QueueEntity>inputQueue = this->getInputQueueList().front();
	while(1)
	{
		if(inputQueue->isEmpty())
		{
			break;
		}

		if(this->streamOutput->isFull())
		{
			break;
		}

		//std::cout << "Input queue of root is non-empty" << std::endl;

		TimeCounterUtility::getInstance()->start();
		Element inputElement;
		inputQueue->dequeue(inputElement);

		#ifdef TOKYO_PEOPLE_FLOW_DEMO
		// if last 5 digits of an inputElement ID is 99999
		int pid;
		inputElement.document.getField("pid").Val(pid);

		if(pid%100000 == -99999)
		{
            boost::shared_ptr<csvOutput> csvStreamOutput = boost::static_pointer_cast<csvOutput>(this->streamOutput);
            //std::cout<< "pid from root op: " <<pid<<std::endl;

			csvStreamOutput->closeFile();
			csvStreamOutput->openFile((-1 * pid)/100000);
		}
		#endif // TOKYO_PEOPLE_FLOW_DEMO

		this->streamOutput->pushNext(inputElement);
		//std::cout << "Output of QueryID: " << this->queryEntity->getQueryID() << std::endl;

		rootOutputSize += inputElement.getSize();

		TimeCounterUtility::getInstance()->pause();
	}
#ifdef DEBUG
	std::cout<<"===================operator over================="<<std::endl;
#endif
}

void RootOperator::setStreamOutput(boost::shared_ptr<IStreamOutput> streamOutput)
{
	this->streamOutput = streamOutput;
	this->streamOutput->initial();
}

void RootOperator::setQueryEntity( QueryEntity* queryEntity)
{
	this->queryEntity = queryEntity;
}

QueryEntity* RootOperator::getQueryEntity()
{
	return this->queryEntity;
}
