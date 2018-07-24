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
#include <boost/noncopyable.hpp>

//wrapper file	example
//{
//		"implement_class" : "RandomGeneratedStreamInput",
//		"class_argument":{},
//		"information_source_schema":{"id": "stream1",
//									"type":"object",
//									"properties": {
//													"id":{
//														"type": "string"
//														}
//												   }
//									}
//}
static const std::string WRAPPER_FOLDER = "wrapper_folder";
static const std::string IMPLEMENT_CLASS = "implement_class";
static const std::string CLASS_ARGUMENT = "class_argument";
static const std::string INFORMATION_SOURCE_SCHEMA = "information_source_schema";

//socket wrapper
static const std::string IP = "input_ip";
static const std::string PORT = "input_port";

//twitter wrapper
static const std::string USER_NAME = "user_name";
static const std::string USER_PASSWORD = "user_password";

//RDBMS wrapper
static const std::string DATABASE_NAME = "database_name";
static const std::string TABLE_NAME = "table_name";

//RSS wrapper
static const std::string URL_ARRAY = "url_array";
static const std::string CORRESPONDING_ENCODING_ARRAY = "corresponding_encoding_array";

//people flow wrapper
static const std::string DATA_FOLDER = "data_folder";

//OLAP dimension wrapper
static const std::string DATA_FILE = "data_file";

class WrapperManager:private boost::noncopyable
{
private:
	static WrapperManager* wrapperManager;
	WrapperManager(void);
	void initial(void);
	void readWrapperFile(std::string filePath);


public:
	~WrapperManager(void);
	static WrapperManager* getInstance(void);
	bool registerWrapper(Document& wrapperDocument);
};

