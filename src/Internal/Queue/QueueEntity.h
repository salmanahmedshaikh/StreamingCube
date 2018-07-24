//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../../Common/stdafx.h"
#include "../../Internal/Buffer/QueueBuffer.h"
#include "../../Internal/Element/Element.h"
#include "../../Operator/Operator.h"
#include <boost/shared_ptr.hpp>
/*
   this is a queue entity in the query plan
   for each queue, it has an output operator and an input operator
   it has a QueueBuffer to save the elements
 */
class QueueEntity
{
private:
	boost::shared_ptr<QueueBuffer> buffer;
	Timestamp oldestTimestamp;
	boost::shared_ptr<Operator>inputOperator;
	boost::shared_ptr<Operator>outputOperator;
	bool firstDequeueCall = true;

	std::string id;

public:
	static int total_dequeue_number ;

	QueueEntity(void);
	~QueueEntity(void);

	void enqueue(Element& element);
	void dequeue(Element& element);
	void dequeue(void);
	void peek(Element& element);

	bool isEmpty(void);
	bool isFull(void);
	void setId(std::string id);
	std::string getId(void);
	void setInputOperator(boost::shared_ptr<Operator> inputOperator);
	void setOutputOperator(boost::shared_ptr<Operator> outputOperator);
	boost::shared_ptr<Operator> getInputOperator();
	boost::shared_ptr<Operator> getOutputOperator();
};

