//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../Internal/Element/Element.h"
#include <boost/shared_ptr.hpp>
class QueueEntity;

enum OperatorState// this is the operator state analysis by the scheduler
{
	READY,   //some elements are int the input queue, the operator is not executed
	SUSPENDED,// during the execution, no elements in any input queue
	//ACTIVE   // execution
};
class Operator
{
private:

	OperatorState operatorState;
	std::list<boost::shared_ptr<QueueEntity> >inputQueueList;
	std::list<boost::shared_ptr<QueueEntity> >outputQueueList;

	std::string id;
public:
	Operator(void);
	virtual ~Operator(void);
	virtual void execution()=0;


	void addInputQueue(boost::shared_ptr<QueueEntity> queueEntity);
	void removeInputQueue(boost::shared_ptr<QueueEntity> queueEntity);
	std::list<boost::shared_ptr<QueueEntity> >& getInputQueueList(void);
	void addOutputQueue(boost::shared_ptr<QueueEntity> queueEntity);
	void removeOutputQueue(boost::shared_ptr<QueueEntity> queueEntity);
	std::list<boost::shared_ptr<QueueEntity> >& getOutputQueueList(void);
	OperatorState getOperatorState(void);
	void setOperatorState(OperatorState operatorState);
	void setId(std::string id);
	std::string getId(void);
	void output(Element& element);

	int totalOutputTuples;
};

