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
#include <cstdlib>
#include <evhttp.h>
#include <boost/noncopyable.hpp>
#include <map>

static const std::string SERVER_IP = "127.0.0.1";  // server ip
static const std::string SERVER_PORT = "8088";     // command port
//register wrapper command example
//{
//	"command_type":"register_wrapper",
//	"wrapper_content":{...}
//}
//wrapper_content is explained by wrapper manager class



//register query command example
//{
// "command_type":"register_query",
// "query_content":"jaql query string",
// "output_specification":{"output_type":"show_result"},
// "query_duration":"10s"
//}
//{
// "command_type":"register_query",
// "query_content":"jaql query string",
// "output_specification":{"output_type":"socket_output",
//                         "output_ip":"localhost",
//                         "output_port":"8080"
//                        },
// "query_duration":"20min"
//}
//output_specification may be one of show_result,socket_output
//query_content is explained by ParserManager class


const static std::string COMMAND_TYPE = "command_type";
const static std::string REGISTER_QUERY = "register_query";
const static std::string REGISTER_QUERY_INTERMEDIATE = "register_query_intermediate";
const static std::string REGISTER_WRAPPER = "register_wrapper";
const static std::string WRAPPER_CONTENT = "wrapper_content";
const static std::string RANDOM_GENERATED = "random_generated";
const static std::string SOCKET_INPUT = "socket_input";
const static std::string QUERY_CONTENT = "query_content";
const static std::string OUTPUT_SPECIFICATION = "output_specification";
const static std::string OUTPUT_TYPE = "output_type";
const static std::string SHOW_RESULT = "show_result";
const static std::string SOCKET_OUTPUT = "socket_output";
const static std::string OUTPUT_IP = "output_ip";
const static std::string OUTPUT_PORT = "output_port";
const static std::string QUERY_DURATION = "query_duration";
const static std::string WORKER_STATUS = "worker_status";
const static std::string COMMAND_CONTENT = "command_content";
const static std::string SENDER_IP = "sender_ip";
const static std::string SENDER_PORT = "sender_port";
const static std::string DESIGNATE_DISPATCHER = "designate_dispatcher";
const static std::string DESIGNATE_EXECUTOR = "designate_executor";
const static std::string HTTP_POST_BOUNDARY = "-------------------------------19588288329222";
const static std::string SHUTDOWN = "/shutdown";
const static std::string SUBMIT = "/submit";

/* server listens to a port waiting for command
 * there are two kinds of command, one is registering query, one is registering wrapper
 * the command manager interacts with the IO manager to deal with socket
 */
class CommandManager:private boost::noncopyable
{
private:
	static CommandManager* commandManager;
	CommandManager(void);
	void initial(void);
	bool processRegisterWrapperCommand(Document& document);
	bool processRegisterQueryCommand(Document& document);
	bool processRegisterIntermediateQueryCommand(Document& document);
	bool processRegisterIntermediateExecutorQueryCommand(Document& document);
	bool processWorkerStatusCommand(Document& document);
	bool processDesignateDispatcherCommand(Document& document);
	bool processHttpQuery(std::string);
	bool processHttpIntermediateQuery(std::string);
	bool sendHttpResponce(struct evhttp_request * req, std::string responceText);
	bool shutdownJsSpinner();
	void generateCubifyConfigFile(std::string fileLocation, std::string fileContents);
	bool processStopQuery(std::string httpRequestStr);

    std::string processOLAPGetLattice();
    std::string processOLAPGetLatticeWithInfo();
	std::string processOLAPQuery(std::string httpRequestStr);
    std::string processOLAPQueryWTimeGrain(std::string httpRequestStr);
    std::string processOLAPDrilldown(std::string);
    std::string processOLAPRollup(std::string);

    std::string getCoreHttpRequestStr(std::string httpRequestStr);

public:
	static CommandManager* getInstance();
	~CommandManager(void);
	//the call back function called by IO manager, whenever new commands receive, this function would be called
	bool processCommand(char* command,size_t length);
	bool processHttpRequest(struct evhttp_request * req);
	bool processExperimentalQueryCommand(int queryID, std::string outputFile, double &avgQueryingCost, long int &storageCostBytes);
};

