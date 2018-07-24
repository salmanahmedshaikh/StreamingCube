//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include <boost/noncopyable.hpp>
#include "../Schema/JsonSchema.h"
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
/* manage all the schemas registered in the system */
class SchemaManager: private boost::noncopyable
{
private:
	static SchemaManager * schemaManager;
	SchemaManager(void);
	boost::unordered_map<std::string, boost::shared_ptr<JsonSchema> >schemaMap;
public:
	static SchemaManager * getInstance(void);
	~SchemaManager(void);
	void registerJsonSchema(boost::shared_ptr<JsonSchema> jsonSchema);
	bool getJsonSchemaById(std::string id, boost::shared_ptr<JsonSchema>& jsonSchema);
};

