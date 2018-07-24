//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../Operator/Operator.h"
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
/* decide which operator to execute, and how many times to execute */
class Scheduler:private boost::noncopyable
{
private:
	static Scheduler * scheduler;
	Scheduler();
	std::list<boost::shared_ptr<Operator> >::iterator it;
	std::list<boost::shared_ptr<Operator> >operatorList;
	boost::mutex registerOperatorListMutex;
public:
	~Scheduler(void);
	static Scheduler* getInstance();
	void getNextOperatorToExecute(boost::shared_ptr<Operator> & op);
	void emptyOperatorList(void);
	bool isEmpty(void); // no operator exist
	void setOperatorList(std::list<boost::shared_ptr<Operator> >operatorList);
};

