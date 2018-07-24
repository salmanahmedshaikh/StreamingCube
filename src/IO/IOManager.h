//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include <boost/noncopyable.hpp>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "../IO/SocketStreamInput.h"
#include "../IO/SocketStreamOutput.h"
#include "../Command/CommandManager.h"
#include "../Dispatcher/DispatcherManager.h"
#include "../IO/DispatcherStreamInput.h"

//const static std::string COMMAND_TYPE = "command_type";
//const static std::string WORKER_STATUS = "worker_status";

/*
typedef struct _BUFFER
{
    char* inputBuffer;
    int inputOffset;
    std::string workerID;
    std::string queryID;
    std::string activeWorkerSeqNum;
}BUFFER;
*/

typedef struct _OUTPUT_EV_BUFFER
{
    int outputBytes;
    struct evbuffer *outputBuffer;
}OUTPUT_EV_BUFFER;

/*
 * IOManager is used to deal with socket
 */
class IOManager:private boost::noncopyable
{
private:
    std::string serverIp;
	std::string serverPort;

	static IOManager * ioManager;
	//boost::unordered_map<SocketStreamOutput*, struct evbuffer*> outputMap;//map each SocketStreamOutput to one output buffer
	boost::unordered_map<SocketStreamOutput*, OUTPUT_EV_BUFFER*> outputMap;//map each SocketStreamOutput to one output buffer
	struct event_base * base;

	static void errorcb(struct bufferevent *bev, short error, void *ctx);

	//Dispatcher stream
	static void dispatcherStream_accept(evutil_socket_t listener, short event, void *arg);
	static void dispatcherStream_read(struct bufferevent *bev, void *ctx);

	//stream input
	static void input_accept(evutil_socket_t listener, short event, void *arg);
	static void input_read(struct bufferevent *bev, void *ctx);

	//stream output
	static void output_connected(struct bufferevent *bev, short events, void *ptr);

	//command input
	static void command_read(struct bufferevent *bev, void *ctx);
	static void command_accept(evutil_socket_t listener, short event, void *arg);

	static void http_accept(struct evhttp_request * req, void * arg);
	//static void http_accept(evutil_socket_t listener, short event, void *arg);
	//the thread function. loop and check if some event is ready, if yes,  the registered function handler would be called
	void createNewThreadToPerfromIO(void);

public:
	IOManager(void);
	~IOManager(void);

    static int socketBytesSent;
	static int socketBytesReceived;

	static IOManager* getInstance(void);

	//void processElement(Element& element);
	int getWorkerCount(void);
	// Send data to worker nodes
	//bool sendCommand(std::string ip,std::string port, std::string command);
    bool sendCommand(std::string ip,std::string port, Document&  commandDocument);
	//receive the input stream by socket
	void addStreamInput(std::string ip,std::string port, SocketStreamInput* streamInput);

	//export the output stream by socket
	bool addStreamOutput(std::string ip,std::string port, SocketStreamOutput* streamOutput);

	// Get data stream from dispatcher
	void getDispatcherStream(std::string port, DispatcherManager* dispatcherManager);

	//write output to the socket
	bool writeOutput(char* buffer, int length,SocketStreamOutput* streamOutput);

	//receive command by socket
	void addCommandInput(std::string ip,std::string port, CommandManager* commandManager);
	void addHttpCommandInput(std::string ip,std::string port, CommandManager* commandManager);

	bool insertIntoDispatcherStreamSourceMap(std::string streamSource, struct evbuffer* input);

    // get streamInput using the stream source (stream ID)
    boost::shared_ptr<IStreamInput> getStreamInput(std::string streamSource);
    //DispatcherStreamInput* getStreamInput(std::string streamSource);
	//bool addParserServer(std::string ip,std::string port);

	////connect parser server
	//std::string writeJaqlQuery(std::string jaqlQueryString);

	//create a new thread to deal with the IO
	void execute(void);



};

