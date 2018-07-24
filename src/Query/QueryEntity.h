//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <boost/shared_ptr.hpp>
#include "../Query/QueryIntermediateRepresentation.h"
#include "../IO/IStreamOutput.h"
#include "../Operator/LeafOperator.h"
#include "../Operator/Operator.h"
#include "../Common/Types.h"
#include <boost/tuple/tuple.hpp>
#include "../Internal/Queue/QueueEntity.h"
class RootOperator;
class QueryEntity
{
private:

	boost::shared_ptr<IStreamOutput> streamOutput;
//	std::list<boost::shared_ptr<Operator> >operatorList;
//	bool active;
	TimeDuration activateDuration;
	TimeDuration rangeWindowSize;
	Timestamp lastActivatedTimestamp;
	std::map<LeafOperator*, bool> masterTagMap;
	std::map<LeafOperator*, boost::shared_ptr<QueueEntity> > outputQueueMap;
	boost::shared_ptr<RootOperator> rootOperator;

	//added by salman
	bool isDispatcher;
	bool isExecutor;
	int queryID;

public:
	QueryEntity(boost::shared_ptr<QueryIntermediateRepresentation>queryIntermediateRepresentation,boost::shared_ptr<IStreamOutput> streamOutput, std::string durationSpecification);
	QueryEntity(boost::shared_ptr<QueryIntermediateRepresentation> queryIntermediateRepresentation,boost::shared_ptr<IStreamOutput> streamOutput,std::string durationSpecification, bool isDispatcher);
	~QueryEntity();

	bool getIsDispatcher();
	void setIsDispatcher(bool is_Dispatcher);
	bool getIsExecutor();
	void setIsExecutor(bool is_Executor);
	void setActive(Timestamp ts);
	bool isActive(Timestamp ts);
	void addMasterTag(LeafOperator* leafOperator, bool masterTag);
	bool getMasterTag(LeafOperator* leafOperator);
	void setRangeWindowSize(TimeDuration rangeWindowSize);
	TimeDuration getRangeWindowSize();

	//salman
	int getQueryID(void);

	void addOutputQueue(LeafOperator* leafOperator, boost::shared_ptr<QueueEntity> outputQueue);
	boost::shared_ptr<QueueEntity> getOutputQueue (LeafOperator* leafOperator) ;
	friend class QueryIntermediateRepresentationInterpreter;
	void changeLeafOperator(boost::shared_ptr<LeafOperator>fromOperator, boost::shared_ptr<LeafOperator>toOperator);
	Timestamp getLastActivatedTimestamp();
	boost::shared_ptr<RootOperator> getRootOperator();
};
