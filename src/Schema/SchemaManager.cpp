//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Schema/SchemaManager.h"
#include <boost/unordered_map.hpp>

SchemaManager * SchemaManager::schemaManager = NULL;
SchemaManager::SchemaManager(void)
{
}

SchemaManager::~SchemaManager(void)
{
}

SchemaManager * SchemaManager::getInstance()
{
	if(schemaManager==NULL)
	{
		schemaManager = new SchemaManager();
	}
	return schemaManager;
}

void SchemaManager::registerJsonSchema(boost::shared_ptr<JsonSchema> jsonSchema)
{
	this->schemaMap.insert(make_pair(jsonSchema->getId(),jsonSchema));
}

bool SchemaManager::getJsonSchemaById(std::string id, boost::shared_ptr<JsonSchema>& jsonSchema)
{
	boost::unordered_map<std::string, boost::shared_ptr<JsonSchema> >::iterator it;
	it = this->schemaMap.find( id );
	if(it!=this->schemaMap.end())
	{
		jsonSchema = it->second;
		return true;
	}
	return false;
}
