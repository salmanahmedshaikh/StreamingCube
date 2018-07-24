//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Utility/MyStatus.h"
#include <iostream>
#include <unistd.h>

#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

MyStatus* MyStatus::myStatus = NULL;

MyStatus::MyStatus(void)
{
}

MyStatus::~MyStatus(void)
{
}

void MyStatus::setWorkerID(std::string workerID)
{
    MyStatus::workerID = atoi(workerID.c_str());
    //std::cout << "MyStatus::workerID " << MyStatus::workerID << std::endl;
}

int MyStatus::getWorkerID()
{
    return MyStatus::workerID;
}

void MyStatus::setTotalActiveWorkers(std::string totalActiveWorkers)
{
    MyStatus::totalActiveWorkers = atoi(totalActiveWorkers.c_str());
    //std::cout << "MyStatus::TotalActiveWorkers " << MyStatus::totalActiveWorkers << std::endl;
}

int MyStatus::getTotalActiveWorkers(void)
{
    return totalActiveWorkers;
}

void MyStatus::setActiveWorkerSeqNum(std::string activeWorkerSeqNum)
{
    MyStatus::activeWorkerSeqNum = atoi(activeWorkerSeqNum.c_str());
    //std::cout << "MyStatus::activeWorkerNum " << MyStatus::activeWorkerNum << std::endl;
}

int MyStatus::getActiveWorkerSeqNum(void)
{
    return activeWorkerSeqNum;
}

void MyStatus::setQueryID(std::string queryID)
{
    MyStatus::queryID = atoi(queryID.c_str());
    //std::cout << "MyStatus::queryID " << MyStatus::queryID << std::endl;
}

int MyStatus::getQueryID(void)
{
    return queryID;
}

MyStatus* MyStatus::getInstance()
 {
	 if(myStatus==NULL)
	 {
		 myStatus = new MyStatus();
	 }
	 return myStatus;
 }

std::string MyStatus::getMyIP()
{
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;
    std::string returnIP="-1";

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
        {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET)
        {   // check it is IP4 and is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            //printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
            std::string hostName = ifa->ifa_name;
            std::string hostIP = addressBuffer;
            if(hostName.compare("eth0") == 0 || hostIP.compare("127.0.0.1") != 0)
            {
                //std::cout << addressBuffer << std::endl;
                returnIP = hostIP;
            }
        }
        else if (ifa->ifa_addr->sa_family == AF_INET6)
        { // check it is IP6 and is a valid IP6 Address
            tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            //printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
        }
    }
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);

    return returnIP;
}

