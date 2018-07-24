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
#include <boost/shared_ptr.hpp>
#include <iostream>
static const char* SCHEMA_ID = "id";
static const char* SCHEMA_TYPE = "type";
static const char* SCHEMA_PROPERTIES = "properties";
static const char* SCHEMA_ITEMS = "items";
static const char* SCHEMA_OBJECT = "object";
static const char* SCHEMA_ARRAY = "array";

// schema example 1
//{
//    "id": "schema1¡å,
//    "description": "a schema example1¡å,
//    "type": "object",
//    "properties": {
//                   "id": {
//                          "description": "identifier",
//                          "type": "string"
//	                       },
//                   "num": {
//                           "description": "total numbers",
//                           "type": "integer"
//			                }
//	                 }
// }
// schema example 2
//{
//    "id": "schema1¡å,
//    "description": "a schema example2¡å,
//    "type": "object",
//    "properties": {
//                   "id": {
//                          "description": "identifier",
//                          "type": "string"
//	                       },
//                   "num": {
//                           "type": "array",
//                           "items": {
//                                     "description": "number array",
//                                     "type": "string"
//		                              }
//			                }
//	                 }
// }
class JsonSchema
{
private:
	Document document;
	std::string schemaId;
	bool valid(void);
	bool validProperties(Document& document);
	bool validItems(Document& document);
public:
	JsonSchema(Document document);
	~JsonSchema(void);
	Document& getSchemaDocument(void);
	std::string getId(void);
	friend ostream & operator <<(ostream &o, const JsonSchema& jsonSchema);
};

