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
#pragma once
#include "../Common/stdafx.h"
#include "../Internal/Element/Element.h"
#include "../IO/IStreamInput.h"
#include "../Schema/JsonSchema.h"
#include "../Internal/Synopsis/WindowSynopsis.h"
#include <boost/asio.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <stdexcept>
#include <string>

using namespace std;

class CSVInput
{

private:
	std::string ip;
	std::string port;
	std::string CSVFile;
	boost::shared_ptr<JsonSchema> schema;
	std::string id;

public:
	CSVInput();
	CSVInput(std::string ip, std::string port, std::string CSVFile, boost::shared_ptr<JsonSchema> schema);
	virtual ~CSVInput();

	std::string getId(void);
    std::string getCSVFile(void);
    boost::shared_ptr<JsonSchema> getSchema(void);
};

