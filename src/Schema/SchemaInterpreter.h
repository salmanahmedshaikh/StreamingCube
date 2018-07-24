//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../Schema/JsonSchema.h"
#include "../Common/Types.h"
#include "../Schema/JsonSchema.h"
#include <boost/shared_ptr.hpp>

/* explain the schema content
   generate random values according to the schema
 */

class SchemaInterpreter
{
private:
	SchemaInterpreter(void);
	~SchemaInterpreter(void);
	static void generateRandomInteger(int& i);
	static void generateRandomString(std::string& str);
	static void generateRandomObject(Document & propertiesDocument, Document& outputDocument);
	static void generateRandomArray(Document & itemsDocument, Document& outputDocument);
public:
	static void generateRandomDocumentBySchema(boost::shared_ptr<JsonSchema> jsonSchema, Document& document);
	static void generateJSONDocumentBySchema(std::string csvLINE, boost::shared_ptr<JsonSchema> jsonSchema, Document& outputDocument);
	static bool checkDocumentSatisfiedSchema(Document& document, boost::shared_ptr<JsonSchema>jsonSchema);
};

