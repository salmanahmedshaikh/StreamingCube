//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../IO/IStreamInput.h"
#include "../Operator/Operator.h"
#include "../Internal/Queue/QueueEntity.h"
#include "../Internal/Element/Element.h"
#include "../Common/Types.h"
#include <boost/shared_ptr.hpp>
#include "../Scheduler/Scheduler.h"
#include "../IO/DispatcherStreamInput.h"
#include <boost/tuple/tuple.hpp>


//multiple queries may share one leaf operator
//when a tuple comes to the leaf operator,
//scan all querie related.
//For each query,if this stream is a master stream, set the input tuple a master mark, and set the query activated
//	  if this stream is not a master stream, if the query is activated, set the master mark true
//						 if the query is not activated,set the master mark false

class QueryEntity;
class LeafOperator : public Operator
{
private:
	boost::shared_ptr<IStreamInput> streamInput;
	std::list<QueryEntity* >queryList;
	bool isMasterStream;

public:
	static int totalInputNumber ;
	LeafOperator(void);
	~LeafOperator(void);
	bool isMaster(void);
	void setStreamInput(boost::shared_ptr<IStreamInput> streamInput);
	void execution();
	boost::shared_ptr<IStreamInput> getStreamInput(void);
	friend class Scheduler;
	void addQuery( QueryEntity* queryEntity);
	std::list<QueryEntity* > getRelatedQueries();
};


