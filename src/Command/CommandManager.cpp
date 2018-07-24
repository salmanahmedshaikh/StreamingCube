//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Command/CommandManager.h"
#include "../IO/IOManager.h"
#include "../Common/Types.h"
#include "../BinaryJson/json.h"
#include "../IO/RandomGeneratedStreamInput.h"
#include "../Server/JsonStreamServer.h"
#include "../IO/SocketStreamInput.h"
#include "../Query/QueryIntermediateRepresentation.h"
#include "../IO/SocketStreamOutput.h"
#include "../Parser/ParserManager.h"
#include "../IO/IStreamOutput.h"
#include "../IO/IStreamInput.h"
#include "../IO/ShowResultStreamOutput.h"
#include "../IO/csvOutput.h"
#include "../Query/QueryManager.h"
#include "../Plan/PlanManager.h"
#include "../Scheduler/Scheduler.h"
#include "../Wrapper/WrapperManager.h"
#include "../Server/JsonStreamServer.h"
#include "../Utility/MyStatus.h"
#include "../Utility/HttpParser.h"
#include "../Utility/TimeCounterUtility.h"
#include "../Configure/ConfigureManager.h"
#include "../Dispatcher/DispatcherManager.h"
#include "../Operator/CollectStream.h"
#include "../OLAP/MaterializedLattice.h"
#include "../OLAP/OLAPManager.h"
#include "../Operator/CubifyOperator.h"
#include "../Utility/AvgStdDeviation.h"
#include <boost/filesystem.hpp>
#include <fstream>


CommandManager* CommandManager::commandManager = NULL;
CommandManager::CommandManager(void)
{
	initial();
}


CommandManager::~CommandManager(void)
{
}

CommandManager* CommandManager::getInstance()
{
	if(commandManager==NULL)
	{
		commandManager = new CommandManager();
	}
	return commandManager;
}

void processElement(Element& element)
{
    //std::cout<<element.document.getField("sex").valuestr();
    //std::cout<<element<<std::endl;
}

//use IOManager to listen to a port waiting for command
void CommandManager::initial()
{
	//use IOManager to listen to a port waiting for command
	//salman
	IOManager::getInstance()->addCommandInput(MyStatus::getInstance()->getMyIP(), ConfigureManager::getInstance()->getConfigureValue("worker_command_port"),this);
	IOManager::getInstance()->addHttpCommandInput(MyStatus::getInstance()->getMyIP(), ConfigureManager::getInstance()->getConfigureValue("http_command_port"),this);
	//IOManager::getInstance()->addCommandInput(SERVER_IP,SERVER_PORT,this);
}

bool CommandManager::sendHttpResponce(struct evhttp_request * req, std::string responceText)
{
    //struct evkeyvalq headers;
    //evhttp_parse_query(req->uri, &headers);

    //struct evkeyvalq * kv = evhttp_request_get_input_headers(req);
    //std::cout << "Content-Type " << evhttp_find_header(inputHeaderKV, "Content-Type") << std::endl;
    //std::cout << "Document " << evhttp_find_header(inputHeaderKV, "Document") << std::endl;

    // Add heading text
    //evbuffer_add_printf(evb, "<HTML><HEAD><TITLE>%s Page</TITLE></HEAD><BODY>\n", SERVER_NAME);    // Add buffer
    //evbuffer_add(evb, RESPONCE, BUF_LEN);
    // Add formatted text
    //evbuffer_add_printf(evb, "Your request is <B>%s</B> from <B>%s</B>.<BR/>Your user agent is '%s'\n",req->uri, req->remote_host, evhttp_find_header(req->input_headers, "User-Agent"));
    // Add footer
    //evbuffer_add_printf(evb, "</BODY></HTML>");

    const short  BUF_LEN           = 26;
    const char   RESPONCE[BUF_LEN] = "";

    // Create responce buffer
    struct evbuffer *evb = evbuffer_new();
    if (!evb) { return false; }

    evbuffer_add_printf(evb, responceText.c_str());    // Add buffer

    // Adding responce headers
    evhttp_add_header(req->output_headers, "Access-Control-Allow-Origin", "*");
    evhttp_add_header(req->output_headers, "Content-Type", "text/html");
    //std::cout << "inside sendHttpResponce " << std::endl;

    // Send reply
    evhttp_send_reply(req, HTTP_OK, "OK", evb);

    // Free memory
    evbuffer_free(evb);

    return true;
}

bool CommandManager::processHttpRequest(struct evhttp_request * req)
{
    //char input_buffer[4096];
    char input_buffer[8192];
    struct evbuffer *input;

    input = req->input_buffer;
    size_t buffer_len = evbuffer_get_length(input);
    //if (buffer_len <= 0)
	//	return false;
	// evbuffer_remove needs to be executed whether or not HTTP request
	evbuffer_remove(input,input_buffer,buffer_len);

	std::string httpRequestStr(input_buffer);

    const char *uri = evhttp_request_uri(req);
    std::string commandType(uri);

    //std::cout << "httpRequestStr " << httpRequestStr << std::endl;
    HttpParser::getInstance()->parsePostRequestBody(httpRequestStr, HTTP_POST_BOUNDARY);

    std::cout << "CommandManager::commandType " << commandType << std::endl;

    if(commandType == SUBMIT)
    {
        std::string query = HttpParser::getInstance()->getPostRequestMapValue("query");
        std::cout << "query " << query << std::endl;

        if(!(query == "KeyNotFound"))
        {
            if (sendHttpResponce(req,"{\"_status\":\"OK\"}"))
                processHttpQuery(query);
        }
        else
        {
            std::cout << "Invalid query! " << std::endl;
            exit(0);
        }
    }
    else if(commandType == "/submitInitialOLAPQuery")
    {
        if(processHttpIntermediateQuery(httpRequestStr));
            sendHttpResponce(req,"{\"_status\":\"Query Submitted\"}");
    }
    else if(commandType == "/OLAPGetLatticeWithInfo")
    {
        std::string OLAPLatticeJSONStr = processOLAPGetLatticeWithInfo();
        std::cout << OLAPLatticeJSONStr << std::endl;
        if(!OLAPLatticeJSONStr.empty())
        {
            sendHttpResponce(req, OLAPLatticeJSONStr);
        }
        else
        {
            std::cout << "CommandManager: No lattice available for the given query." << std::endl;
            exit(0);
        }
    }
    else if(commandType == "/JsOLAPStatus")
	{
        std::stringstream JsOLAPStatusStr;
        std::list<boost::shared_ptr<QueryEntity> > queryEntityList = QueryManager::getInstance()->getQueryEntityList();
        //std::cout << "queryEntityList size " << queryEntityList.size() << std::endl;
        JsOLAPStatusStr << "{\"_status\":\"Running\", \"numOfQueriesInExecution\":" << queryEntityList.size() << "}";
        std::cout << "JsOLAPStatusStr " << JsOLAPStatusStr.str() << std::endl;
        sendHttpResponce(req, JsOLAPStatusStr.str());
	}
    else if(commandType == SHUTDOWN)
	{
        if (sendHttpResponce(req,"{\"_status\":\"OK\"}"))
            shutdownJsSpinner();
	}
	else if(commandType == "/OLAPGetLattice")
    {
        std::string OLAPLatticeJSONStr = processOLAPGetLattice();
        if(!OLAPLatticeJSONStr.empty())
        {
            sendHttpResponce(req, OLAPLatticeJSONStr);
        }
        else
        {
            std::cout << "CommandManager: No lattice available for the given query." << std::endl;
            exit(0);
        }
    }
    else if(commandType == "/OLAPQuery")
    {
        //std::string OLAPQueryResultJSONStr = processOLAPQuery(httpRequestStr);
        std::string OLAPQueryResultJSONStr = processOLAPQueryWTimeGrain(httpRequestStr);

        if(!OLAPQueryResultJSONStr.empty())
        {
            sendHttpResponce(req, OLAPQueryResultJSONStr);
        }
        else
        {
            std::cout << "CommandManager: No result found for the given query." << std::endl;
            exit(0);
        }
    }
    else if(commandType == "/OLAPDrilldown")
    {
        std::string OLAPDrilldownNodesJSONStr = processOLAPDrilldown(httpRequestStr);
        if(!OLAPDrilldownNodesJSONStr.empty())
        {
            sendHttpResponce(req, OLAPDrilldownNodesJSONStr);
        }
        else
        {
            std::cout << "CommandManager: No result found for the given query." << std::endl;
            exit(0);
        }
    }
    else if(commandType == "/OLAPRollup")
    {
        std::string OLAPRollupNodesJSONStr = processOLAPRollup(httpRequestStr);
        if(!OLAPRollupNodesJSONStr.empty())
        {
            sendHttpResponce(req, OLAPRollupNodesJSONStr);
        }
        else
        {
            std::cout << "CommandManager: No result found for the given query." << std::endl;
            exit(0);
        }
    }
    else if(commandType == "/generateConfigFile")
    {
        std::string fileLocation = "configure/cubify.conf";
        generateCubifyConfigFile(fileLocation, httpRequestStr);
    }
    else if(commandType == "/stopQuery")
    {
        if(processStopQuery(httpRequestStr))
            sendHttpResponce(req,"{\"_status\":\"Query Stopped\"}");
    }
    else if(commandType == "/experimentalQuery")
    {
        int queryID = 0;
        long int storageCost = 0;
        double avgQueryingCost = 0;
        std::string outputFile = "StreamingCubeExperiments.txt";
        //std::cout << "experimentalQuery Executed" << std::endl;
        if(processExperimentalQueryCommand(queryID, outputFile, avgQueryingCost, storageCost))
        {
            std::stringstream responseText;
            responseText << "{\"Status\":\"";
            responseText << "Execution Finished";
            responseText << "\"}";

            sendHttpResponce(req, responseText.str());
        }
    }

    return true;
}

bool CommandManager::processStopQuery(std::string httpRequestStr)
{
    std::string httpRequestSubstr = getCoreHttpRequestStr(httpRequestStr);

    Document httpRequestDoc = fromjson(httpRequestSubstr);
    int requestedQueryID = httpRequestDoc.getField("queryID").Int();
    //std::cout << queryID << std::endl;

    std::list<boost::shared_ptr<QueryEntity> > queryEntityList = QueryManager::getInstance()->getQueryEntityList();
    std::list<boost::shared_ptr<QueryEntity> >::iterator queryEntityListIt;

    for(queryEntityListIt = queryEntityList.begin(); queryEntityListIt != queryEntityList.end(); queryEntityListIt++)
    {
        boost::shared_ptr<QueryEntity> queryEntity = *queryEntityListIt;

        if(requestedQueryID == queryEntity->getQueryID())
        {
            std::cout << "queryEntityList Size " << QueryManager::getInstance()->getQueryEntityList().size() << std::endl;
            std::cout << "Query IDs " << queryEntity->getQueryID() << std::endl;
            QueryManager::getInstance()->removeQuery(*queryEntityListIt);

            std::cout << "queryEntityList Size " << QueryManager::getInstance()->getQueryEntityList().size() << std::endl;
        }
    }

    return true;
}

void CommandManager::generateCubifyConfigFile(std::string fileLocation, std::string fileContents)
{
    // Saving own PID
    std::ofstream configFile;
    std::size_t pos = 0;

    while((pos = fileContents.find('$', pos+1)) != std::string::npos)
    {
        fileContents.replace(pos, 1, "\n");
    }
    configFile.open(fileLocation.c_str(), ios::out|ios::binary);
    configFile << fileContents;
    configFile.close();
}

std::string CommandManager::processOLAPGetLatticeWithInfo()
{
    boost::shared_ptr<CubifyOperator> cubifyOp = OLAPManager::getInstance()->getCubifyOpByQueryID(0); // Assuming queryID = 0 temporarily
    boost::shared_ptr<MaterializedLattice> mLattice = cubifyOp->getAssociatedMaterializedLattice();

    std::map<vertexInfo, std::vector<DimensionKeyName> > allLatticeNodesWithInfo;
    mLattice->getAllLatticeNodesWithInfo(allLatticeNodesWithInfo);

    std::stringstream OLAPLatticeJSONStr;
    OLAPLatticeJSONStr << "{\"latticeNodes\":[";

    std::map<vertexInfo, std::vector<DimensionKeyName> >::iterator allLatticeNodesWithInfoIt;
    for(allLatticeNodesWithInfoIt = allLatticeNodesWithInfo.begin(); allLatticeNodesWithInfoIt != allLatticeNodesWithInfo.end(); allLatticeNodesWithInfoIt++)
    {
        std::stringstream OLAPLatticeDims;
        for(int i = 0; i < allLatticeNodesWithInfoIt->second.size(); i++)
        {
            if(i < allLatticeNodesWithInfoIt->second.size() - 1)
                OLAPLatticeDims << allLatticeNodesWithInfoIt->second[i] << ", ";
            else
                OLAPLatticeDims << allLatticeNodesWithInfoIt->second[i];
            /*
            if(i < allLatticeNodesWithInfoIt->second.size() - 1)
                OLAPLatticeDims << "\"" << allLatticeNodesWithInfoIt->second[i] << "\", ";
            else
                OLAPLatticeDims << "\"" << allLatticeNodesWithInfoIt->second[i] << "\"";
            */
        }

        std::stringstream isMaterialized;
        if(allLatticeNodesWithInfoIt->first.isMaterialized == 1)
            isMaterialized << "Yes";
        else
            isMaterialized << "No";

        if( std::distance(allLatticeNodesWithInfo.begin(), allLatticeNodesWithInfoIt) ==  (allLatticeNodesWithInfo.size() - 1) )
            OLAPLatticeJSONStr << "{\"NodeID\":\"" << allLatticeNodesWithInfoIt->first.vertexID << "\", \"IsMaterialized\":\"" << isMaterialized.str() << "\", \"Dimensions\":\"" << OLAPLatticeDims.str() << "\"}";
        else
            OLAPLatticeJSONStr << "{\"NodeID\":\"" << allLatticeNodesWithInfoIt->first.vertexID << "\", \"IsMaterialized\":\"" << isMaterialized.str() << "\", \"Dimensions\":\"" << OLAPLatticeDims.str() << "\"},";
    }

    OLAPLatticeJSONStr << "]}";
    //std::cout << OLAPLatticeJSONStr.str() << std::endl;

    return OLAPLatticeJSONStr.str();
}

std::string CommandManager::processOLAPGetLattice()
{
    std::stringstream OLAPLatticeJSONStr;

    boost::shared_ptr<CubifyOperator> cubifyOp = OLAPManager::getInstance()->getCubifyOpByQueryID(0); // Assuming queryID = 0 temporarily
    boost::shared_ptr<MaterializedLattice> mLattice = cubifyOp->getAssociatedMaterializedLattice();

    std::vector<std::string> dimensionNamesVector;
    OLAPManager::getInstance()->getConfigValue(0, "DimensionNames", dimensionNamesVector);

    OLAPLatticeJSONStr << "{\"dimensionNames\":[";
    for(int i = 0; i < dimensionNamesVector.size(); i++)
    {
        if( i ==  (dimensionNamesVector.size() - 1) )
            OLAPLatticeJSONStr << "{\"dimID\":\"" << i+1 << "\", \"dimName\":\"" << dimensionNamesVector[i] << "\"}";
        else
            OLAPLatticeJSONStr << "{\"dimID\":\"" << i+1 << "\", \"dimName\":\"" << dimensionNamesVector[i] << "\"},";
    }
    OLAPLatticeJSONStr << "], ";

    std::vector<std::string> dimensionsVector;
    OLAPManager::getInstance()->getConfigValue(0, "Dimensions", dimensionsVector);

    OLAPLatticeJSONStr << "\"dimensions\":[";
    for(int i = 0; i < dimensionsVector.size(); i++)
    {
        if( i ==  (dimensionsVector.size() - 1) )
            OLAPLatticeJSONStr << "{\"dimAttributeID\":\"" << i+1 << "\", \"dimAttribute\":\"" << dimensionsVector[i] << "\"}";
        else
            OLAPLatticeJSONStr << "{\"dimAttributeID\":\"" << i+1 << "\", \"dimAttribute\":\"" << dimensionsVector[i] << "\"},";
    }
    OLAPLatticeJSONStr << "], ";

    std::map<int, std::vector<DimensionKeyName> > latticeNodes;
    mLattice->getAllLatticeNodes(latticeNodes);

    OLAPLatticeJSONStr << "\"latticeNodes\":[";

    std::map<int, std::vector<DimensionKeyName> >::iterator latticeNodesIt;
    for(latticeNodesIt = latticeNodes.begin(); latticeNodesIt != latticeNodes.end(); latticeNodesIt++)
    {
        std::stringstream OLAPLatticeDims;
        for(int i = 0; i < latticeNodesIt->second.size(); i++)
        {
            if(i < latticeNodesIt->second.size() - 1)
                OLAPLatticeDims << latticeNodesIt->second[i] << ", ";
            else
                OLAPLatticeDims << latticeNodesIt->second[i];
        }

        if( std::distance(latticeNodes.begin(), latticeNodesIt) ==  (latticeNodes.size() - 1) )
            OLAPLatticeJSONStr << "{\"nodeID\":\"" << latticeNodesIt->first << "\", \"nodeAttributes\":\"" << OLAPLatticeDims.str() << "\"}";
        else
            OLAPLatticeJSONStr << "{\"nodeID\":\"" << latticeNodesIt->first << "\", \"nodeAttributes\":\"" << OLAPLatticeDims.str() << "\"},";
    }

    OLAPLatticeJSONStr << "]}";
    //std::cout << OLAPLatticeJSONStr.str() << std::endl;
    return OLAPLatticeJSONStr.str();
}

std::string CommandManager::getCoreHttpRequestStr(std::string httpRequestStr)
{
    size_t pos1 = httpRequestStr.find("-----------------------------19588288329222");
    size_t pos2 = httpRequestStr.find("-----------------------------19588288329222", pos1 + 1);

    std::string httpRequestSubstr = httpRequestStr.substr(43, pos2 - (pos1 + 43));

    return httpRequestSubstr;
}

std::string CommandManager::processOLAPQuery(std::string httpRequestStr)
{
    std::stringstream OLAPQueryResultJSONStr;

    boost::shared_ptr<CubifyOperator> cubifyOp = OLAPManager::getInstance()->getCubifyOpByQueryID(0); // Assuming queryID = 0 temporarily
    boost::shared_ptr<MaterializedLattice> mLattice = cubifyOp->getAssociatedMaterializedLattice();

    std::map<std::vector<DimensionKey>, double> keyValues;
    std::map<std::vector<DimensionKey>, double>::iterator keyValuesIt;
    std::vector<DimensionKeyName> queryDimensions;

    std::string httpRequestSubstr = getCoreHttpRequestStr(httpRequestStr);

    Document httpRequestDoc = fromjson(httpRequestSubstr);
    int nodeID = httpRequestDoc.getField("nodeID").Int();
    std::string aggrOp = httpRequestDoc.getField("aggrOp").valuestr();
    std::string timeGrain = httpRequestDoc.getField("timeGrain").valuestr();
    //std::cout << nodeID << ", " << aggrOp << ", " << timeGrain << std::endl;
    mLattice->getLatticeNodeByID(nodeID, queryDimensions);

    mLattice->getAllAggregatedValuesForANode(queryDimensions, aggrOp, keyValues);
    Timestamp resultTimestamp = cubifyOp->getLatestElementTimestamp();

    OLAPQueryResultJSONStr << "{\"resultTimestamp\":" << resultTimestamp << ", \"queryResult\":[";

    for(keyValuesIt = keyValues.begin(); keyValuesIt != keyValues.end(); keyValuesIt++)
    {
        OLAPQueryResultJSONStr << "{";

        for(int i = 0; i < queryDimensions.size(); i++)
        {
            std::string dimValueStr = mLattice->getDimValue(queryDimensions[i], keyValuesIt->first[i]);

            //OLAPQueryResultJSONStr << "\"" << queryDimensions[i] << "\":\"" << keyValuesIt->first[i] << "\", ";
            OLAPQueryResultJSONStr << "\"" << queryDimensions[i] << "\":\"" << dimValueStr << "\", ";
            //std::cout << OLAPQueryResultJSONStr.str() << std::endl;
        }

        if(std::distance(keyValues.begin(), keyValuesIt) < keyValues.size() - 1)
            OLAPQueryResultJSONStr << "\"value\":" << keyValuesIt->second << "},";
        else
            OLAPQueryResultJSONStr << "\"value\":" << keyValuesIt->second << "}";
    }

    OLAPQueryResultJSONStr << "]}";

    std::cout << OLAPQueryResultJSONStr.str() << std::endl;

    return OLAPQueryResultJSONStr.str();
}

std::string CommandManager::processOLAPQueryWTimeGrain(std::string httpRequestStr)
{
    std::stringstream OLAPQueryResultJSONStr;

    boost::shared_ptr<CubifyOperator> cubifyOp = OLAPManager::getInstance()->getCubifyOpByQueryID(0); // Assuming queryID = 0 temporarily
    boost::shared_ptr<MaterializedLattice> mLattice = cubifyOp->getAssociatedMaterializedLattice();

    //std::map<std::vector<DimensionKey>, std::map<Timestamp, double > > keyTimeValueMap;
    //std::map<std::vector<DimensionKey>, std::map<Timestamp, double > >::iterator keyTimeValueMapIt;
    boost::unordered_map<std::vector<DimensionKey>, std::map<Timestamp, double > > keyTimeValueMap;
    boost::unordered_map<std::vector<DimensionKey>, std::map<Timestamp, double > >::iterator keyTimeValueMapIt;

    std::vector<DimensionKeyName> queryDimensions;

    std::string httpRequestSubstr = getCoreHttpRequestStr(httpRequestStr);

    Document httpRequestDoc = fromjson(httpRequestSubstr);
    int nodeID = httpRequestDoc.getField("nodeID").Int();
    std::string aggrOp = httpRequestDoc.getField("aggrOp").valuestr();
    std::string timeGrain = httpRequestDoc.getField("timeGrain").valuestr();
    //std::cout << nodeID << ", " << aggrOp << ", " << timeGrain << std::endl;
    //std::cout << "Queried Node " << nodeID << std::endl;
    mLattice->getLatticeNodeByID(nodeID, queryDimensions);

    TimeCounterUtility::getInstance()->reset();
    TimeCounterUtility::getInstance()->start();

    mLattice->getAllAggregatedValuesForANodeWRelation(queryDimensions, aggrOp, timeGrain, keyTimeValueMap);
    //mLattice->getAllAggregatedValuesForANode(queryDimensions, aggrOp, timeGrain, keyTimeValueMap);

    double queryAnsweringTime;
    TimeCounterUtility::getInstance()->stop(queryAnsweringTime);
    //std::cout << "queryAnsweringTime " << queryAnsweringTime << std::endl;

    Timestamp resultTimestamp = cubifyOp->getLatestElementTimestamp();

    OLAPQueryResultJSONStr << "{\"resultTimestamp\":" << resultTimestamp << ", \"queriedNode\":" << nodeID << ", \"queryAnsweringNode\":" << mLattice->getQueryAnsweringNodeID() << ", \"queryAnsweringTime\":" << queryAnsweringTime << ", \"queryResult\":[";

    // looping through outer map
    for(keyTimeValueMapIt = keyTimeValueMap.begin(); keyTimeValueMapIt != keyTimeValueMap.end(); keyTimeValueMapIt++)
    {
        std::map<Timestamp, double> timeValueInnerMap = keyTimeValueMapIt->second;
        std::map<Timestamp, double>::iterator timeValueInnerMapIt;

        // looping through inner map
        for(timeValueInnerMapIt = timeValueInnerMap.begin(); timeValueInnerMapIt != timeValueInnerMap.end(); timeValueInnerMapIt++)
        {
            OLAPQueryResultJSONStr << "{";

            for(int i = 0; i < queryDimensions.size(); i++)
            {
                std::string dimValueStr = mLattice->getDimValue(queryDimensions[i], keyTimeValueMapIt->first[i]);

                //OLAPQueryResultJSONStr << "\"" << queryDimensions[i] << "\":\"" << keyValuesIt->first[i] << "\", ";
                OLAPQueryResultJSONStr << "\"" << queryDimensions[i] << "\":\"" << dimValueStr << "\", ";
                //std::cout << OLAPQueryResultJSONStr.str() << std::endl;
            }

            //std::cout << std::distance(keyTimeValueMap.begin(), keyTimeValueMapIt) << ", " << keyTimeValueMap.size() << ", " << std::distance(timeValueInnerMap.begin(), timeValueInnerMapIt) << ", " << timeValueInnerMap.size() << std::endl;
            if( (std::distance(keyTimeValueMap.begin(), keyTimeValueMapIt) == keyTimeValueMap.size() -1)  && (std::distance(timeValueInnerMap.begin(), timeValueInnerMapIt) == timeValueInnerMap.size() -1) )
                OLAPQueryResultJSONStr << "\"timestamp\":" << timeValueInnerMapIt->first << ", \"value\":" << timeValueInnerMapIt->second << "}";
            else
                OLAPQueryResultJSONStr << "\"timestamp\":" << timeValueInnerMapIt->first << ", \"value\":" << timeValueInnerMapIt->second << "},";
        }
    }

    OLAPQueryResultJSONStr << "]}";

    std::cout << OLAPQueryResultJSONStr.str() << std::endl;

    return OLAPQueryResultJSONStr.str();
}


std::string CommandManager::processOLAPDrilldown(std::string httpRequestStr)
{
    std::stringstream OLAPDrilldownNodesJSONStr;

    boost::shared_ptr<CubifyOperator> cubifyOp = OLAPManager::getInstance()->getCubifyOpByQueryID(0); // Assuming queryID = 0 temporarily
    boost::shared_ptr<MaterializedLattice> mLattice = cubifyOp->getAssociatedMaterializedLattice();

    std::map<int, std::vector<DimensionKeyName> > IDNodeMap;
    std::map<int, std::vector<DimensionKeyName> >::iterator IDNodeMapIt;
    std::vector<DimensionKeyName> queryDimensions;

    std::string httpRequestSubstr = getCoreHttpRequestStr(httpRequestStr);

    Document httpRequestDoc = fromjson(httpRequestSubstr);
    int nodeID = httpRequestDoc.getField("nodeID").Int();

    mLattice->getLatticeNodeByID(nodeID, queryDimensions);
    mLattice->getDrillDownNodes(queryDimensions, IDNodeMap);

    OLAPDrilldownNodesJSONStr << "{\"latticeNodes\":[";

    for(IDNodeMapIt = IDNodeMap.begin(); IDNodeMapIt != IDNodeMap.end(); IDNodeMapIt++)
    {
        std::stringstream OLAPLatticeDims;
        for(int i = 0; i < IDNodeMapIt->second.size(); i++)
        {
            if(i < IDNodeMapIt->second.size() - 1)
                OLAPLatticeDims << IDNodeMapIt->second[i] << ", ";
            else
                OLAPLatticeDims << IDNodeMapIt->second[i];
        }

        if( std::distance(IDNodeMap.begin(), IDNodeMapIt) ==  (IDNodeMap.size() - 1) )
            OLAPDrilldownNodesJSONStr << "{\"nodeID\":\"" << IDNodeMapIt->first << "\", \"nodeAttributes\":\"" << OLAPLatticeDims.str() << "\"}";
        else
            OLAPDrilldownNodesJSONStr << "{\"nodeID\":\"" << IDNodeMapIt->first << "\", \"nodeAttributes\":\"" << OLAPLatticeDims.str() << "\"},";
    }

    OLAPDrilldownNodesJSONStr << "]}";
    return OLAPDrilldownNodesJSONStr.str();
}

std::string CommandManager::processOLAPRollup(std::string httpRequestStr)
{
    std::stringstream OLAPRollupNodesJSONStr;

    boost::shared_ptr<CubifyOperator> cubifyOp = OLAPManager::getInstance()->getCubifyOpByQueryID(0); // Assuming queryID = 0 temporarily
    boost::shared_ptr<MaterializedLattice> mLattice = cubifyOp->getAssociatedMaterializedLattice();

    std::map<int, std::vector<DimensionKeyName> > IDNodeMap;
    std::map<int, std::vector<DimensionKeyName> >::iterator IDNodeMapIt;
    std::vector<DimensionKeyName> queryDimensions;

    std::string httpRequestSubstr = getCoreHttpRequestStr(httpRequestStr);

    Document httpRequestDoc = fromjson(httpRequestSubstr);
    int nodeID = httpRequestDoc.getField("nodeID").Int();

    mLattice->getLatticeNodeByID(nodeID, queryDimensions);
    mLattice->getRollUpNodes(queryDimensions, IDNodeMap);

    OLAPRollupNodesJSONStr << "{\"latticeNodes\":[";

    for(IDNodeMapIt = IDNodeMap.begin(); IDNodeMapIt != IDNodeMap.end(); IDNodeMapIt++)
    {
        std::stringstream OLAPLatticeDims;
        for(int i = 0; i < IDNodeMapIt->second.size(); i++)
        {
            if(i < IDNodeMapIt->second.size() - 1)
                OLAPLatticeDims << IDNodeMapIt->second[i] << ", ";
            else
                OLAPLatticeDims << IDNodeMapIt->second[i];
        }

        if( std::distance(IDNodeMap.begin(), IDNodeMapIt) ==  (IDNodeMap.size() - 1) )
            OLAPRollupNodesJSONStr << "{\"nodeID\":\"" << IDNodeMapIt->first << "\", \"nodeAttributes\":\"" << OLAPLatticeDims.str() << "\"}";
        else
            OLAPRollupNodesJSONStr << "{\"nodeID\":\"" << IDNodeMapIt->first << "\", \"nodeAttributes\":\"" << OLAPLatticeDims.str() << "\"},";
    }

    OLAPRollupNodesJSONStr << "]}";
    return OLAPRollupNodesJSONStr.str();
}

//the call back function called by IO manager, whenever new commands receive, this function would be called
//some kinds of command:
//1.register wrapper
//2.register query
bool CommandManager::processCommand(char* command,size_t length)
{
    //std::cout << "command manager" << std::endl;
    //std::cout << command << std::endl;

	/*
	std::string httpRequestStr(command);

	// If HTTP POST request
	if(httpRequestStr.find(HTTP_POST_BOUNDARY) != std::string::npos)
	{
        HttpParser::getInstance()->parsePostRequest(httpRequestStr, HTTP_POST_BOUNDARY);
        commandType = HttpParser::getInstance()->getPostRequestMapValue("HttpCommand");

        if(commandType == REGISTER_QUERY)
        {
            processHttpQuery(HttpParser::getInstance()->getPostRequestMapValue("query"));
        }

        return true;
	}

	// If HTTP GET request
	if(httpRequestStr.find(SHUTDOWN) != std::string::npos && httpRequestStr.find("HTTP") != std::string::npos)
	{
        JsonStreamServer::getInstance()->setShutdownJsSpinner(true); // True causes the system to shutdown after displaying summary
        return true;
	}
	*/

	std::string commandType;

    Document commandDocument(command);
	commandType = commandDocument.getField(COMMAND_TYPE).valuestr();

	if(commandType == REGISTER_WRAPPER)
	{
		processRegisterWrapperCommand(commandDocument);
	}
	else if(commandType == REGISTER_QUERY)
	{
		std::cout << "Command being processed by CommandManager::processCommand REGISTER_QUERY" << std::endl;
		processRegisterQueryCommand(commandDocument);
	}
	else if(commandType == REGISTER_QUERY_INTERMEDIATE)
	{
        std::cout << "Command being processed by CommandManager::processCommand REGISTER_QUERY_INTERMEDIATE" << std::endl;

        //sleep few seconds to let dispatcher populate input streams of executors
        //sleep(5);
        MyStatus::getInstance()->setTotalActiveWorkers(commandDocument.getField("total_active_workers").valuestr());
        MyStatus::getInstance()->setWorkerID(commandDocument.getField("worker_id").valuestr());
        MyStatus::getInstance()->setQueryID(commandDocument.getField("query_id").valuestr());
        MyStatus::getInstance()->setActiveWorkerSeqNum(commandDocument.getField("active_worker_seq_num").valuestr());

        //totalActiveWorkers = commandDocument.getField("number_active_workers").valuestr();
        //currentWorkerID = commandDocument.getField("worker_id").valuestr();
        //std::cout << "numActiveWorkers: " << numActiveWorkers << " workerID: " << workerID << std::endl;
        Document commandDoc = commandDocument.getObjectField("command_document");
        //std::cout << commandDoc.getField(QUERY_CONTENT).valuestr() << std::endl;
        processRegisterIntermediateQueryCommand(commandDoc);
	}
	else if(commandType == DESIGNATE_EXECUTOR)
	{
        std::cout << "Command being processed by CommandManager::processCommand DESIGNATE_EXECUTOR" << std::endl;

        MyStatus::getInstance()->setTotalActiveWorkers(commandDocument.getField("total_active_workers").valuestr());
        MyStatus::getInstance()->setWorkerID(commandDocument.getField("worker_id").valuestr());
        MyStatus::getInstance()->setQueryID(commandDocument.getField("query_id").valuestr());
        MyStatus::getInstance()->setActiveWorkerSeqNum(commandDocument.getField("active_worker_seq_num").valuestr());

        Document commandDoc = commandDocument.getObjectField("command_document");
        processRegisterIntermediateExecutorQueryCommand(commandDoc);

	}
	else if(commandType == WORKER_STATUS)
	{
			//std::cout << "Command manager worker status executed " << std::endl;
	        processWorkerStatusCommand(commandDocument);
	}
	else if(commandType == DESIGNATE_DISPATCHER)
	{
			std::cout << "Dispatcher designated" << std::endl;
	        processDesignateDispatcherCommand(commandDocument);
	}
	else
	{
		assert(false); // never reached
		return false;
	}
	return true;
}

bool CommandManager::shutdownJsSpinner()
{
    #ifdef TOKYO_PEOPLE_FLOW_DEMO

    namespace fs = boost::filesystem;

    fs::path p("/home/jsspinner/Dropbox/public/peopleMoveByTime/output-csv");
    if(fs::exists(p) && fs::is_directory(p))
    {
        fs::directory_iterator end;
        for(fs::directory_iterator it(p); it != end; ++it)
        {
            try
            {
                if(fs::is_regular_file(it->status()) && (it->path().extension() == ".csv" || it->path().extension() == ".tmp"))
                {
                    fs::remove(it->path());
                }
            }
            catch(const std::exception &ex)
            {
                ex;
            }
        }
    }

    #endif // TOKYO_PEOPLE_FLOW_DEMO
    JsonStreamServer::getInstance()->setShutdownJsSpinner(true); // True causes the system to shutdown after displaying summary
}

// salman
bool CommandManager::processHttpQuery(std::string jaqlQueryString)
{
    // Testing Query instead of jaqlQueryString
    /*
    // Query on system generated data
    jaqlQueryString =
	"stream1 = readFromWrapper (\"performanceTestStream2\", true) ;\
	tmp1 = stream1 -> window[range 1 seconds] ;\
	tmp1 -> istream;";

	// Query on socket stream data "peopleFlow"
	jaqlQueryString =
	"stream1 = readFromWrapper (\"socket_stream1\", true) ;\
	tmp1 = stream1 -> window[range 1 seconds] ;\
	tmp1 -> istream;";

	jaqlQueryString =
	"stream1 = readFromWrapper (\"optimistiCQ\", true) ;\
	tmp1 = stream1 -> window[range 1 seconds] -> filter $.tuple_id == 1;\
	tmp1 -> istream;";
    */

    std::cout << "jaqlQueryString from CommandManager " << jaqlQueryString << std::endl;
	boost::shared_ptr<QueryIntermediateRepresentation> intermediateQuery = ParserManager::getInstance()->processQuery(jaqlQueryString);
	std::string queryDuration = "10"; // in seconds

    //Socket Output problem need to be checked
    //boost::shared_ptr<IStreamOutput> streamOutput(new SocketStreamOutput(ip,port));
    //boost::shared_ptr<IStreamOutput> streamOutput(new ShowResultStreamOutput());

    // For TOKYO_PEOPLE_FLOW_DEMO, write output to some csv file
    boost::shared_ptr<IStreamOutput> streamOutput(new csvOutput("/home/jsspinner/Dropbox/public/peopleMoveByTime/output-csv/"));

    //Subitting the query
    JsonStreamServer::getInstance()->registerQuery(intermediateQuery, streamOutput, queryDuration);

	return true;
}

bool CommandManager::processHttpIntermediateQuery(std::string httpRequestStr)
{
    std::string httpRequestSubstr = getCoreHttpRequestStr(httpRequestStr);
    //std::cout << "httpRequestSubstr " << httpRequestSubstr << std::endl;

	std::string queryDuration = "10"; // in seconds
    boost::shared_ptr<IStreamOutput> streamOutput(new ShowResultStreamOutput());
    //std::cout << "jaqlQueryString from CommandManager " << intermediateQueryStr << std::endl;

    Document queryIntermediateDocument = fromjson(httpRequestSubstr);
    boost::shared_ptr<QueryIntermediateRepresentation> intermediateQuery(new QueryIntermediateRepresentation(queryIntermediateDocument));

    //Subitting the query
    JsonStreamServer::getInstance()->registerQuery(intermediateQuery, streamOutput, queryDuration);
	return true;
}
// ~salman


bool CommandManager::processDesignateDispatcherCommand(Document& commandDocument)
{
	//JsonStreamServer::getInstance()->isDispatcher = true;

	std::string workerIPIDPairsStr = commandDocument.getField("worker_ip_id_pairs").valuestr();

	MyStatus::getInstance()->setTotalActiveWorkers(commandDocument.getField("total_active_workers").valuestr());
    MyStatus::getInstance()->setWorkerID(commandDocument.getField("worker_id").valuestr());
    MyStatus::getInstance()->setQueryID(commandDocument.getField("query_id").valuestr());
    MyStatus::getInstance()->setActiveWorkerSeqNum(commandDocument.getField("active_worker_seq_num").valuestr());

	std::size_t pos1 = 0;
	std::size_t pos2 = 0;
	std::size_t pos3 = 0;
	std::string IPStr = "";
	std::string IDStr = "";
	int pos1InitialVal = -1;

	// Creating a map of workers(executors) IPs and their IDs using the values in workerIPIDStr
	std::map<std::string,int> workerIPIDMap;
	while(1)
	{
	  pos2 = workerIPIDPairsStr.find(",", pos1+1);
	  pos3 = workerIPIDPairsStr.find(",", pos2+1);

	   if(pos2 == std::string::npos || pos3 == std::string::npos)
		break;

	   IPStr = workerIPIDPairsStr.substr(pos1 + 1 + pos1InitialVal, pos2 - pos1 - 1 - pos1InitialVal);
	   pos1InitialVal = 0;
	   IDStr = workerIPIDPairsStr.substr(pos2 + 1, pos3 - pos2 -1);

	   pos1 = pos3;

 	   //std::cout << "IPStr " << IPStr << std::endl;
	   //std::cout << "IDStr " << IDStr << std::endl;

	   workerIPIDMap.insert(make_pair(IPStr, boost::lexical_cast<int>(IDStr)));

	}

	// Stream data reading queries are stored in dispatcherQIRStr separated by "<>" symbol
	std::string dispatcherQIRStr = commandDocument.getField("dispatcher_QIR").valuestr();
	pos1 = 0;
	pos2 = 0;
	std::string dispatcherQueryStr;
	std::vector<std::string> dispatcherQueryVec;
	while(1)
	{
		// Queries are separated by "<>" symbol
		pos2 = dispatcherQIRStr.find("<>", pos1 + 1);

		if(pos2 == std::string::npos)
			break;

		dispatcherQueryStr = dispatcherQIRStr.substr(pos1, pos2 - pos1);
		pos1 = pos2 + 2;

		//std::cout << "dispatcherQueryStr " << dispatcherQueryStr << std::endl;
		dispatcherQueryVec.push_back(dispatcherQueryStr);
	}

	//std::string queryIntermediateString = commandDocument.getField(QUERY_CONTENT).valuestr();

	//Parse the queryIntermediateString to obtain the sub-queries (i.e, the queries containing the leaf and the root operators)
	//The vector dispatcherQIRVector contains all the queries responsible for reading the data streams
	//DispatchStream dispatcher;
 	//dispatcherQIRVector = dispatcher.getDispatcherQIR(queryIntermediateString);
	DispatcherManager::getInstance()->execute(workerIPIDMap, dispatcherQueryVec);
	//std::cout << "from CommandManager " << dispatcherQIR << std::endl;

	//queryIntermediateString = "{"type":"root","input":{"type":"leaf","stream_source":"stream1","is_master":true}}";
	//{"type":"root","input":{"type":"leaf","stream_source":"stream1","is_master":true}
	//queryIntermediateString = "{\"type\":\"root\",\"input\":{\"type\":\"leaf\", \"stream_source\":\"stream1\",\"is_master\":true}}";

	//Document queryIntermediateDocument = fromjson(queryIntermediateString);
    	//boost::shared_ptr<QueryIntermediateRepresentation> intermediateQuery(new QueryIntermediateRepresentation(queryIntermediateDocument));

	//std::cout << workerIPIDMap.size() << std::endl;
	//std::string queryIntermediateString = commandDocument.getField(QUERY_CONTENT).valuestr();
	//std::cout << "from command manager " << queryIstd::map<std::string,int> workerIPIDMapntermediateString << std::endl;
	//Document queryIntermediateDocument = fromjson(queryIntermediateString);
	//boost::shared_ptr<QueryIntermediateRepresentation> intermediateQuery(new QueryIntermediateRepresentation(queryIntermediateDocument));

	//Document outputSpecificationDocument = commandDocument.getField(OUTPUT_SPECIFICATION).embeddedObject();

	/*
	std::map<std::string,int>::iterator workerIPIDMapIt;
	for(workerIPIDMapIt = workerIPIDMap.begin(), workerIPIDMapIt != workerIPIDMap.end(), workerIPIDMapIt++)
	{
		Document queryIntermediateDocument = fromjson(queryIntermediateString);
	        boost::shared_ptr<QueryIntermediateRepresentation> intermediateQuery(new QueryIntermediateRepresentation(queryIntermediateDocument));

		std::string ip = *workerIPIDMapIt;
		std::string port = ConfigureManager::getInstance()->getConfigureValue("worker_data_port");
		//std::cout << ip << " " << port << std::endl;
		boost::shared_ptr<IStreamOutput> streamOutput(new SocketStreamOutput(ip,port));
		//JsonStreamServer::getInstance()->registerQuery(intermediateQuery, streamOutput);
		JsonStreamServer::getInstance()->registerQuery(intermediateQuery, streamOutput);
	}

	*/

	return true;
}

bool CommandManager::processWorkerStatusCommand(Document& commandDocument)
{
    //std::string commandType = commandDocument.getField(COMMAND_TYPE).valuestr();
    //std::cout << "Command Type " << commandType << std::endl;
    std::string workerIP = MyStatus::getInstance()->getMyIP();
    std::string server_ip = commandDocument.getField(SENDER_IP).valuestr();
    std::string server_port = commandDocument.getField(SENDER_PORT).valuestr();

    DocumentBuilder workerStatusCommandbuilder;
    workerStatusCommandbuilder.append("command_type","worker_status");
    workerStatusCommandbuilder.append("command_content","worker_alive");
    workerStatusCommandbuilder.append("worker_ip", workerIP);
    workerStatusCommandbuilder.append("worker_command_port", ConfigureManager::getInstance()->getConfigureValue("worker_command_port"));

    Document workerStatusCommandDocument = workerStatusCommandbuilder.obj();

    IOManager::getInstance()->sendCommand(server_ip, ConfigureManager::getInstance()->getConfigureValue("worker_status_port"), workerStatusCommandDocument);

    return true;
}

bool CommandManager::processRegisterWrapperCommand(Document& document)
{
	Document wrapperDocument = document.getField(WRAPPER_CONTENT).embeddedObject();
	WrapperManager::getInstance()->registerWrapper(wrapperDocument);
	return true;
}

bool CommandManager::processRegisterQueryCommand(Document& document)
{
	std::string jaqlQueryString = document.getField(QUERY_CONTENT).valuestr();
	boost::shared_ptr<QueryIntermediateRepresentation> intermediateQuery = ParserManager::getInstance()->processQuery(jaqlQueryString);
	std::cout<<"Jaql query accepted : " << jaqlQueryString<<std::endl;
	Document outputSpecificationDocument = document.getField(OUTPUT_SPECIFICATION).embeddedObject();
	std::string outputType =  outputSpecificationDocument.getField(OUTPUT_TYPE).valuestr();
	std::string queryDuration = outputSpecificationDocument.getField(QUERY_DURATION).valuestr();

	if(outputType == SHOW_RESULT)
	{
		boost::shared_ptr<IStreamOutput> streamOutput(new ShowResultStreamOutput());
		JsonStreamServer::getInstance()->registerQuery(intermediateQuery, streamOutput, queryDuration);
	}
	else if(outputType == SOCKET_OUTPUT)
	{
		std::string ip = outputSpecificationDocument.getField(OUTPUT_IP).valuestr();
		std::string port = outputSpecificationDocument.getField(OUTPUT_PORT).valuestr();
		boost::shared_ptr<IStreamOutput> streamOutput(new SocketStreamOutput(ip,port));

		JsonStreamServer::getInstance()->registerQuery(intermediateQuery, streamOutput, queryDuration);
	}
	else
	{
		assert(false);//never reached
		return false;
	}
	return true;
}

bool CommandManager::processRegisterIntermediateQueryCommand(Document& document)
{
    std::string queryIntermediateString = document.getField(QUERY_CONTENT).valuestr();
    Document queryIntermediateDocument = fromjson(queryIntermediateString);
    boost::shared_ptr<QueryIntermediateRepresentation> intermediateQuery(new QueryIntermediateRepresentation(queryIntermediateDocument));
    int queryID;

    //std::cout<<"Query Intermediate String : " << queryIntermediateString <<std::endl;

	Document outputSpecificationDocument = document.getField(OUTPUT_SPECIFICATION).embeddedObject();
	std::string outputType =  outputSpecificationDocument.getField(OUTPUT_TYPE).valuestr();
	std::string queryDuration = outputSpecificationDocument.getField(QUERY_DURATION).valuestr();
	if(outputType == SHOW_RESULT)
	{
		boost::shared_ptr<IStreamOutput> streamOutput(new ShowResultStreamOutput());
		queryID = JsonStreamServer::getInstance()->registerQuery(intermediateQuery, streamOutput, queryDuration);
	}
	else if(outputType == SOCKET_OUTPUT)
	{
		std::string ip = outputSpecificationDocument.getField(OUTPUT_IP).valuestr();
		std::string port = outputSpecificationDocument.getField(OUTPUT_PORT).valuestr();
		boost::shared_ptr<IStreamOutput> streamOutput(new SocketStreamOutput(ip,port));

		queryID = JsonStreamServer::getInstance()->registerQuery(intermediateQuery, streamOutput, queryDuration);
	}
	else
	{
		assert(false);//never reached
		return false;
	}
	return true;
}

bool CommandManager::processRegisterIntermediateExecutorQueryCommand(Document& document)
{
    std::string queryIntermediateString = document.getField(QUERY_CONTENT).valuestr();
    Document queryIntermediateDocument = fromjson(queryIntermediateString);
    boost::shared_ptr<QueryIntermediateRepresentation> intermediateQuery(new QueryIntermediateRepresentation(queryIntermediateDocument));
    int queryID;
    //std::cout<<"Query Intermediate String : " << queryIntermediateString <<std::endl;

	Document outputSpecificationDocument = document.getField(OUTPUT_SPECIFICATION).embeddedObject();
	std::string outputType =  outputSpecificationDocument.getField(OUTPUT_TYPE).valuestr();
	std::string queryDuration = outputSpecificationDocument.getField(QUERY_DURATION).valuestr();
	if(outputType == SHOW_RESULT)
	{
		boost::shared_ptr<IStreamOutput> streamOutput(new ShowResultStreamOutput());
		queryID = JsonStreamServer::getInstance()->registerExecutorQuery(intermediateQuery, streamOutput, queryDuration);
	}
	else if(outputType == SOCKET_OUTPUT)
	{
		std::string ip = outputSpecificationDocument.getField(OUTPUT_IP).valuestr();
		std::string port = outputSpecificationDocument.getField(OUTPUT_PORT).valuestr();
		boost::shared_ptr<IStreamOutput> streamOutput(new SocketStreamOutput(ip,port));

		queryID = JsonStreamServer::getInstance()->registerExecutorQuery(intermediateQuery, streamOutput, queryDuration);
	}
	else
	{
		assert(false);//never reached
		return false;
	}
	return true;
}

bool CommandManager::processExperimentalQueryCommand(int queryID, std::string outputFile, double &avgQueryingCost, long int &storageCostBytes)
{
    ofstream OutputFile;
    OutputFile.open(outputFile.c_str(), std::ofstream::out | std::ofstream::app);

    /*
    OutputFile << "*********************** StreamingCubeExperiments ***********************\n";
    OutputFile << "Materialization Scheme \nRef. Freq. Allocation Method \nNum. of Materialized Vertices (Incl. finest vertex) \nMax. Storage (Num of Tuples) \n\n";
    OutputFile << OLAPManager::getInstance()->getConfigValue(queryID, "MMode") << "\n";
    OutputFile << OLAPManager::getInstance()->getConfigValue(queryID, "RFMode") << "\n";
    OutputFile << OLAPManager::getInstance()->getNumOfMaterializedVertices(queryID) << "\n";
    OutputFile << OLAPManager::getInstance()->getConfigValue(queryID, "MaxStorageNumTuples") << "\n";
    */

    boost::shared_ptr<CubifyOperator> cubifyOp = OLAPManager::getInstance()->getCubifyOpByQueryID(queryID); // Assuming queryID = 0 temporarily
    boost::shared_ptr<MaterializedLattice> mLattice = cubifyOp->getAssociatedMaterializedLattice();

    std::string aggrOp = "sum";
    std::string timeGrain = "second";
    std::vector<double> queryAnsTimeAllQueriesVec;

    //for(queryDimensionsIt = queryDimensions.begin(); queryDimensionsIt != queryDimensions.end(); queryDimensionsIt++)
        //std::cout << *queryDimensionsIt << " | ";

    std::map<vertexInfo, std::vector<std::string> > latticeVertices;
    std::map<vertexInfo, std::vector<std::string> >::iterator latticeVerticesIt;
    latticeVertices = OLAPManager::getInstance()->getLatticeVertices(queryID);

    std::vector<std::string> refFrequencies;
    OLAPManager::getInstance()->getConfigValue(queryID, "RefFrequencies", refFrequencies);

    int totalFrequency = 0;
    double queryAnsweringTimeAllQueries = 0;
    long int storageCost = 0;
    double totalQueryingCost = 0;

    int i = 0;
    int nodeSize = 0;
    for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++, i++)
    {
        // Get query dimensions by querying node ID
        std::vector<DimensionKeyName> queryDimensions;
        //std::map<std::vector<DimensionKey>, std::map<Timestamp, double > > keyTimeValueMap; // Map to collect the result
        boost::unordered_map<std::vector<DimensionKey>, std::map<Timestamp, double > > keyTimeValueMap; // Map to collect the result

        mLattice->getLatticeNodeByID(latticeVerticesIt->first.vertexID, queryDimensions);

        TimeCounterUtility::getInstance()->reset();
        TimeCounterUtility::getInstance()->start();

        nodeSize = mLattice->getAllAggregatedValuesForANodeWRelation(queryDimensions, aggrOp, timeGrain, keyTimeValueMap);
        //mLattice->getAllAggregatedValuesForANode(queryDimensions, aggrOp, timeGrain, keyTimeValueMap);
        if(nodeSize > 0)
            storageCost += (nodeSize * (queryDimensions.size() * sizeof(int) + sizeof(int) ) ); // size of keys and a value in bytes

        double queryAnsweringTime = 0;
        TimeCounterUtility::getInstance()->stop(queryAnsweringTime);
        //std::cout << queryAnsweringTime << std::endl;
        queryAnsweringTimeAllQueries += ( queryAnsweringTime* atoi(refFrequencies[i].c_str()) );
        totalFrequency += atoi(refFrequencies[i].c_str());
        //totalQueryingCost += queryAnsweringTime;
        //std::cout << "refFrequencies[i].c_str() " << refFrequencies[i].c_str() << std::endl;
        //std::cout << "Querying Node | Querying Time: " << latticeVerticesIt->first.vertexID << " | " << queryAnsweringTime* atoi(refFrequencies[i].c_str()) << std::endl;

        //OutputFile << "Queried Node" << "\n";
        //OutputFile << "Query Answering Node" << "\n";
        //OutputFile << "Query Answering Time" << "\n";
        //OutputFile << latticeVerticesIt->first.vertexID << "\n";
        //OutputFile << mLattice->getQueryAnsweringNodeID() << "\n";
        //OutputFile << queryAnsweringTime << "\n";
        //OutputFile << "**********************************************\n";

    }
    //OutputFile << j << " QueryAnsweringTimeAllQueries :" << queryAnsweringTimeAllQueries << "\n";
    //std::cout << j << " QueryAnsweringTimeAllQueries :" << queryAnsweringTimeAllQueries << std::endl;
    //queryAnsTimeAllQueriesVec.push_back(queryAnsweringTimeAllQueries);


    //double avg = AvgStdDeviation::getInstance()->getAverage(queryAnsTimeAllQueriesVec);
    //double stdDeviation = AvgStdDeviation::getInstance()->getStdDeviation(queryAnsTimeAllQueriesVec);

    //OutputFile << "\navg \nstdDev\n\n" << avg << "\n" << stdDeviation << "\n\n";
    //std::cout << "Avg: " << avg << " | Std.Deviation: " << stdDeviation << std::endl;
    //std::cout << "avg | stdDev " << avg << " | " << stdDeviation << std::endl;
    OutputFile.close();
    storageCostBytes = storageCost;
    avgQueryingCost = queryAnsweringTimeAllQueries/totalFrequency;

    //std::cout << queryAnsweringTimeAllQueries << ", totalFrequency " << totalFrequency << ", avgQueryingCost " <<  avgQueryingCost << std::endl;
    return true;
}
