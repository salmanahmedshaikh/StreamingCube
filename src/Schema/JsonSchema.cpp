//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Schema/JsonSchema.h"
#include "../Common/Types.h"
#include <iostream>

JsonSchema::JsonSchema( Document document)
{
	this->document = document.copy();
	bool bl = valid();
	assert(bl==true);
	if(bl==false)
	{
		std::cout<<"schema error"<<std::endl;
		exit(0);
	}
	this->schemaId = this->document.getField(SCHEMA_ID).valuestr();

}

JsonSchema::~JsonSchema(void)
{
}

std::string JsonSchema::getId(void)
{
	return this->schemaId;
}

bool JsonSchema::valid(void) //valid top level
{
	assert(this->document.hasField(SCHEMA_ID));
	//assert(std::string (this->document.getField(SCHEMA_TYPE).valuestr())==SCHEMA_OBJECT);
	assert(this->document.hasField(SCHEMA_PROPERTIES));
	assert(this->document.getField(SCHEMA_PROPERTIES).isObjectType());
	Document propertiesDocument = this->document.getField(SCHEMA_PROPERTIES).embeddedObject();
	validProperties(propertiesDocument);
	return true;
}

bool JsonSchema::validProperties(Document& document)
{
	DocumentIterator it(document);
	while(it.more())
	{
		DocumentElement documentElement =  it.next();

		Document nestDocument = documentElement.embeddedObject();
		assert(nestDocument.hasField(SCHEMA_TYPE));
		assert(nestDocument.getField(SCHEMA_TYPE).isStringType());

		if(std::string(nestDocument.getField("type").valuestr())==SCHEMA_OBJECT)
		{
			assert( nestDocument.getField(SCHEMA_PROPERTIES).isObjectType());
			Document propertiesDocument = nestDocument.getField(SCHEMA_PROPERTIES).embeddedObject();
			validProperties(propertiesDocument);
		}
		else if(std::string(nestDocument.getField(SCHEMA_TYPE).valuestr())==SCHEMA_ARRAY)
		{
			//assert( nestDocument.getField(SCHEMA_PROPERTIES).isArrayType());
			Document itemsDocument = nestDocument.getField(SCHEMA_ITEMS).embeddedObject();
			validItems(itemsDocument);
		}
	}
	return true;
}

bool JsonSchema::validItems(Document& document)
{
	//assert(document.hasField(SCHEMA_TYPE));
	//assert(document.getField(SCHEMA_TYPE).isStringType());
	if(std::string(document.getField(SCHEMA_TYPE).valuestr())==SCHEMA_OBJECT)
	{
		assert( document.getField(SCHEMA_PROPERTIES).isObjectType());
		Document propertiesDocument = document.getField(SCHEMA_PROPERTIES).embeddedObject();
		validProperties(propertiesDocument);
	}
	else if(std::string(document.getField(SCHEMA_TYPE).valuestr())==SCHEMA_ARRAY)
	{
		assert( document.getField(SCHEMA_PROPERTIES).isArrayType());
		Document itemsDocument = document.getField(SCHEMA_ITEMS).embeddedObject();
		validItems(itemsDocument);
	}
	return true;
}

Document& JsonSchema::getSchemaDocument(void)
{
	return this->document;
}

std::ostream & operator <<(std::ostream &o, const JsonSchema& jsonSchema)
{
	o<<jsonSchema.document<<std::endl;
	return o;
}
