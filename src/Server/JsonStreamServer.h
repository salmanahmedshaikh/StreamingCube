//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../Scheduler/Scheduler.h"
#include "../Operator/Operator.h"
#include "../IO/IStreamInput.h"
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "../IO/IStreamInput.h"
#include "../IO/IStreamOutput.h"
#include "../Schema/JsonSchema.h"
#include "../Query/QueryIntermediateRepresentation.h"
#include "../Command/CommandManager.h"
#include "../IO/IOManager.h"
#include "../Common/Types.h"
#include "../BinaryJson/json.h"
#include "../IO/RandomGeneratedStreamInput.h"
#include "../Server/JsonStreamServer.h"
#include "../IO/SocketStreamInput.h"
#include "../IO/SocketStreamOutput.h"
#include "../IO/IStreamOutput.h"
#include "../IO/IStreamInput.h"
#include "../IO/ShowResultStreamOutput.h"
#include "../Query/QueryManager.h"
#include "../Plan/PlanManager.h"
#include "../Scheduler/Scheduler.h"
#include "../Wrapper/WrapperManager.h"
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
/* the JSON server engine
 */

class JsonStreamServer:private boost::noncopyable
{
private:

	static JsonStreamServer* jsonStreamServer;
	int workerCount;
	bool recursiveExecuteFuncCall = false;
	bool shutdownJsSpinner = false;

	std::vector<std::string> activeWorkersIPs;
	JsonStreamServer(void);
	~JsonStreamServer(void);
	void initial();

public:
	static JsonStreamServer* getInstance(void);
	void execute();
	void setShutdownJsSpinner(bool);
	//bool isDispatcher;

	std::string getQueryIntermediateRepresentation(std::string jaqlQueryString);
	void registerQuery(std::string jaqlQueryString, boost::shared_ptr<IStreamOutput> streamOutput, std::string durationSpecification);
	int registerQuery(boost::shared_ptr<QueryIntermediateRepresentation>queryIntermediateRepresentation, boost::shared_ptr<IStreamOutput> streamOutput, std::string durationSpecification);
	int registerExecutorQuery(boost::shared_ptr<QueryIntermediateRepresentation> queryIntermediateRepresentation, boost::shared_ptr<IStreamOutput> streamOutput,	std::string durationSpecification);
	void registerQuery(boost::shared_ptr<QueryIntermediateRepresentation> queryIntermediateRepresentation, boost::shared_ptr<IStreamOutput> streamOutput,	std::string durationSpecification, bool isDispatcher);
	void registerStreamInput(boost::shared_ptr<IStreamInput> streamInput);
	void pushActiveWorkerNodeIP(std::string workerNodeIP);
};

