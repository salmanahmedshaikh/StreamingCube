//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include <event.h>
#include <evhttp.h>
#include <stdint.h>
#include <fcntl.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/thread.h>
#include <assert.h>
#include <event2/event-config.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <boost/unordered_map.hpp>
#include <boost/thread.hpp>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>
#include "../IO/IOManager.h"
#include "../IO/SocketStreamInput.h"
#include "../Internal/Buffer/BufferConstraint.h"
#include "../Utility/MyStatus.h"
#include "../Utility/HttpParser.h"
#include "../Operator/CollectStream.h"
#include "../Configure/ConfigureManager.h"
#include "../IO/DispatcherStreamInput.h"

//salman
#include <evhttp.h>

#ifdef   FD_SETSIZE
#undef   FD_SETSIZE
#endif
#define   FD_SETSIZE   2048
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "libevent.lib")
#pragma comment(lib, "libevent_core.lib")
#pragma comment(lib, "libevent_extras.lib")

int WATER = 4096;
int workerCount = 0;

int IOManager::socketBytesSent = 0;
int IOManager::socketBytesReceived = 0;

//bool isExecutor = false;

char global_input_buffer[4096];
boost::unordered_map<struct evbuffer*, BUFFER* > bufferMap;
//boost::unordered_map<struct evbuffer*, DispatcherStreamInput* > dispatcherStreamInputMap;
boost::unordered_map<struct evbuffer*, boost::shared_ptr<IStreamInput> > dispatcherStreamInputMap;
boost::unordered_map<std::string, struct evbuffer* > dispatcherStreamSourceMap;

int IOManager::getWorkerCount(void)
{
    return workerCount;
}

// Send data to worker nodes
bool IOManager::sendCommand(std::string ip,std::string port, Document&  commandDocument)
{
	int sClient;
	struct sockaddr_in server;
	char szMessage[2048];
	int ret;

	//assert(command.length()<2048);
	memcpy(szMessage, commandDocument.objdata(), commandDocument.objsize());
	//IOManager::socketBytesSent += commandDocument.objsize();

	//memcpy(szMessage, command, command.size());
	if ((sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
	{
		perror("socket creation error");
		exit(1);
	}

	int iport = atoi(port.c_str());
	memset(&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = PF_INET;
	server.sin_port = htons(iport);
	//server.sin_port = iport;
	server.sin_addr.s_addr = inet_addr( ip.c_str());

	if(connect(sClient, (struct sockaddr *) &server, sizeof(struct sockaddr_in))<0)
	{
        std::string errMsg = "Failed to connect to the server: " + ip;
        //std::string errMsg2 = "Failed to connect to the server port: " + port;

		perror(errMsg.c_str());

		if(commandDocument.getField(COMMAND_TYPE).valuestr() == WORKER_STATUS)
		{
            workerCount++;
            //std::cout << "IOManager::workerCount failed " << workerCount << std::endl;
            close(sClient);
            return false;
		}
		else
		{
            exit(1);
        }
	}

	if(send(sClient, szMessage, commandDocument.objsize(), 0)<0)
	{
        if(commandDocument.getField(COMMAND_TYPE).valuestr() == WORKER_STATUS)
        {
            workerCount++;
            //std::cout << "IOManager::workerCount failed2 " << workerCount << std::endl;
            close(sClient);
            return false;
        }
		perror("send error");
		exit(1);
	}
	close(sClient);

	return true;
}

////////////////////stream input logic//////////////
void IOManager::input_read(struct bufferevent *bev, void *ctx)
{
    //std::cout << "input_read called " << std::endl;

	SocketStreamInput* streamInput = (SocketStreamInput*)ctx;
	struct evbuffer *input;
	input = bufferevent_get_input(bev);

	size_t buffer_len = evbuffer_get_length(input);

	//std::cout<<buffer_len<<std::endl;
	if (buffer_len <= 0)
		return ;
	evbuffer_remove(input,global_input_buffer,buffer_len);

    //std::cout << "buffer_len " << buffer_len << std::endl;
	//std::cout << "global_input_buffer " << std::endl << global_input_buffer << std::endl;

	streamInput->readData(global_input_buffer, buffer_len);

}

bool IOManager::insertIntoDispatcherStreamSourceMap(std::string streamSource, struct evbuffer* input)
{
    boost::unordered_map<std::string, struct evbuffer* >::iterator dispatcherStreamSourceMapIt;

    if ((dispatcherStreamSourceMapIt = dispatcherStreamSourceMap.find(streamSource)) == dispatcherStreamSourceMap.end())
    {
        dispatcherStreamSourceMap.insert(make_pair(streamSource, input));

        return true;
    }
    return false;
}

//DispatcherStreamInput* IOManager::getStreamInput(std::string streamSource)
boost::shared_ptr<IStreamInput> IOManager::getStreamInput(std::string streamSource)
{
    boost::unordered_map<std::string, struct evbuffer* >::iterator dispatcherStreamSourceMapIt;
    dispatcherStreamSourceMapIt = dispatcherStreamSourceMap.find(streamSource);
    assert(dispatcherStreamSourceMapIt != dispatcherStreamSourceMap.end());

    boost::unordered_map<struct evbuffer*, boost::shared_ptr<IStreamInput> >::iterator dispatcherStreamInputMapIt;
    dispatcherStreamInputMapIt = dispatcherStreamInputMap.find(dispatcherStreamSourceMapIt->second);
    assert(dispatcherStreamInputMapIt != dispatcherStreamInputMap.end());

	return dispatcherStreamInputMapIt->second;
}



void IOManager::dispatcherStream_read(struct bufferevent *bev, void *ctx)
{
    // Setting the _isDispatcherStreamInput Flag true, so that the QueryIntermediateRepresentationInterpreter can obtain the stream source from dispatcher
    //SocketStreamInputgetInstance()-> setIsDispatcherStreamInput(true);
    //_isDispatcherStreamInput = true;
    //static char inputBuffer[8192];// the buffer containing the input data
    //static int inputOffset = 0;       // the length of the input data in the buffer

    //DispatcherStreamInput* streamInput = (DispatcherStreamInput*)ctx;

    //BUFFER * buffer = NULL;
    //void (*pCallBackFunction)(Element& element);
    //pCallBackFunction = (void ( *)(Element &)) callBackFunction;
    struct evbuffer *input;
    input = bufferevent_get_input(bev);

    size_t buffer_len = evbuffer_get_length(input);
    if (buffer_len <= 0)
		return ;

    IOManager::socketBytesReceived += buffer_len;
    //std::cout << "buffer_len " << buffer_len << std::endl;
    //evbuffer_remove(input,global_input_buffer,buffer_len);
    //streamInput->readData(global_input_buffer, buffer_len);

    BUFFER* buffer = NULL;
    //DispatcherStreamInput* streamInput = NULL;

    boost::unordered_map<struct evbuffer*, BUFFER* >::iterator bufferMapIt;
    boost::unordered_map<struct evbuffer*, boost::shared_ptr<IStreamInput> >::iterator dispatcherStreamInputMapIt;
    //boost::unordered_map<struct evbuffer*, DispatcherStreamInput* >::iterator dispatcherStreamInputMapIt;
    //boost::shared_ptr<QueueEntity> workerOutputQueue (new QueueEntity());
    //std::list<boost::shared_ptr<QueueEntity> >& getInputQueueList(void);

    // if buffer not found for the specific input stream OR following if is true when we receive input from a specific input for the first time
    if ((bufferMapIt = bufferMap.find(input)) == bufferMap.end())
    {
        buffer = new BUFFER();// need to be released some time
        //std::cout << "Buffer initialized : " << streamInput << std::endl;
        //streamInput = new DispatcherStreamInput();
        boost::shared_ptr<IStreamInput> streamInput (new DispatcherStreamInput());

        buffer->inputBuffer = (char*) malloc(PAGE_SIZE * 2);
        buffer->inputOffset = 0;

        bufferMap.insert(make_pair(input, buffer));
        // instances of DispatcherStreamInput in dispatcherStreamInputMap
        dispatcherStreamInputMap.insert(make_pair(input, streamInput));

        //streamInput->readData(buffer, input, buffer_len);
        // Casting streamInput (parent) into child type
        IStreamInput *dispStreamInput = streamInput.get();
        ((DispatcherStreamInput *)dispStreamInput)->setIsDispatcherStreamInput(true);
        ((DispatcherStreamInput *)dispStreamInput)->readData(buffer, input, buffer_len);

        std::cout << "StreamInput from IOManager " << dispStreamInput << std::endl;
        //std::cout << "_isDispatcherStreamInput from IOManager " << IOManager::getInstance()->getIsDispatcherStreamInput() << std::endl;
    }
    else
    {
        buffer = bufferMapIt->second;

        dispatcherStreamInputMapIt = dispatcherStreamInputMap.find(input);
        boost::shared_ptr<IStreamInput> streamInput = dispatcherStreamInputMapIt->second;
        IStreamInput *dispStreamInput = streamInput.get();
        ((DispatcherStreamInput *)dispStreamInput)->readData(buffer, input, buffer_len);
        //streamInput->readData(buffer, input, buffer_len);
    }

    //std::cout << "buffer_len: " << buffer_len << std::endl;
    //streamInput->readData(buffer, input, buffer_len);
}



void IOManager::errorcb(struct bufferevent *bev, short error, void *ctx)//error handler
{
	int test  = 1;
	if (error & BEV_EVENT_EOF) {
		/* connection has been closed, do any clean up here */
		/* ... */

		test++;
	} else if (error & BEV_EVENT_ERROR) {
		/* check errno to see what error occurred */
		/* ... */
		test++;
	} else if (error & BEV_EVENT_TIMEOUT) {
		/* must be a timeout event handle, handle it */
		/* ... */
		test++;
	}
	bufferevent_free(bev);
}



void IOManager::dispatcherStream_accept(evutil_socket_t listener, short event, void *arg)
{
    //std::cout << "IOManager::dispatcherStream_accept called " << std::endl;
    struct event_base *base = IOManager::getInstance()->base;
    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);
    int fd = accept(listener, (struct sockaddr*)&ss, &slen);
    if (fd < 0) {
        perror("accept");
    } else if (fd > FD_SETSIZE) {
        close(fd);
    } else {
        struct bufferevent *bev;
        evutil_make_socket_nonblocking(fd);
        bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

        bufferevent_setcb(bev, dispatcherStream_read, NULL, errorcb, arg);

        bufferevent_setwatermark(bev, EV_READ, 0, PAGE_SIZE);
        bufferevent_enable(bev, EV_READ|EV_WRITE);
    }
}

void IOManager::input_accept(evutil_socket_t listener, short event, void *arg)//listen socket callback function
{
	struct event_base *base = IOManager::getInstance()->base;
	struct sockaddr_storage ss;
	socklen_t slen = sizeof(ss);
	int fd = accept(listener, (struct sockaddr*)&ss, &slen);
	if (fd < 0) {
		perror("accept");
	} else if (fd > FD_SETSIZE) {
		close(fd);
	} else {
		struct bufferevent *bev;
		evutil_make_socket_nonblocking(fd);
		bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE);

		bufferevent_setcb(bev, input_read, NULL, errorcb, arg);

		bufferevent_setwatermark(bev, EV_READ, 0, WATER);
		bufferevent_enable(bev, EV_READ|EV_WRITE);
	}
}

////////////////////stream output logic//////////////
void IOManager::output_connected(struct bufferevent *bev, short events, void *ptr)
{
	OUTPUT_EV_BUFFER* outputEVBuffer = new OUTPUT_EV_BUFFER(); // need to release space later

	SocketStreamOutput * socketStreamOutput = (SocketStreamOutput*)ptr;

	//if(isExecutor)
	if(!(socketStreamOutput->isDispatcher))
	{
		bufferevent_setwatermark(bev, EV_WRITE, PAGE_SIZE*2, PAGE_SIZE*2);
		struct timeval write_timeout = {1,0};
		bufferevent_set_timeouts(bev, NULL, &write_timeout);
	}
	else // not executor
	{
        bufferevent_setwatermark(bev, EV_WRITE, 0, PAGE_SIZE);
        //output = bufferevent_get_output(bev);
	}

	/*
	struct evbuffer  *output;
	bufferevent_setwatermark(bev, EV_WRITE, 0, PAGE_SIZE);
	output = bufferevent_get_output(bev);
	struct event_base *base = IOManager::getInstance()->base;
	*/

	outputEVBuffer->outputBytes = 0;
	outputEVBuffer->outputBuffer = bufferevent_get_output(bev);

	struct event_base *base = IOManager::getInstance()->base;

	if (events & BEV_EVENT_CONNECTED)
	{
		//IOManager::getInstance()->outputMap.insert(make_pair((SocketStreamOutput*)ptr,output));
		IOManager::getInstance()->outputMap.insert(make_pair((SocketStreamOutput*)ptr, outputEVBuffer));
	}
	else if (events & BEV_EVENT_ERROR)
	{
		/* An error occured while connecting. */
	}
}

bool IOManager::writeOutput(char* buffer, int length, SocketStreamOutput* streamOutput)
{
    //std::cout << "streamOutput " << streamOutput << std::endl;

	boost::unordered_map<SocketStreamOutput*, OUTPUT_EV_BUFFER*>::iterator outputEVBufferMapIt;

	//boost::unordered_map<SocketStreamOutput*, struct evbuffer*>::iterator it;
	outputEVBufferMapIt = this->outputMap.find(streamOutput);
	if(outputEVBufferMapIt==outputMap.end())
	{
		//assert(false);
		usleep(5000);
		addStreamOutput(streamOutput->ip,streamOutput->port,streamOutput);
		return false;
	}

	/*
	struct evbuffer  *output = outputEVBufferMapIt->second;
	evbuffer_add(output, buffer, length);
	*/

	OUTPUT_EV_BUFFER* outputEVBuffer = outputEVBufferMapIt->second;
	struct evbuffer *output = outputEVBuffer->outputBuffer;

	//if(isExecutor)
	//if(!(streamOutput->isDispatcher))
	if(false)
    {
        if(outputEVBuffer->outputBytes + length > PAGE_SIZE*2)
        {
            IOManager::socketBytesSent += PAGE_SIZE*2;
            char chPadding[] = {'|'};
            for(int i = 0; i < (PAGE_SIZE*2 - outputEVBuffer->outputBytes); i++)
                {
                    evbuffer_add(output, chPadding, 1);
                    //IOManager::socketBytesSent += 1;
                }
               // IOManager::socketBytesSent += (PAGE_SIZE*2 - outputEVBuffer->outputBytes);
                outputEVBuffer->outputBytes = 0;
                return false;
        }
        else
        {
            evbuffer_add(output, buffer, length);
            //IOManager::socketBytesSent += length;
            outputEVBuffer->outputBytes = outputEVBuffer->outputBytes + length;
        }
    }
    else
    {
        evbuffer_add(output, buffer, length);
        //std::cout << "Buffer and length : " << buffer << " | " << length << " | IP and Port : " << streamOutput->ip << " , " << streamOutput->port <<  std::endl;
        IOManager::socketBytesSent += length;
    }

	return true;

}
////////////////////command input logic//////////////

void IOManager::command_accept(evutil_socket_t listener, short event, void *arg)//listen socket callback function
{
	std::cout << "command_accept " << std::endl;

	struct event_base *base = IOManager::getInstance()->base;
	struct sockaddr_storage ss;
	socklen_t slen = sizeof(ss);
	int fd = accept(listener, (struct sockaddr*)&ss, &slen);
	if (fd < 0) {
		perror("accept");
	} else if (fd > FD_SETSIZE) {
		close(fd);
	} else {

    //salman
    // To get client IP and port
    char ipstr[INET6_ADDRSTRLEN];
    int port;
    getpeername(fd, (struct sockaddr*)&ss, &slen);
    // deal with both IPv4 and IPv6:
    if (ss.ss_family == AF_INET)
    {
        struct sockaddr_in *s = (struct sockaddr_in *)&ss;
        port = ntohs(s->sin_port);
        inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
    }
    else
    { // AF_INET6
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&ss;
        port = ntohs(s->sin6_port);
        inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
    }
    // End getting client IP and port

    HttpParser::getInstance()->insertInPostRequestMap("ClientIP",ipstr);
    std::stringstream portSS;
    portSS << port;
    HttpParser::getInstance()->insertInPostRequestMap("ClientPort",portSS.str());
    //~salman


    struct bufferevent *bev;
    evutil_make_socket_nonblocking(fd);
    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE);

    bufferevent_setcb(bev, command_read, NULL, errorcb, arg);

    bufferevent_setwatermark(bev, EV_READ, 0, PAGE_SIZE);
    bufferevent_enable(bev, EV_READ|EV_WRITE);
	}
}

//void IOManager::http_accept(evutil_socket_t listener, short event, void *arg)
void IOManager::http_accept(struct evhttp_request * req, void * arg)
{
    //std::cout << "http command_accept " << std::endl;
    CommandManager * commandManager = (CommandManager*)arg;
    bool bl = commandManager->processHttpRequest(req);

}


void IOManager::command_read(struct bufferevent *bev, void *ctx)
{
	CommandManager * commandManager = (CommandManager*)ctx;
	struct evbuffer *input,*output;

	input = bufferevent_get_input(bev);
	output = bufferevent_get_output(bev);

	size_t buffer_len = evbuffer_get_length(input);

	if (buffer_len <= 0)
		return;

	// evbuffer_remove needs to be executed whether or not HTTP request
	evbuffer_remove(input,global_input_buffer,buffer_len);

	bool bl = commandManager->processCommand(global_input_buffer, buffer_len);

	*(bool*)global_input_buffer = bl;
	evbuffer_add(output, global_input_buffer, 1);
}


IOManager * IOManager::ioManager = NULL;
IOManager::IOManager(void)
{
#ifdef EVTHREAD_USE_PTHREADS_IMPLEMENTED
#endif
	evthread_use_pthreads();
	this->base = event_base_new();
	if (!this->base)
		return;

}


IOManager::~IOManager(void)
{
}

IOManager* IOManager::getInstance(void)
{
	if(ioManager==NULL)
	{
		ioManager = new IOManager();
	}
	return ioManager;
}

void IOManager::createNewThreadToPerfromIO()
{
	//event_base_loop(this->base,EVLOOP_NONBLOCK);
	event_base_dispatch(this->base);

}
void IOManager::execute()
{
	//wangyan
	boost::thread ioThread(boost::bind(&IOManager::createNewThreadToPerfromIO,this));
	//ioThread.join();
	int test = 0;
	test++;

}

void IOManager::addStreamInput(std::string ip, std::string port, SocketStreamInput* streamInput)
{
	int iport = atoi(port.c_str());
	evutil_socket_t listener;
	struct sockaddr_in my_addr;
	struct event *listener_event;
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(iport);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	//my_addr.sin_addr.s_addr = inet_addr(ip.c_str());
	listener = socket(AF_INET, SOCK_STREAM, 0 );
	evutil_make_socket_nonblocking(listener);
		int one = 1;
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

	if(::bind(listener, (struct sockaddr*)&my_addr, sizeof(my_addr))==-1)
	{
		perror("bind");
		return;
	};
	if (listen(listener, 16000)<0) {
		perror("listen");
		return;
	}

	listener_event = event_new(base, listener, EV_READ|EV_PERSIST, input_accept, (void*)streamInput);
	event_add(listener_event, NULL);

}





void IOManager::getDispatcherStream(std::string port, DispatcherManager* dispatcherManager)
{
    int iport = atoi(port.c_str());
    evutil_socket_t listener;
    struct sockaddr_in my_addr;
    struct event *listener_event;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(iport);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    listener = socket(AF_INET, SOCK_STREAM, 0 );
    evutil_make_socket_nonblocking(listener);
    int one = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    if(::bind(listener, (struct sockaddr*)&my_addr, sizeof(my_addr))==-1)
    {
        perror("bind");
        return;
    };

    if (listen(listener, 16)<0)
    {
        perror("listen");
        return;
    }

    struct event_base *base = IOManager::getInstance()->base;

    listener_event = event_new(base, listener, EV_READ|EV_PERSIST, dispatcherStream_accept, (void*)dispatcherManager);
    event_add(listener_event, NULL);
}



bool IOManager::addStreamOutput(std::string ip, std::string port, SocketStreamOutput* streamOutput)
{
	int iport = atoi(port.c_str());
	struct event_base *base = this->base;
	struct bufferevent *bev;
	struct sockaddr_in sin;

	//base = event_base_new();
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	//sin.sin_addr.s_addr = htonl(0x7f000001); /* 127.0.0.1 */
	sin.sin_addr.s_addr = inet_addr(ip.c_str());
	sin.sin_port = htons(iport); /* Port 8088 */

	bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE);

	bufferevent_setcb(bev, NULL, NULL, output_connected, (void*)streamOutput);
	//bufferevent_setcb(bev, NULL, NULL, output_connected, (void*)streamOutput);

	if (bufferevent_socket_connect(bev,(struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
			/* Error starting connection */
			bufferevent_free(bev);
			return false;
	}

	//event_base_dispatch(base);
	return true;
}


// Gets the command (including jsonQuery) from the client
void IOManager::addCommandInput(std::string ip,std::string port, CommandManager* commandManager)
{
	int iport = atoi(port.c_str());
	evutil_socket_t listener;
	struct sockaddr_in my_addr;
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(iport);
	int sockfd;

	struct event *listener_event;
	my_addr.sin_addr.s_addr = INADDR_ANY;
	//my_addr.sin_addr.s_addr = inet_addr(ip.c_str());
	listener = socket(AF_INET, SOCK_STREAM, 0 );
	evutil_make_socket_nonblocking(listener);

	int one = 1;
	setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

	if((sockfd = ::bind(listener, (struct sockaddr*)&my_addr, sizeof(my_addr)))==-1)
	{
		perror("bind");
		return;
	};

	if (listen(listener, 16000)<0)
	{
		perror("listen");
		return;
	}

    // sending back resposne
	/*
	int n;
    n = write(sockfd,"I got your message",18);
    if (n < 0) std::cout << "ERROR writing to socket" << std::endl;
    */

    //shutdown(sockfd, SHUT_RDWR);
    //close(sockfd);

	listener_event = event_new(base, listener, EV_READ|EV_PERSIST, command_accept, (void*)commandManager);
	event_add(listener_event, NULL);
}


void IOManager::addHttpCommandInput(std::string ip,std::string port, CommandManager* commandManager)
{
    //std::cout << "http commandInput_accept " << std::endl;

    const short  SERVER_BACKLOG    = 128;

    int iport = atoi(port.c_str());
	evutil_socket_t listener;
	struct sockaddr_in my_addr;

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(iport);
	my_addr.sin_addr.s_addr = INADDR_ANY;

	int sockfd;
	struct event *listener_event;

	//my_addr.sin_addr.s_addr = inet_addr(ip.c_str());
	//int server_sock = socket(AF_INET, SOCK_STREAM, 0);

	listener = socket(AF_INET, SOCK_STREAM, 0);
	evutil_make_socket_nonblocking(listener);

    if (listener == -1)
    {
        std::cout << "Error socket(): " << strerror(errno) << std::endl;
        return;
    }

    int one = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1)
    {
        std::cout << "Error setsockopt(): " << strerror(errno) << std::endl;
        return;
    }

    // Bind server socket to ip:port
    if (::bind(listener, (const sockaddr*)&my_addr, sizeof(my_addr)) == -1)
    {
        std::cout << "Error bind(): " << strerror(errno) << " on: " << ip << ":" << port << std::endl;
        return;
    }

    // Make server to listen
    if (listen(listener, SERVER_BACKLOG) == -1)
    {
        std::cout << "Error listen(): " << strerror(errno) << std::endl;
        return;
    }


    // Init events
    //struct event_base *base = IOManager::getInstance()->base;
    //event_base * base   = (event_base *)event_init();
    evhttp * http_server = evhttp_new(base);

    // Ignore SIGPIPE
    // signal(SIGPIPE, SIG_IGN);

    int fd;
    fd = evhttp_accept_socket(http_server, listener);

    /*
    if (fd = evhttp_accept_socket(http_server, listener) == -1)
    {
        std::cout << "Error evhttp_accept_socket(): " << strerror(errno) << std::endl;
        return;
    }
    */

    if (fd < 0)
    {
		perror("accept");
	}
	else if (fd > FD_SETSIZE)
	{
		close(fd);
	}
	else
	{
        // Set HTTP request callback
        evhttp_set_gencb(http_server, http_accept, (void*)commandManager);

        // Dispatch events
        //event_base_dispatch(base);
        event_base_loop(base, EVLOOP_NONBLOCK);
	}
}
