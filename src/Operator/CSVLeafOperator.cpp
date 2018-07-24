//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////


/*
 * CSVInput.h
 *
 *  Created on: August 2, 2016
 *      Author: salman
 */
#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <stdexcept>
#include <string>

#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "../BinaryJson/json.h"
#include "../Utility/DocumentIdentifierGenerator.h"
#include "../Utility/TimestampGenerator.h"
#include "../Query/QueryUtility.h"
#include "../Operator/CSVLeafOperator.h"
#include "../Schema/SchemaInterpreter.h"

using namespace std;

CSVLeafOperator::CSVLeafOperator()
{}

CSVLeafOperator::~CSVLeafOperator()
{
	// TODO Auto-generated destructor stub
}

void CSVLeafOperator::setCSVInput(boost::shared_ptr<CSVInput> csvInput)
{
	this->csvInput = csvInput;
	this->isCSVLeafExecuted = false;
}

void CSVLeafOperator::execution()
{
    #ifdef DEBUG
	std::cout<<"===================operator begin================="<<std::endl;
	std::cout<<"operatorid : "<<this->getId()<<std::endl;
    #endif

    this->csvFileStream.open(csvInput->getCSVFile().c_str());
    //std::cout << "File path: " << csvInput->getCSVFile().c_str() << std::endl;
    while(this->csvFileStream.peek()!=EOF)
    {
        std::string strLine;
        Element element;
        Document document;

        // read one whole line from the file
        getline (this->csvFileStream, strLine);

        // The case when the file has multiple EOL characters
        if (strLine.size() < 1)
            return;

        // generate document using json schema
        SchemaInterpreter::generateJSONDocumentBySchema(strLine, csvInput->getSchema(), document);

        element.timestamp = TimestampGenerator::getCurrentTime();
        element.mark = PLUS_MARK;
        element.id = DocumentIdentifierGenerator::generateNewDocumentIdentifier();
        element.document = document;
        element.masterTag = true;
        //std::cout << element << std::endl;
		output(element);
    }

    #ifdef DEBUG
	std::cout<<"===================operator over================="<<std::endl;
    #endif

    this->csvFileStream.close();
    this->isCSVLeafExecuted = true;
}

bool CSVLeafOperator::getIsCSVLeafExecuted()
{
    return this->isCSVLeafExecuted;
}


