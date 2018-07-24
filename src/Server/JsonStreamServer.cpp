//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////


#include "../Common/stdafx.h"
#include "../Server/JsonStreamServer.h"
#include "../BinaryJson/json.h"
#include "../Internal/Memory/MemoryManager.h"
#include "../Common/Types.h"
#include "../Internal/Element/Element.h"
#include "../Utility/TimestampGenerator.h"
#include "../Utility/DocumentIdentifierGenerator.h"
#include "../Internal/Queue/QueueEntity.h"
#include "../Internal/Synopsis/RelationSynopsis.h"
#include "../Internal/Synopsis/LineageSynopsis.h"
#include "../Server/JsonStreamServer.h"
#include "../Schema/JsonSchema.h"
#include "../Schema/SchemaInterpreter.h"
#include "../Operator/Operator.h"
#include "../Operator/CubifyOperator.h"
#include "../Operator/RowWindowOperator.h"
#include "../Operator/JoinOperator.h"
#include "../Operator/SelectionOperator.h"
#include "../Operator/ProjectionOperator.h"
#include "../Operator/RootOperator.h"
#include "../Operator/RstreamOperator.h"
#include "../Operator/IstreamOperator.h"
#include "../Operator/DstreamOperator.h"
#include "../Operator/LeafOperator.h"
#include "../Operator/RelationLeafOperator.h"
#include "../Scheduler/Scheduler.h"
#include "../Operator/SmartRowWindowOperator.h"
#include "../Utility/TimeCounterUtility.h"
#include "../Utility/WorkerStatus.h"
#include "../Utility/CPUUtilization.h"
#include "../IO/IOManager.h"
#include "../IO/SocketStreamInput.h"
#include "../IO/csvOutput.h"
#include "../IO/SpecifiedInputRateStreamInput.h"
#include "../IO/IStreamInput.h"
#include "../IO/RandomGeneratedStreamInput.h"
#include "../IO/ShowResultStreamOutput.h"
#include "../IO/IStreamOutput.h"
#include "../Query/QueryManager.h"
#include "../Query/QueryIntermediateRepresentation.h"
#include "../Query/GetJaqlQuery.h"
#include "../Schema/JsonSchema.h"
#include "../Plan/PlanManager.h"
#include "../Parser/ParserManager.h"
#include "../Schema/SchemaManager.h"
#include "../Command/CommandManager.h"
#include "../Configure/ConfigureManager.h"
#include "../Wrapper/WrapperManager.h"
#include "../Query/QueryEntity.h"
#include "../IO/SocketStreamOutput.h"
#include "../IO/DispatcherStreamInput.h"
#include "../Dispatcher/DispatcherManager.h"
#include "../Utility/HttpParser.h"
#include "../Operator/CSVLeafOperator.h"

#include <sys/time.h>
#include <unistd.h>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

JsonStreamServer* JsonStreamServer::jsonStreamServer = NULL;
JsonStreamServer::JsonStreamServer(void)
{
	initial(); //initialize the server
}

JsonStreamServer::~JsonStreamServer(void)
{

}

JsonStreamServer* JsonStreamServer::getInstance(void)
{
	if (jsonStreamServer == NULL)
	{
		jsonStreamServer = new JsonStreamServer();
	}
	return jsonStreamServer;
}

void JsonStreamServer::setShutdownJsSpinner(bool serverStatus)
{
    this->shutdownJsSpinner = serverStatus;
}

/* initial each module manager in the system, which initialize the Jsspinner*/
void JsonStreamServer::initial()
{
	//std::cout<<"initial"<<std::endl;
	ConfigureManager::getInstance(); // read configuration file, this manager should be initialed first
	//std::cout<<"configuration initialed"<<std::endl;
	IOManager::getInstance();        // deal with IO socket
	//std::cout<<"IOManager initialed"<<std::endl;
	MemoryManager::getInstance(); // allocate and manage memory for queues and synopsis
	//std::cout<<"MemoryManager initialed"<<std::endl;
	PlanManager::getInstance(); // generate query plan and instantiate operators by query intermediate representation
	//std::cout<<"PlanManager initialed"<<std::endl;
	ParserManager::getInstance(); // manage the input and output of the query registered
	//std::cout<<"ParserManager initialed"<<std::endl;
	QueryManager::getInstance(); // manage the input and output of the query registered
	//std::cout<<"QueryManager initialed"<<std::endl;
	SchemaManager::getInstance();    // register schema information
	//std::cout<<"SchemaManager initialed"<<std::endl;
	CommandManager::getInstance(); // system listens to a port waiting for command
	//std::cout<<"CommandManager initialed"<<std::endl;
	WrapperManager::getInstance(); // read the wrapper specification in the wrapper folder, and register each wrapper
	//std::cout<<"WrapperManager initialed"<<std::endl;
	DispatcherManager::getInstance();  // responsible for accepting input from dispatcher
	//std::cout<<"DispatcherManager initialized"<<std::endl;
}


/* server execution, each time get one operator and the execution times from the scheduler */
void JsonStreamServer::execute()
{
	struct timeval begin,end;
    double systemThroughput = 0.0;
	bool queryExecutionTimeBegin = false;
	bool allCSVLeafOperatorsExecuted = false;
    int systemExecutionSecond = atoi(ConfigureManager::getInstance()->getConfigureValue("SystemExecSeconds").c_str());
    //std::cout << "systemExecutionSecond " << systemExecutionSecond << std::endl;
    //int systemExecutionSecond = 17;

    std::string numQueriesStr = ConfigureManager::getInstance()->getConfigureValue("num_queries");
    int numQueries = atoi(numQueriesStr.c_str());
    bool isQuerySharing = true;

    #ifdef SHARE_RANGE_WINDOW
    isQuerySharing = true;
    #endif // SHARE_RANGE_WINDOW

    #ifdef SHARE_SMART_WINDOW
    isQuerySharing = true;
    #endif // SHARE_SMART_WINDOW

    #ifdef SHARE_NO_OPERATOR
    isQuerySharing = false;
    #endif // SHARE_RANGE_WINDOW



	IOManager::getInstance()->execute();
	TimeCounterUtility::getInstance()->reset();
	gettimeofday(&begin, NULL);
	//begin.tv_sec = -9999999;
	//begin.tv_usec = -9999999;



	while (1) {

		boost::shared_ptr<Operator> op;

		if (!Scheduler::getInstance()->isEmpty()) // check if there is an operator waiting to execute
		{
            Scheduler::getInstance()->getNextOperatorToExecute(op);
            //std::cout << typeid(*op).name() << std::endl;

            //if( !queryExecutionTimeBegin )
            // For queries with CSVLeafOperator
			if( !queryExecutionTimeBegin &&  allCSVLeafOperatorsExecuted )
            {
                gettimeofday(&begin,NULL);
                queryExecutionTimeBegin = true;
                //std::cout << "time begin " << begin.tv_sec * 1000000 + begin.tv_usec << std::endl;
            }

			gettimeofday(&end, NULL);

            double diff = ((end.tv_sec * 1000000 + end.tv_usec) - (begin.tv_sec * 1000000 + begin.tv_usec))/1000000.0;
            //std::cout << "diff " << diff << std::endl;

			if(diff >= systemExecutionSecond) //system running for specified number of seconds
			//if(false)
			{
                    ofstream OutputFile;
                    //std::string outputFile = "streamingCubeQueryingTime.txt";
                    std::string outputFile = "streamOLAPEvaluation.txt";
                    int queryID = 0;

                    //OutputFile.open(outputFile.c_str(), std::ofstream::out | std::ofstream::app);
                    //OutputFile << "*********************** streamOLAPEvaluation ***********************\n";
                    /*
                    OutputFile << "Storage Cost (Bytes) \nAvg. Querying Cost (s) \nMax. Throughput (tuples/s) \nMaintenance Cost (per tuple): \nNetwork Load (Bytes/s): \n\n";
                    OutputFile << OLAPManager::getInstance()->getConfigValue(queryID, "MMode") << "\n";
                    OutputFile << OLAPManager::getInstance()->getConfigValue(queryID, "RFMode") << "\n";
                    OutputFile << OLAPManager::getInstance()->getNumOfMaterializedVertices(queryID) << "\n";
                    OutputFile << OLAPManager::getInstance()->getConfigValue(queryID, "MaxStorageNumTuples") << "\n";
                    */

                    //OutputFile << "Num of Materialized Vertices" << "\n";
                    //OutputFile << "IoI" << "\n";
                    //OutputFile << "SystemExecSeconds" << "\n";

                    //OutputFile << OLAPManager::getInstance()->getConfigValue(queryID, "MVerticesNum") << "\n";
                    //OutputFile << ConfigureManager::getInstance()->getConfigureValue("IoI") << "\n";
                    //OutputFile << ConfigureManager::getInstance()->getConfigureValue("SystemExecSeconds") << "\n";

                    //OutputFile << "**********************************************\n";

                    //OutputFile.close();


                    long int storageCost = 0;
                    double avgQueryingCost = 0;

                    //std::cout << LeafOperator::totalInputNumber/(systemExecutionSecond*1.0) <<  "," << QueueEntity::total_dequeue_number/(systemExecutionSecond*1.0) << std::endl;
                    CommandManager::getInstance()->processExperimentalQueryCommand(queryID, outputFile, avgQueryingCost, storageCost);
                    std::cout << OLAPManager::getInstance()->getNumOfMaterializedVertices(queryID) << "," << storageCost << "," << avgQueryingCost << "," << LeafOperator::totalInputNumber/(systemExecutionSecond*1.0) << "," << 1/(LeafOperator::totalInputNumber/(systemExecutionSecond*1.0)) << "," << IOManager::socketBytesSent/(systemExecutionSecond*1.0)  << std::endl;
                    //std::cout << IOManager::socketBytesSent/(systemExecutionSecond*1.0)  << ",";

                    //std::cout << "Storage Cost (Bytes): " << storageCost << std::endl;
                    //std::cout << "Avg. Querying Cost (s): " << avgQueryingCost << std::endl;
                    //std::cout << "Max. Throughput (tuples/s): " << LeafOperator::totalInputNumber/(systemExecutionSecond*1.0) << std::endl;
                    //std::cout << "Maintenance Cost (per tuple): " << 1/(LeafOperator::totalInputNumber/(systemExecutionSecond*1.0)) << std::endl;
                    //std::cout << "Network Load (Bytes/s): " << IOManager::socketBytesSent/(systemExecutionSecond*1.0) << std::endl;
                    //std::cout << CubifyOperator::insertedTupleCounter/(systemExecutionSecond*1.0) << std::endl;
                    //OutputFile.close();
                    break;
			}

			//Scheduler::getInstance()->getNextOperatorToExecute(op);

			//create a thread for RelationLeafOperator?
			if (typeid(*op).name() == typeid(RelationLeafOperator).name())
			{
				boost::shared_ptr<RelationLeafOperator> relationLeafOperator = boost::static_pointer_cast<RelationLeafOperator>(op);
				relationLeafOperator->start();
			}
			else if (typeid(*op).name() == typeid(CSVLeafOperator).name())
			{
                boost::shared_ptr<CSVLeafOperator> csvLeafOperator = boost::static_pointer_cast<CSVLeafOperator>(op);
                if(!(csvLeafOperator->getIsCSVLeafExecuted()))
                {
                    op->execution(); //execute the operator
                    gettimeofday(&begin, NULL);
                }
                else
                {
                    allCSVLeafOperatorsExecuted = true;
                }
			}
			else
			{
				op->execution(); //execute the operator
			}


		} else { //release CPU resource
				 //Sleep(5000);

		}
	}

}

void JsonStreamServer::registerQuery(std::string jaqlQueryString, boost::shared_ptr<IStreamOutput> streamOutput, std::string durationSpecification)
{
	boost::shared_ptr<QueryIntermediateRepresentation> intermediateQuery = ParserManager::getInstance()->processQuery(jaqlQueryString);
	//std::cout << "intermediateQuery " << intermediateQuery << std::endl;
	registerQuery(intermediateQuery, streamOutput, durationSpecification);
}

int JsonStreamServer::registerQuery(boost::shared_ptr<QueryIntermediateRepresentation> queryIntermediateRepresentation, boost::shared_ptr<IStreamOutput> streamOutput,	std::string durationSpecification)
{
	boost::shared_ptr<QueryEntity> queryEntity = QueryManager::getInstance()->addQuery(queryIntermediateRepresentation,streamOutput, durationSpecification);
	PlanManager::getInstance()->createQueryPlan(queryEntity); // generate query plan of the registered query

	stringstream queryID;
	queryID << queryEntity->getQueryID();
	//cout << "QueryID: " << queryID.str() << std::endl;
    HttpParser::getInstance()->insertInPostRequestMap("QueryID",queryID.str());

	Scheduler::getInstance()->setOperatorList(PlanManager::getInstance()->getAllOperators());// inform scheduler

	return atoi((queryID.str()).c_str());
}

int JsonStreamServer::registerExecutorQuery(boost::shared_ptr<QueryIntermediateRepresentation> queryIntermediateRepresentation, boost::shared_ptr<IStreamOutput> streamOutput,	std::string durationSpecification)
{
	boost::shared_ptr<QueryEntity> queryEntity = QueryManager::getInstance()->addQuery(queryIntermediateRepresentation,streamOutput, durationSpecification);
	queryEntity->setIsExecutor(true);
	PlanManager::getInstance()->createQueryPlan(queryEntity); // generate query plan of the registered query

	stringstream queryID;
	queryID << queryEntity->getQueryID();
	//cout << "QueryID: " << queryID.str() << std::endl;
    HttpParser::getInstance()->insertInPostRequestMap("QueryID",queryID.str());

	Scheduler::getInstance()->setOperatorList(PlanManager::getInstance()->getAllOperators());// inform scheduler

	return atoi((queryID.str()).c_str());
}

void JsonStreamServer::registerQuery(boost::shared_ptr<QueryIntermediateRepresentation> queryIntermediateRepresentation, boost::shared_ptr<IStreamOutput> streamOutput,	std::string durationSpecification, bool isDispatcher)
{
	boost::shared_ptr<QueryEntity> queryEntity = QueryManager::getInstance()->addQuery(queryIntermediateRepresentation,streamOutput, durationSpecification, isDispatcher);

	PlanManager::getInstance()->createQueryPlan(queryEntity); // generate query plan of the registered query
	Scheduler::getInstance()->setOperatorList(PlanManager::getInstance()->getAllOperators());// inform scheduler
}

void JsonStreamServer::registerStreamInput(boost::shared_ptr<IStreamInput> streamInput)
{
	QueryManager::getInstance()->registerStream(streamInput); //register query
}
