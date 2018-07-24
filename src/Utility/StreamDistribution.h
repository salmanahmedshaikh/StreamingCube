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
#include "../Internal/Element/Element.h"

class StreamDistribution
{
private:
    static StreamDistribution* streamDistribution;

    int totalActiveWorkers;
    int activeWorkerSeqNum;
    //bool replicateStreamSource;

public:
	StreamDistribution(void);
	~StreamDistribution(void);
	static StreamDistribution* getInstance();

	bool roundRobinDistribution(bool isMasterStream, bool replicateStreamSource, DocumentId elementID);
	//bool groupByDistribution(bool isMasterStream, DocumentId elementID);
	bool groupByDistribution(bool isMasterStream, Element element, std::string groupByAttribute);
};

