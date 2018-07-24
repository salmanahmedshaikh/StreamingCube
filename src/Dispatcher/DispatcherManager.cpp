//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../Common/Types.h"
#include "../Dispatcher/DispatcherManager.h"
#include "../Configure/ConfigureManager.h"
#include "../BinaryJson/json.h"
#include "../Query/QueryIntermediateRepresentation.h"
#include "../Server/JsonStreamServer.h"


DispatcherManager * DispatcherManager::dispatcherManager = NULL;
DispatcherManager::DispatcherManager(void)
{
    initialize();
}

DispatcherManager::~DispatcherManager(void)
{
}

DispatcherManager* DispatcherManager::getInstance(void)
{
	if(dispatcherManager==NULL)
	{
		dispatcherManager = new DispatcherManager();
	}
	return dispatcherManager;
}

void DispatcherManager::initialize()
{
	//use IOManager's function getDispatcherStream to listen on a port for data streams
    IOManager::getInstance()->getDispatcherStream(ConfigureManager::getInstance()->getConfigureValue("worker_data_port"), this);
}

void DispatcherManager::execute(std::map<std::string, int> workerIPIDMap, std::vector<std::string> dispatcherQueryVec)
{
    //dispatcherQueryVec contains all the queries responsible for reading the data streams (i.e., they contain only leaf and root operators)
    //Following nested loop sends each query to all the workers (executors) in the cluster
    //Outer loop, loops through all the queries and the inner loop, loops through all the workers (executors) available for execution
	//std::vector<std::string>  dispatcherQIRVector = getDispatcherQIR(queryIntermediateString);
	std::string activateDuration = ConfigureManager::getInstance()->getConfigureValue("activate_duration");
	std::vector<std::string>::iterator dispatcherQueryVecIt;

	for(dispatcherQueryVecIt=dispatcherQueryVec.begin(); dispatcherQueryVecIt != dispatcherQueryVec.end(); dispatcherQueryVecIt++)
	{
        std::string dispatcherQueryString = *dispatcherQueryVecIt;
		//std::cout << *dispatcherQueryVecIt << std::endl;

		Document queryIntermediateDocument = fromjson(*dispatcherQueryVecIt);
        boost::shared_ptr<QueryIntermediateRepresentation> intermediateQuery(new QueryIntermediateRepresentation(queryIntermediateDocument));
        //std::string streamSource = queryIntermediateDocument.getField("stream_source").valuestr();

        //*dispatcherQueryVecIt = {"type":"root","input":{"type":"leaf","stream_source":"stream1","is_master":true}}}}}
        std::size_t pos1 = dispatcherQueryString.find("stream_source", 0);
        std::size_t pos2 = dispatcherQueryString.find("\"" , pos1 + 17);

        std::string streamSource = dispatcherQueryString.substr(pos1 + 16, pos2 - (pos1 + 16));
        //std::cout << "Stream source from DispatcherManager : " << streamSource << std::endl;

		std::map<std::string,int>::iterator workerIPIDMapIt;
		for(workerIPIDMapIt = workerIPIDMap.begin(); workerIPIDMapIt != workerIPIDMap.end(); workerIPIDMapIt++)
		{
			std::string ip = workerIPIDMapIt->first;
	        std::string port = ConfigureManager::getInstance()->getConfigureValue("worker_data_port");
        	std::cout << "Sending data stream to worker: " << ip << ", on port: " << port << std::endl;

			// Preparing to send data on port 8095 to all the executors
			boost::shared_ptr<IStreamOutput> streamOutput(new SocketStreamOutput(ip,port,streamSource, true));
			//boost::shared_ptr<IStreamOutput> streamOutput(new SocketStreamOutput(ip,port,streamSource));
			//boost::shared_ptr<IStreamOutput> streamOutput(new ShowResultStreamOutput());
            //JsonStreamServer::getInstance()->registerQuery(intermediateQuery, streamOutput);
            //JsonStreamServer::getInstance()->registerQuery(intermediateQuery, streamOutput, activateDuration);
            JsonStreamServer::getInstance()->registerQuery(intermediateQuery, streamOutput, activateDuration, true);
		}

	}
}
