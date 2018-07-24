//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Utility/StreamDistribution.h"
#include "../Utility/MyStatus.h"
#include "../Utility/Hashing.h"
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <cmath>
#include <math.h>

StreamDistribution* StreamDistribution::streamDistribution = NULL;

StreamDistribution::StreamDistribution(void)
{
    StreamDistribution::totalActiveWorkers = MyStatus::getInstance()->getTotalActiveWorkers();
    StreamDistribution::activeWorkerSeqNum = MyStatus::getInstance()->getActiveWorkerSeqNum();
    //StreamDistribution::replicateStreamSource = false;
}

StreamDistribution::~StreamDistribution(void)
{
}

StreamDistribution* StreamDistribution::getInstance()
 {
	 if(streamDistribution==NULL)
	 {
		 streamDistribution = new StreamDistribution();
	 }
	 return streamDistribution;
 }

 bool StreamDistribution::roundRobinDistribution(bool isMasterStream, bool replicateStreamSource, DocumentId elementID)
 {
    //std::cout << "StreamDistribution::totalActiveWorkers " << StreamDistribution::totalActiveWorkers << std::endl;
    if(isMasterStream || replicateStreamSource || (elementID - StreamDistribution::activeWorkerSeqNum) % StreamDistribution::totalActiveWorkers == 0)
        return true;
    else
        return false;
 }

 bool StreamDistribution::groupByDistribution(bool isMasterStream, Element element, std::string groupByAttribute)
 {
    std::string groupByAttributeStr;
    Document& elementDocument = element.document;
    BinaryJsonElement partialElement = elementDocument.getField(groupByAttribute.c_str());

    if(partialElement.isInteger())
    {
            int elementVal;
            partialElement.Val(elementVal);
            groupByAttributeStr = boost::lexical_cast<std::string>(elementVal);
            //std::cout << elementVal << std::endl;
    }
    else if(partialElement.isDouble())
    {
            double elementVal;
            partialElement.Val(elementVal);
            groupByAttributeStr = boost::lexical_cast<std::string>(elementVal);
            //std::cout << elementVal << std::endl;
    }
    else if(partialElement.isLong())
    {
            long long elementVal;
            partialElement.Val(elementVal);
            groupByAttributeStr = boost::lexical_cast<std::string>(elementVal);
            //std::cout << elementVal << std::endl;
    }
    else if(partialElement.isNumber())
    {
            long long elementVal;
            partialElement.Val(elementVal);
            groupByAttributeStr = boost::lexical_cast<std::string>(elementVal);
            //std::cout << elementVal << std::endl;
    }
    else if(partialElement.isStringType())
    {
            std::string elementVal;
            partialElement.Val(elementVal);
            groupByAttributeStr = boost::lexical_cast<std::string>(elementVal);
            //std::cout << elementVal << std::endl;
    }
    else if(partialElement.isNull())
    {
            std::cout << "Group-By attribute not found." << std::endl;
            assert(false);

    }
    else
    {
            std::cout << "Not a valid Group By attribute." << std::endl;
            assert(false);
    }

    //std::string groupByAttributeVal = elementDocument.getStringField(groupByAttribute.c_str());
    //std::cout << groupByAttributeVal << std::endl;

		//std::set<std::string> elementFields;
		//int numFields = elementDocument.getFieldNames(elementFields);
		//std::set<std::string>::iterator elementFieldsIt;


		//for(elementFieldsIt=elementFields.begin(); elementFieldsIt != elementFields.end(); elementFieldsIt++)
		//{
        //    std::string str = boost::lexical_cast<std::string>(*elementFieldsIt);
            //std::cout << *elementFieldsIt << std::endl;

            //std::cout << elementDocument.getStringField(str.c_str()) << std::endl;

            //std::stringstream  partialElementStr << partialElement;
            //const std::string partialElementStr = boost::lexical_cast<std::string>(partialElement);
            //std::cout << partialElementStr << std::endl;
            //std::cout << elementDocument.getFieldDotted(str.c_str()) << std::endl;
            //std::cout << elementDocument.getField() << std::endl;
		//}

    //std::stringstream elementIDSStr;
    //elementIDSStr << elementID;
    //const std::string& elementIDStr = elementIDSStr.str();

    int hashVal = Hashing::getInstance()->stringHashing(groupByAttributeStr.c_str(), StreamDistribution::totalActiveWorkers);
    //std::cout << "hashVal " << hashVal << std::endl;
    if(isMasterStream || hashVal == StreamDistribution::activeWorkerSeqNum)
        return true;
    else
        return false;
 }

