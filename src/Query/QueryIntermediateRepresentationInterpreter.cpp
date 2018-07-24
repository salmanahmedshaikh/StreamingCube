//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Query/QueryIntermediateRepresentationInterpreter.h"
#include "../Common/Types.h"
#include "../Operator/RootOperator.h"
#include "../IO/IStreamOutput.h"
#include "../IO/RelationInput.h"
#include "../IO/CSVInput.h"
#include "../Query/QueryManager.h"
#include "../Operator/DstreamOperator.h"
#include "../Operator/IstreamOperator.h"
#include "../Operator/RstreamOperator.h"
#include "../Operator/ProjectionOperator.h"
#include "../Operator/SelectionOperator.h"
#include "../Operator/JoinOperator.h"
#include "../Operator/RowWindowOperator.h"
#include "../Operator/RangeWindowOperator.h"
#include "../Operator/LeafOperator.h"
#include "../Operator/SmartRowWindowOperator.h"
#include "../Operator/GroupAggregationOperator.h"
#include "../Operator/RelationLeafOperator.h"
#include "../Operator/CSVLeafOperator.h"
#include "../Operator/CubifyOperator.h"
#include "../Query/QueryConditionAbstract.h"
#include "../Query/QueryUtility.h"
#include "../Query/QueryProjectionAbstract.h"
#include "../Server/JsonStreamServer.h"
#include "../Configure/ConfigureManager.h"
//#include "../OLAP/OLAPManager.h"

#include <boost/shared_ptr.hpp>
#include "../Query/QueryEntity.h"
#include "../Utility/QueryIdentityGenerator.h"

QueryIntermediateRepresentationInterpreter* QueryIntermediateRepresentationInterpreter::queryIntermediateRepresentationInterpreter = NULL;
//bool isExecutorNode = false;

QueryIntermediateRepresentationInterpreter::QueryIntermediateRepresentationInterpreter()
{
	streamSourceCount = 0;
	nonMasterStreamSourceCount = 0;
	//replicateStreamSource = false;
}

QueryIntermediateRepresentationInterpreter::~QueryIntermediateRepresentationInterpreter(void)
{

}

QueryIntermediateRepresentationInterpreter* QueryIntermediateRepresentationInterpreter::getInstance()
{
	if (queryIntermediateRepresentationInterpreter == NULL)
	{
		queryIntermediateRepresentationInterpreter = new QueryIntermediateRepresentationInterpreter();
	}
	return queryIntermediateRepresentationInterpreter;
}

boost::shared_ptr<Operator> QueryIntermediateRepresentationInterpreter::resolve(Document& document, QueryEntity* queryEntity)
{
	//std::cout<<document<<std::endl<<std::endl;
	std::string operatorKind = document.getField(QUERY_TYPE).valuestr();
	//std::cout << operatorKind << std::endl;

	if (operatorKind == QUERY_ROOT) {

		boost::shared_ptr<RootOperator> rootOperator(new RootOperator());
		queryEntity->rootOperator = rootOperator;
		//boost::shared_ptr<IStreamOutput> streamOutput = QueryManager::getInstance()->getStreamOutput();
		rootOperator->setStreamOutput(queryEntity->streamOutput);
		rootOperator->setQueryEntity(queryEntity);
		Document inputDocument = document.getField(QUERY_INPUT).embeddedObject();
		//std::cout<<inputDocument<<std::endl;
		boost::shared_ptr<Operator> inputOperator = resolve(inputDocument, queryEntity);
		connectTwoOperator(inputOperator, rootOperator);
		return rootOperator;

	}
	else if (operatorKind == QUERY_CUBIFY)
	{
        std::string configFile = "./configure/cubify.conf";
		boost::shared_ptr<CubifyOperator> cubifyOperator(new CubifyOperator(configFile, queryEntity->getQueryID()));

		cubifyOperator->setQueryEntity(queryEntity);
		OLAPManager::getInstance()->mapQueryIDCubifyOp(queryEntity->getQueryID(), cubifyOperator);

		Document inputDocument = document.getField(QUERY_INPUT).embeddedObject();

		boost::shared_ptr<Operator> inputOperator = resolve(inputDocument, queryEntity);
		connectTwoOperator(inputOperator, cubifyOperator);

		return cubifyOperator;
	}
	else if (operatorKind == QUERY_ISTREAM) {

		boost::shared_ptr<IstreamOperator> istreamOperator(new IstreamOperator());
		Document inputDocument = document.getField(QUERY_INPUT).embeddedObject();
		boost::shared_ptr<Operator> inputOperator = resolve(inputDocument, queryEntity);
		connectTwoOperator(inputOperator, istreamOperator);
		return istreamOperator;
	}
	else if (operatorKind == QUERY_DSTREAM) {

		boost::shared_ptr<DstreamOperator> dstreamOperator(new DstreamOperator());
		Document inputDocument = document.getField(QUERY_INPUT).embeddedObject();
		boost::shared_ptr<Operator> inputOperator = resolve(inputDocument, queryEntity);
		connectTwoOperator(inputOperator, dstreamOperator);
		return dstreamOperator;
	}
	else if (operatorKind == QUERY_RSTREAM) {

		boost::shared_ptr<RstreamOperator> rstreamOperator(new RstreamOperator());
		Document inputDocument = document.getField(QUERY_INPUT).embeddedObject();
		boost::shared_ptr<Operator> inputOperator = resolve(inputDocument, queryEntity);
		connectTwoOperator(inputOperator, rstreamOperator);
		return rstreamOperator;

	} else if (operatorKind == QUERY_PROJECTION) {

		Document projectionAttributeDocument = document.getField( QUERY_PROJECTION_CONTENT).embeddedObject();
		boost::shared_ptr<QueryProjectionAbstract> queryProjection(QueryUtility::resolveQueryProjection(projectionAttributeDocument));
		boost::shared_ptr<ProjectionOperator> projectionOperator(new ProjectionOperator(queryProjection));

		Document inputDocument = document.getField(QUERY_INPUT).embeddedObject();
		boost::shared_ptr<Operator> inputOperator = resolve(inputDocument, queryEntity);
		connectTwoOperator(inputOperator, projectionOperator);
		return projectionOperator;

	} else if (operatorKind == QUERY_SELECTION) {

		boost::shared_ptr<SelectionOperator> selectionOperator(new SelectionOperator());
		Document querySelectionDocument = document.getField(QUERY_SELECTION_CONDITION).embeddedObject();
		selectionOperator->setSelectionCondition(boost::shared_ptr< QueryConditionAbstract>(QueryUtility::resolveQueryCondition(querySelectionDocument)));

		Document inputDocument = document.getField(QUERY_INPUT).embeddedObject();
		boost::shared_ptr<Operator> inputOperator = resolve(inputDocument,	queryEntity);

		connectTwoOperator(inputOperator, selectionOperator);
		return selectionOperator;

	} else if (operatorKind == QUERY_JOIN) {

		boost::shared_ptr<JoinOperator> joinOperator(new JoinOperator());

		bool leftOuter = document.getField(QUERY_LEFT_OUTER).Bool();
		bool rightOuter = document.getField(QUERY_RIGHT_OUTER).Bool();
		joinOperator->setLeftOuter(leftOuter);
		joinOperator->setRightOuter(rightOuter);
		Document leftAttributeDocument = document.getField(QUERY_LEFT_JOIN_ATTRIBUTE).embeddedObject();
		Document rightAttributeDocument = document.getField(QUERY_RIGHT_JOIN_ATTRIBUTE).embeddedObject();
		//std::cout<<leftAttributeDocument<<std::endl;
		//std::cout<<rightAttributeDocument<<std::endl;
		boost::shared_ptr<QueryAttribute> leftQueryAttribute = QueryUtility::resolveQueryAttribute(leftAttributeDocument);
		boost::shared_ptr<QueryAttribute> rightQueryAttribute = QueryUtility::resolveQueryAttribute(rightAttributeDocument);
		joinOperator->setLeftJoinAttribute(leftQueryAttribute);
		joinOperator->setRightJoinAttribute(rightQueryAttribute);
		Document resultProjectionDocument = document.getField(QUERY_PROJECTION_CONTENT).embeddedObject();
		boost::shared_ptr<QueryProjectionAbstract> resultProjection =QueryUtility::resolveQueryProjection(resultProjectionDocument);
		joinOperator->setResultQueryProjection(resultProjection);
		Document leftInputDocument =document.getField(QUERY_LEFT_INPUT).embeddedObject();
		Document rightInputDocument =document.getField(QUERY_RIGHT_INPUT).embeddedObject();
		boost::shared_ptr<Operator> leftInputOperator = resolve(leftInputDocument, queryEntity);
		boost::shared_ptr<Operator> rightInputOperator = resolve(rightInputDocument, queryEntity);

		connectThreeOperator(leftInputOperator, rightInputOperator,joinOperator);
		return joinOperator;

	} else if (operatorKind == QUERY_ROWWINDOW) {
		//std::cout<<document.getField(QUERY_WINDOW_SIZE)<<std::endl;
		int rowWindowSize = document.getField(QUERY_WINDOW_SIZE).numberInt();
		boost::shared_ptr<RowWindowOperator> rowWindowOperator(new RowWindowOperator(rowWindowSize));

		Document inputDocument =document.getField(QUERY_INPUT).embeddedObject();
		boost::shared_ptr<Operator> inputOperator = resolve(inputDocument,queryEntity);
		connectTwoOperator(inputOperator, rowWindowOperator);

		//set the output queue of the leaf operator for the query
		assert(typeid(*inputOperator) == typeid(LeafOperator));
		boost::shared_ptr<LeafOperator> leafOperator =boost::dynamic_pointer_cast<LeafOperator>(inputOperator);
		queryEntity->addOutputQueue(&(*leafOperator),leafOperator->getOutputQueueList().front());

		return rowWindowOperator;

	} else if (operatorKind == QUERY_SMART_ROW_WINDOW) {
		// This block must never execute as there is no keyword smartrowwindow in the query intermediate representation

		assert(false);
		//std::cout<<document.getField(QUERY_WINDOW_SIZE)<<std::endl;
		int rowWindowSize = document.getField(QUERY_WINDOW_SIZE).numberInt();
		//std::cout << "rowWindowSize QIRI " << rowWindowSize << std::endl;
		boost::shared_ptr<SmartRowWindowOperator> smartRowWindowOperator(new SmartRowWindowOperator(rowWindowSize));
		smartRowWindowOperator->addQuery(queryEntity);
		Document inputDocument =document.getField(QUERY_INPUT).embeddedObject();
		boost::shared_ptr<Operator> inputOperator = resolve(inputDocument,queryEntity);
		connectTwoOperator(inputOperator, smartRowWindowOperator);
		//set the output queue of the leaf operator for the query
		assert(typeid(*inputOperator) == typeid(LeafOperator));
		boost::shared_ptr<LeafOperator> leafOperator =boost::dynamic_pointer_cast<LeafOperator>(inputOperator);
		queryEntity->addOutputQueue(&(*leafOperator),leafOperator->getOutputQueueList().front());

		return smartRowWindowOperator;

	} else if (operatorKind == QUERY_RANGEWINDOW) {

		//Timeunit timeRange = document.getField(QUERY_WINDOW_SIZE).Long();
		std::string windowSizeStr =document.getField(QUERY_WINDOW_SIZE).valuestr();
		Timeunit timeRange = std::atol(windowSizeStr.substr(0,windowSizeStr.find(' ')).c_str());
		std::string windowSizeUnit = windowSizeStr.substr(windowSizeStr.find(' ') + 1, windowSizeStr.length());
		//std::cout << "windowSizeUnit " << windowSizeUnit << std::endl;
		if (windowSizeUnit.compare("milliseconds") == 0)
        {
            timeRange = timeRange / 1000.0;
        }

		if (windowSizeUnit.compare("seconds") == 0)
		{
			//timeRange = timeRange;
		} else if (windowSizeUnit.compare("minutes") == 0)
		{
			timeRange = timeRange * 60;
		} else if (windowSizeUnit.compare("hours") == 0)
		{
			timeRange = timeRange * 60 * 60;
		} else if (windowSizeUnit.compare("days") == 0)
		{
			timeRange = timeRange * 60 * 60 * 24;
		}
		//std::cout << "timeRange QIRI " << timeRange << std::endl;
		boost::shared_ptr<RangeWindowOperator> rangeWindowOperator(new RangeWindowOperator(timeRange));
		// Setting the rangeWindowSize for this query.
		queryEntity->setRangeWindowSize(timeRange);

		Document inputDocument =document.getField(QUERY_INPUT).embeddedObject();
		boost::shared_ptr<Operator> inputOperator = resolve(inputDocument,queryEntity);
		connectTwoOperator(inputOperator, rangeWindowOperator);
		//set the output queue of the leaf operator for the query
		assert(typeid(*inputOperator) == typeid(LeafOperator));
		boost::shared_ptr<LeafOperator> leafOperator =boost::dynamic_pointer_cast<LeafOperator>(inputOperator);
		queryEntity->addOutputQueue(&(*leafOperator),leafOperator->getOutputQueueList().front());

		return rangeWindowOperator;

	} else if (operatorKind == QUERY_SMART_RANGE_WINDOW) {
		// This block must never execute as there is no keyword smartrangewindow in the query intermediate representation
		assert(false);

	} else if (operatorKind == QUERY_LEAF)
	{
		streamSourceCount++;
		//std::cout<<document<<std::endl;
		boost::shared_ptr<LeafOperator> leafOperator(new LeafOperator());
		std::string sourceId =document.getField(QUERY_STREAM_SOURCE).valuestr();
		//std::cout << "sourceId " << sourceId << std::endl;
		bool isMasterSource =document.getField(QUERY_IS_MASTER_SOURCE).boolean();

		//std::string nodeType = ConfigureManager::getInstance()->getConfigureValue("node_type");

		//if(isExecutorNode) // get data stream from dispatcher
		//if(!(queryEntity->getIsDispatcher()))
		//std::cout << "Query ID: " << queryEntity->getQueryID() << " | Is Executor " << queryEntity->getIsExecutor() << std::endl;
		if(false){
		//if (queryEntity->getIsExecutor()) {
			std::cout << "Getting input from Dispatcher" << std::endl;
			boost::shared_ptr<IStreamInput> streamInput =IOManager::getInstance()->getStreamInput(sourceId);
			IStreamInput *dispStreamInput = streamInput.get();
			((DispatcherStreamInput *) dispStreamInput)->setIsDispatcherStreamInput(false);
			//std::cout << "StreamInput from QueryInterRepInt " << streamInput << std::endl;

			//boost::shared_ptr<IStreamInput> streamInput(new (IOManager::getInstance()->getStreamInput(sourceId)));
			//DispatcherStreamInput* streamInput = IOManager::getInstance()->getStreamInput(sourceId);
			//leafOperator->setStreamInput(streamInput, isMasterStreamSource);
			leafOperator->setStreamInput(streamInput);

			leafOperator->addQuery(queryEntity);
			queryEntity->addMasterTag(&(*leafOperator), isMasterSource);

		}
		else //direct data reading
		{
			//std::cout << "Reading direct input " << std::endl;
			boost::shared_ptr<IStreamInput> streamInput =QueryManager::getInstance()->getRegisteredStreamById(sourceId);
			//std::cout << "streamInput in QIRI " << streamInput << std::endl;
			//leafOperator->setStreamInput(streamInput, isMasterStreamSource);
			leafOperator->setStreamInput(streamInput);
			//std::cout << "after setting streamInput of leaf in QIRI " << streamInput << std::endl;
			leafOperator->addQuery(queryEntity);

			queryEntity->addMasterTag(&(*leafOperator), isMasterSource);
		}

		/*
		 boost::shared_ptr<IStreamInput> streamInput = QueryManager::getInstance()->getRegisteredStreamById(sourceId);
		 leafOperator->setStreamInput(streamInput);
		 leafOperator->addQuery(queryEntity);
		 queryEntity->addMasterTag(&(*leafOperator), isMasterSource);
		 */
		return leafOperator;

	} else if (operatorKind == QUERY_RELATION_LEAF)
	{
		//std::cout<<document<<std::endl;
		boost::shared_ptr<RelationLeafOperator> relationOperator(new RelationLeafOperator());
		std::string sourceId = document.getField(QUERY_TABLE_SOURCE).valuestr();

		boost::shared_ptr<RelationInput> relationInput =QueryManager::getInstance()->getRegisteredRelationById(sourceId);
		relationOperator->setRelationInput(relationInput);

		return relationOperator;
	}
	else if (operatorKind == QUERY_CSV_LEAF)
	{
        //std::cout<<operatorKind<<std::endl;
		boost::shared_ptr<CSVLeafOperator> csvLeafOperator(new CSVLeafOperator());
		// Get sourceID of dimension i.e., class name of the dimension specified in the query
		// QUERY_CSV_SOURCE = csv_source = productDimension -> [read("productDimension")]
		// The json schema ID (id) mentioned in wrapper file and the one used in query must be same
		std::string sourceId = document.getField(QUERY_CSV_SOURCE).valuestr();

		boost::shared_ptr<CSVInput> CSVInput = QueryManager::getInstance()->getRegisteredCSVById(sourceId);
		csvLeafOperator->setCSVInput(CSVInput);

		return csvLeafOperator;
	}
	else if (operatorKind == QUERY_GROUPBY_AGGREGATION)
	{

    boost::shared_ptr<GroupAggregationOperator> groupAggregationOperator(new GroupAggregationOperator());

    //GROUPBY_ATTRIBUTENAME
    if (document.hasField(QUERY_GROUPBY_ATTRIBUTENAME.c_str())) {

        Document idArray = document.getField(QUERY_GROUPBY_ATTRIBUTENAME).embeddedObject();
        boost::shared_ptr<QueryAttribute> gbAttribute = QueryUtility::resolveQueryAttribute(idArray);

        groupAggregationOperator->setGbAttribute(gbAttribute);
    }

    //aggregations
    Document resultProjectionDocument =document.getField("aggregations").embeddedObject();
    boost::shared_ptr<QueryProjectionAbstract> resultProjection =QueryUtility::resolveQueryProjection(resultProjectionDocument);
    groupAggregationOperator->setResultQueryProjection(resultProjection);

    //INPUT
    Document inputDocument =document.getField(QUERY_INPUT).embeddedObject();
    boost::shared_ptr<Operator> inputOperator = resolve(inputDocument,queryEntity);

    connectTwoOperator(inputOperator, groupAggregationOperator);

    return groupAggregationOperator;

    }
    else
    {
        assert(false); //never reached
    }
}

void QueryIntermediateRepresentationInterpreter::connectTwoOperator(boost::shared_ptr<Operator> inputOperator,boost::shared_ptr<Operator> outputOperator)
{
	boost::shared_ptr<QueueEntity> queueEntity(new QueueEntity());
	inputOperator->addOutputQueue(queueEntity);
	queueEntity->setInputOperator(inputOperator);
	outputOperator->addInputQueue(queueEntity);
	queueEntity->setOutputOperator(outputOperator);

}

void QueryIntermediateRepresentationInterpreter::connectThreeOperator(boost::shared_ptr<Operator> leftInputOperator, boost::shared_ptr<Operator> rightInputOperator,boost::shared_ptr<Operator> outputOperator)
{
	boost::shared_ptr<QueueEntity> leftQueueEntity(new QueueEntity());
	leftInputOperator->addOutputQueue(leftQueueEntity);
	leftQueueEntity->setInputOperator(leftInputOperator);
	outputOperator->addInputQueue(leftQueueEntity);
	leftQueueEntity->setOutputOperator(outputOperator);

	boost::shared_ptr<QueueEntity> rightQueueEntity(new QueueEntity());
	rightInputOperator->addOutputQueue(rightQueueEntity);
	rightQueueEntity->setInputOperator(rightInputOperator);
	outputOperator->addInputQueue(rightQueueEntity);
	rightQueueEntity->setOutputOperator(outputOperator);
}
