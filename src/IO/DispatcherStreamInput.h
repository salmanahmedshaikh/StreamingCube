//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../Internal/Element/Element.h"
#include "../IO/IStreamInput.h"
#include "../Schema/JsonSchema.h"
//#include "../IO/IOManager.h"
#include <boost/asio.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include "../Internal/Synopsis/WindowSynopsis.h"


typedef struct _BUFFER
{
    char* inputBuffer;
    int inputOffset;
    std::string workerID;
    std::string queryID;
    std::string activeWorkerSeqNum;
}BUFFER;


class DispatcherStreamInput:public IStreamInput
{
private:
	boost::asio::streambuf streambuffer;
	std::string ip;
	std::string port;
	void getNextElement(Element& element);
	boost::mutex m_oMutex;
	boost::shared_ptr<WindowSynopsis> windowSynopsis;
	bool isStarterElementArrived;
	bool isDispatcherStreamInput;

public:
	DispatcherStreamInput(void);
	DispatcherStreamInput(std::string streamSource);
	~DispatcherStreamInput(void);

	void setIsDispatcherStreamInput(bool isDispatcherStreamInput);
	bool getIsDispatcherStreamInput(void);

	std::string streamSource;
	//void readData(char* data, int length);
	void readData(BUFFER* buffer, struct evbuffer*, int);
	bool isEmpty();
	void initial();
	static int ignoredInputNumber;
};

