//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../IO/IStreamOutput.h"
class SocketStreamOutput :  public IStreamOutput
{
private:
    //static SocketStreamOutput* socketStreamOutput;
    void createCommandElement(Element&, std::string);


public:
    static int totalOutputElementsCounter;
    static bool isFirstElement;
	static bool isLastElement;

	std::string ip;
	std::string port;
	bool isDispatcherStarterElementSent;
	bool isDispatcher;
	std::string streamSource;

	SocketStreamOutput(std::string ip,std::string port);
	SocketStreamOutput(std::string ip,std::string port, std::string streamSource);
	SocketStreamOutput(std::string ip,std::string port, std::string streamSource, bool isExecutor);

	~SocketStreamOutput(void);
	void pushNext(Element& element);
	bool isFull(void) ;
	void initial(void) ;
	static SocketStreamOutput* getInstance();
	int getTotalOutputElementsCount(void);
};

