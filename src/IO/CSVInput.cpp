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
//#include "../BinaryJson/BinaryInline.h"
//#include "../BinaryJson/BinaryJsonElement.h"
//#include "../BinaryJson/BinaryJsonObjectIterator.h"
#include "../Common/Types.h"
#include "../Schema/SchemaInterpreter.h"
#include "../Utility/DocumentIdentifierGenerator.h"
#include "../Utility/TimestampGenerator.h"
#include "../IO/CSVInput.h"

#include <iostream>
#include <boost/smart_ptr/shared_ptr.hpp>

using namespace std;

CSVInput::CSVInput() {}

CSVInput::~CSVInput() {
	// TODO Auto-generated destructor stub
}

CSVInput::CSVInput(std::string ip, std::string port, std::string CSVFile, boost::shared_ptr<JsonSchema> schema)
{
	this->ip = ip;
	this->port = port;
	this->CSVFile = CSVFile;
	this->schema = schema;
	this->id = schema->getId();
}

std::string CSVInput::getId(void)
{
	return this->id;
}

std::string CSVInput::getCSVFile(void)
{
	return this->CSVFile;
}

boost::shared_ptr<JsonSchema> CSVInput::getSchema(void)
{
	return this->schema;
}
