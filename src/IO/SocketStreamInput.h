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
#include <boost/asio.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include "../Internal/Synopsis/WindowSynopsis.h"

#define SOCKET_INPUT_BUFFER_SIZE 5120
#define REMAINING_INPUT_BUFFER_SIZE 1024

class SocketStreamInput:public IStreamInput
{
private:
	boost::asio::streambuf streambuffer;
	std::string ip;
	std::string port;
	void getNextElement(Element& element);
	boost::mutex m_oMutex;
	boost::shared_ptr<WindowSynopsis> windowSynopsis;
	//bool checkWindowSynopsisCorrect(int number);
	std::string remainingDataFromPrevArrivalStr = "";

	char *remainingPlusNewData = (char*)malloc(SOCKET_INPUT_BUFFER_SIZE);
	char *remainingDataChr = (char*)malloc(REMAINING_INPUT_BUFFER_SIZE);

public:
	SocketStreamInput(std::string ip,std::string port,boost::shared_ptr<JsonSchema> schema);
	~SocketStreamInput(void);
	void readData(char* data, int length);
	bool isEmpty();
	void initial();
	static int ignoredInputNumber;
	static int totalInputNumber;

};

