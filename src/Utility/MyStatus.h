//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
class MyStatus
{
private:
    static MyStatus* myStatus;
    int workerID;
    int totalActiveWorkers;
    int activeWorkerSeqNum;
    int queryID;

public:

	MyStatus(void);
	~MyStatus(void);
	static MyStatus* getInstance();

	std::string getMyIP();

	int getWorkerID();
	void setWorkerID(std::string);

	int getTotalActiveWorkers(void);
	void setTotalActiveWorkers(std::string);

	int getActiveWorkerSeqNum(void);
	void setActiveWorkerSeqNum(std::string);

	int getQueryID(void);
	void setQueryID(std::string);
};

