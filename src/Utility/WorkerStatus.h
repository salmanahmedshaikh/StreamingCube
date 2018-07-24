//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"

class WorkerStatus
{
private:
std::vector<std::string> workerNodes;
uint16_t in_cksum(uint16_t *addr, unsigned len);
static WorkerStatus* workerStatus;


public:
	WorkerStatus(void);
	~WorkerStatus(void);
	static WorkerStatus* getInstance();
	int ping(std::string target);
	std::vector<std::string> getWorkerNodes(void);
	std::vector<std::string> getWorkerIPs(void);


};
