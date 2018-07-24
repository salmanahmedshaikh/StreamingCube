//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Operator/SelectionOperator.h"
#include "../Internal/Queue/QueueEntity.h"
#include "../Query/QueryConditionAbstract.h"
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "../Scheduler/Scheduler.h"
#include "../Utility/TimeCounterUtility.h"
SelectionOperator::SelectionOperator(void)
{

}


SelectionOperator::~SelectionOperator(void)
{
}


void SelectionOperator::execution()
{

#ifdef DEBUG
	std::cout<<"===================operator begin================="<<std::endl;
	std::cout<<"operatorid : "<<this->getId()<<std::endl;
#endif
	assert(this->getInputQueueList().size()==1);
	assert(this->getOutputQueueList().size()==1);
	boost::shared_ptr<QueueEntity>inputQueue = this->getInputQueueList().front();
	boost::shared_ptr<QueueEntity>outputQueue = this->getOutputQueueList().front();

	//std::cout<<"===================Selection operator================="<<std::endl;

	while(1)
	{
		Element inputElement;
		if(outputQueue->isFull())
		{
			break;
		}
		if(inputQueue->isEmpty())
		{
			break;
		}
		TimeCounterUtility::getInstance()->start();
		inputQueue->dequeue(inputElement);

		//std::cout << "Element from Selection Op " << std::endl << inputElement << std::endl;
		#ifdef TOKYO_PEOPLE_FLOW_DEMO
        int pid;
        inputElement.document.getField("pid").Val(pid);

        if(pid%100000 == -99999)
        {
            //std::cout << "pid from selection Op " << pid << std::endl;
            outputQueue->enqueue(inputElement);
            continue;
        }
		#endif // TOKYO_PEOPLE_FLOW_DEMO

		if(isSatisfySelectionCondition(inputElement))
		{
			outputQueue->enqueue(inputElement);
			//std::cout << inputElement << std::endl;
		}

		TimeCounterUtility::getInstance()->pause();

	}
#ifdef DEBUG
	std::cout<<"===================operator over================="<<std::endl;
#endif
}

void SelectionOperator::setSelectionCondition(boost::shared_ptr<QueryConditionAbstract> condition)
{
	this->condition = condition;
	//std::cout << "Selection condition is set: " << condition << std::endl;
}

bool SelectionOperator::isSatisfySelectionCondition(Element& element)
{
	return this->condition->getValue(element);

}
