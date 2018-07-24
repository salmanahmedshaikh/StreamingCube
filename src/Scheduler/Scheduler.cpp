//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Scheduler/Scheduler.h"
#include "../Operator/LeafOperator.h"
#include "../Operator/RootOperator.h"
#include "../Operator/RelationLeafOperator.h"
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/any.hpp>
Scheduler* Scheduler::scheduler = NULL;
Scheduler::Scheduler()
{
}

Scheduler::~Scheduler(void)
{
}

Scheduler* Scheduler::getInstance()
{
	if (scheduler == NULL) {
		scheduler = new Scheduler();
	}
	return scheduler;
}

void Scheduler::getNextOperatorToExecute(boost::shared_ptr<Operator> & op) {

	boost::mutex::scoped_lock lk(registerOperatorListMutex);
	if (it == this->operatorList.end())
	{
		it = this->operatorList.begin();
	}
	op = *it;

	//skip the RelationLeafOperator
	if (typeid(*op).name() == typeid(RelationLeafOperator).name()) {
		operatorList.erase(it++);
	} else {
		it++;
	}
	return;
}

void Scheduler::emptyOperatorList(void)
{
    boost::mutex::scoped_lock lk(registerOperatorListMutex);
    std::list<boost::shared_ptr<Operator> >().swap(operatorList);
}

bool Scheduler::isEmpty(void)
{
	boost::mutex::scoped_lock lk(registerOperatorListMutex);
	return this->operatorList.size() == 0;
}

void Scheduler::setOperatorList(
		std::list<boost::shared_ptr<Operator> > operatorList) {
	//std::cout << "setting op list in scheduler " << std::endl;
	boost::mutex::scoped_lock lk(registerOperatorListMutex);

	this->operatorList = operatorList;
	this->it = this->operatorList.begin();

}

