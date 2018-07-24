//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "QueryEntity.h"
#include "../Query/QueryIntermediateRepresentationInterpreter.h"
#include "../IO/IStreamOutput.h"
#include "QueryUtility.h"
#include "../Common/Types.h"
#include "../Operator/LeafOperator.h"
#include "../Utility/QueryIdentityGenerator.h"

QueryEntity::~QueryEntity()
{
}

QueryEntity::QueryEntity(boost::shared_ptr<QueryIntermediateRepresentation> queryIntermediateRepresentation,boost::shared_ptr<IStreamOutput> streamOutput,std::string durationSpecification)
{
	this->lastActivatedTimestamp = -1;
	this->activateDuration = QueryUtility::getDuration(durationSpecification);
	this->streamOutput = streamOutput;
	Document& queryDocument = queryIntermediateRepresentation->getDocument();
	//salman
	this->isDispatcher = false;
	this->queryID = QueryIdentityGenerator::generateNewQueryIdentity();
	this->streamOutput->setStreamOutputQueryID(this->queryID);
	// Resolve the query operators into sequence of operators to execute
	QueryIntermediateRepresentationInterpreter::getInstance()->resolve(queryDocument,this);
}

QueryEntity::QueryEntity(boost::shared_ptr<QueryIntermediateRepresentation> queryIntermediateRepresentation,boost::shared_ptr<IStreamOutput> streamOutput,std::string durationSpecification, bool isDispatcher)
{
	this->lastActivatedTimestamp = -1;
	this->activateDuration = QueryUtility::getDuration(durationSpecification);
	this->streamOutput = streamOutput;
	Document& queryDocument = queryIntermediateRepresentation->getDocument();
	//salman
	this->isExecutor = !isDispatcher;
	this->isDispatcher = isDispatcher;
	this->queryID = QueryIdentityGenerator::generateNewQueryIdentity();
	this->streamOutput->setStreamOutputQueryID(this->queryID);
	// Resolve the query operators into sequence of operators to execute
	QueryIntermediateRepresentationInterpreter::getInstance()->resolve(queryDocument,this);
}

int QueryEntity::getQueryID(void)
{
    return this->queryID;
}

bool QueryEntity::getIsDispatcher()
{
    return this->isDispatcher;
}

void QueryEntity::setIsDispatcher(bool is_Dispatcher)
{
    this->isDispatcher = is_Dispatcher;
}

void QueryEntity::setIsExecutor(bool is_Executor)
{
    this->isExecutor = is_Executor;
}

bool QueryEntity::getIsExecutor()
{
    return this->isExecutor;
}

void QueryEntity::setRangeWindowSize(TimeDuration rangeWindowSize) // rangeWindowSize unit is seconds
{
    // setting the unit of microsecond (converting from seconds to microseconds)
    this->rangeWindowSize = rangeWindowSize * 1000 * 1000;
}

TimeDuration QueryEntity::getRangeWindowSize()
{
    return this->rangeWindowSize;
}

// Arrival of any tuple from master stream activates this query
void QueryEntity::setActive(Timestamp ts)
{
	assert(ts >= lastActivatedTimestamp);
	this->lastActivatedTimestamp = ts;
}

bool QueryEntity::isActive(Timestamp ts)
{
	//std::cout<<ts-lastActivatedTimestamp<<std::endl;
	//std::cout<<"activateDuration : "<<this->activateDuration<<std::endl;
	//std::cout<<"this->rangeWindowSize : "<<this->rangeWindowSize<<std::endl;

	if(ts-lastActivatedTimestamp <= this->activateDuration || ts-lastActivatedTimestamp <= this->rangeWindowSize)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void QueryEntity::addMasterTag(LeafOperator* leafOperator, bool masterTag)
{
	this->masterTagMap.insert(make_pair(leafOperator, masterTag));
	//std::cout << "leafOperator " << leafOperator << " inserted" << std::endl;
	//std::cout << "masterTagMap.size() " << masterTagMap.size() << std::endl;
}
bool QueryEntity::getMasterTag(LeafOperator *leafOperator)
{
	std::map<LeafOperator*, bool>::iterator it;
	it = this->masterTagMap.find(leafOperator);
	assert(it!=masterTagMap.end());
	bool result = it->second;
	return it->second;

}


void QueryEntity::addOutputQueue(LeafOperator* leafOperator, boost::shared_ptr<QueueEntity> outputQueue)
{
	this->outputQueueMap.insert(make_pair(leafOperator, outputQueue));
}
boost::shared_ptr<QueueEntity> QueryEntity::getOutputQueue (LeafOperator* leafOperator)
{

	std::map<LeafOperator*, boost::shared_ptr<QueueEntity> >::iterator it;
	it = this->outputQueueMap.find(leafOperator);
	assert(it!=outputQueueMap.end());
	return it->second;
}
boost::shared_ptr<RootOperator> QueryEntity::getRootOperator()
{
	return this->rootOperator;
}
void QueryEntity::changeLeafOperator(boost::shared_ptr<LeafOperator>fromOperator, boost::shared_ptr<LeafOperator>toOperator)
{
	LeafOperator * fromOp = &*fromOperator;
	LeafOperator * toOp = &*toOperator;

	//std::cout << "fromOp " << fromOp << ": of queryEntity: " << this << std::endl;
	//std::cout << "toOp " << toOp << std::endl;

	std::map<LeafOperator*, bool>::iterator it;
	it = masterTagMap.find(fromOp);
	assert(it != masterTagMap.end());
	bool masterTag = it->second;

	//std::cout << "masterTag " << masterTag << std::endl;
	//std::cout << "masterTagMap.size() " << masterTagMap.size() << std::endl;

	masterTagMap.erase(it);
	masterTagMap.insert(make_pair(toOp, masterTag));

	std::map<LeafOperator*, boost::shared_ptr<QueueEntity> >::iterator itt;
	itt = outputQueueMap.find(fromOp);
	assert(itt != outputQueueMap.end());
	boost::shared_ptr<QueueEntity> outputQueue = itt->second;
	outputQueueMap.erase(itt);
	outputQueueMap.insert(make_pair(toOp, outputQueue));
}

Timestamp QueryEntity::getLastActivatedTimestamp()
{
	return this->lastActivatedTimestamp;
}
