//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Schema/SchemaInterpreter.h"
#include "../Schema/JsonSchema.h"
#include "../Common/Types.h"
#include "../Utility/TimestampGenerator.h"

SchemaInterpreter::SchemaInterpreter(void)
{

}


SchemaInterpreter::~SchemaInterpreter(void)
{

}

void SchemaInterpreter::generateJSONDocumentBySchema(std::string csvLINE, boost::shared_ptr<JsonSchema> jsonSchema, Document& outputDocument)
{
    DocumentBuilder documentBuilder;
    Document  schemaDocument = jsonSchema->getSchemaDocument();
    Document  propertiesDocument = schemaDocument.getField(SCHEMA_PROPERTIES).embeddedObject();

    DocumentIterator it(propertiesDocument);
    std::size_t pos1 = 0;
    std::size_t pos2 = 0;

    while(it.more())
	{
		DocumentElement documentElement =  it.next();

		std::string fieldName = documentElement.fieldName();
		Document fieldDocument = documentElement.embeddedObject();

		std::string fieldType = std::string(fieldDocument.getField(SCHEMA_TYPE).valuestr());
		JSONTYPE jsonType =  generateJsonType(fieldType);

		std::size_t commaPos = csvLINE.find(',', pos1 + 1);
        std::string valueStr = "";

        if(commaPos != std::string::npos)
        {
            pos2 = commaPos;
            valueStr = csvLINE.substr(pos1, pos2 - pos1);
        }
        else if(commaPos == std::string::npos)
        {
            pos2 = 0;
            valueStr = csvLINE.substr(pos1);
        }
        else
        {
            std::cout << "Error! Schema interpreter passed EOL" << std::endl;
            assert(false);
            exit(0);
        }

		if(jsonType == JSON_NUMBER)
		{
            std::stringstream ss(valueStr);
            int valueInt;
            ss >> valueInt;
            pos1 = pos2 + 1;

			documentBuilder.append(fieldName,valueInt);
		}
		else if(jsonType == JSON_INT)
		{
            std::stringstream ss(valueStr);
            int valueInt;
            ss >> valueInt;
            pos1 = pos2 + 1;

			documentBuilder.append(fieldName,valueInt);
		}
		else if(jsonType == JSON_FLOAT)
		{
            std::stringstream ss(valueStr);
            float valueFloat;
            ss >> valueFloat;
            pos1 = pos2 + 1;

			documentBuilder.append(fieldName,valueFloat);
		}
		/*
		else if(jsonType == JSON_DOUBLE)
		{
            std::stringstream ss(valueStr);
            double valueDouble;
            ss >> valueDouble;
            pos1 = pos2;

			documentBuilder.append(fieldName,valueDouble);
		}*/
		else if(jsonType == JSON_STRING)
		{
            pos1 = pos2 + 1;

			documentBuilder.append(fieldName,valueStr);
		}
	}

    outputDocument = documentBuilder.obj();
}

void SchemaInterpreter::generateRandomDocumentBySchema(boost::shared_ptr<JsonSchema> jsonSchema, Document& outputDocument)
{

	Document  schemaDocument = jsonSchema->getSchemaDocument();
	Document  propertiesDocument = schemaDocument.getField(SCHEMA_PROPERTIES).embeddedObject();
	generateRandomObject(propertiesDocument,outputDocument);
}

void SchemaInterpreter::generateRandomInteger(int& i)
{
//	Timestamp t = TimestampGenerator::getCurrentTime();
//	i = t%5+20;
	i = rand()%10;
}

void SchemaInterpreter::generateRandomString(std::string& str)
{
	//std::string strArray[3];
	//strArray[0] = "id_001";
	//strArray[1] = "id_002";
	//strArray[2] = "id_003";
	//Timestamp t = TimestampGenerator::getCurrentTime();
	str = "hello world";
	//str = strArray[0];
	//str = "string1";
}

void SchemaInterpreter::generateRandomObject(Document & propertiesDocument, Document& outputDocument)
{
	DocumentBuilder documentBuilder;
	DocumentIterator it(propertiesDocument);
	while(it.more())
	{
		DocumentElement documentElement =  it.next();
		std::string fieldName = documentElement.fieldName();
		Document fieldDocument = documentElement.embeddedObject();
		std::string fieldType = std::string(fieldDocument.getField(SCHEMA_TYPE).valuestr());
		JSONTYPE jsonType =  generateJsonType(fieldType);

		if(jsonType==JSON_TRUE)
		{
			documentBuilder.appendBool(fieldName,true);
		}
		else if(jsonType == JSON_FALSE)
		{
			documentBuilder.appendBool(fieldName,false);
		}
		else if(jsonType == JSON_ARRAY)
		{
			Document nestedDocument;
			Document  nestedItemsDocument = fieldDocument.getField(SCHEMA_ITEMS).embeddedObject();
			generateRandomArray(nestedItemsDocument,nestedDocument);
			documentBuilder.append(fieldName,nestedDocument);
		}
		else if(jsonType == JSON_NULL)
		{
			documentBuilder.appendBool(fieldName,NULL);
		}
		else if(jsonType == JSON_NUMBER)
		{
			int i;
			generateRandomInteger(i);
			documentBuilder.append(fieldName,i);
		}
		else if(jsonType == JSON_OBJECT)
		{
			Document nestedDocument;
			Document  nestedPropertiesDocument = fieldDocument.getField(SCHEMA_PROPERTIES).embeddedObject();
			generateRandomObject(nestedPropertiesDocument,nestedDocument);
			documentBuilder.append(fieldName,nestedDocument);
		}
		else if(jsonType == JSON_STRING)
		{
			string str;
			generateRandomString(str);
			documentBuilder.append(fieldName,str);
		}

	}
	outputDocument = documentBuilder.obj();
}
void SchemaInterpreter::generateRandomArray(Document & itemsDocument, Document& outputDocument)
{
	ArrayBuilder arrayBuilder;

	std::string fieldType = std::string(itemsDocument.getField(SCHEMA_TYPE).valuestr());
	JSONTYPE jsonType =  generateJsonType(fieldType);

	int arraySize = 3;
	if(jsonType==JSON_TRUE)
	{
		for(int i = 0 ;i<arraySize ; i++)
		{
			strstream ss;
			ss<<i;
			arrayBuilder.append(ss.str(),true);
		}

	}
	else if(jsonType == JSON_FALSE)
	{
		for(int i = 0 ;i<arraySize ; i++)
		{
			strstream ss;
			ss<<i;
			arrayBuilder.append(ss.str(),false);
		}
	}
	else if(jsonType == JSON_ARRAY)
	{
		for(int i = 0 ;i<arraySize ; i++)
		{
			strstream ss;
			ss<<i;
			Document nestedDocument;
			Document  nestedItemsDocument = itemsDocument.getField(SCHEMA_ITEMS).embeddedObject();
			generateRandomArray(nestedItemsDocument,nestedDocument);
			arrayBuilder.append(ss.str(),nestedDocument);
		}

	}
	else if(jsonType == JSON_NULL)
	{
//todo wangyan
//		for(int i = 0 ;i<arraySize ; i++)
//		{
//			strstream ss;
//			ss<<i;
//			arrayBuilder.append(ss.str(),NULL);
//		}
	}
	else if(jsonType == JSON_NUMBER)
	{
		int random;
		for(int i = 0 ;i<arraySize ; i++)
		{
			generateRandomInteger(random);
			strstream ss;
			ss<<i;
			arrayBuilder.append(ss.str(),random);
		}

	}
	else if(jsonType == JSON_OBJECT)
	{
		for(int i = 0 ;i<arraySize ; i++)
		{

			strstream ss;
			ss<<i;
			Document nestedDocument;
			Document  nestedPropertiesDocument = itemsDocument.getField(SCHEMA_PROPERTIES).embeddedObject();
			generateRandomObject(nestedPropertiesDocument,nestedDocument);
			arrayBuilder.append(ss.str(),nestedDocument);
		}

	}
	else if(jsonType == JSON_STRING)
	{
		for(int i = 0 ;i<arraySize ; i++)
		{
			strstream ss;
			ss<<i;
			string str;
			generateRandomString(str);
			arrayBuilder.append(ss.str(),str);
		}

	}
	outputDocument = arrayBuilder.obj();
}
bool SchemaInterpreter::checkDocumentSatisfiedSchema(Document& document, boost::shared_ptr<JsonSchema>jsonSchema)
{
	//wang yan
	//just check the attribute name, not check the attribute type
	Document schemaDocument = jsonSchema->getSchemaDocument();
	Document propertyDocument = schemaDocument.getField(SCHEMA_PROPERTIES).Obj();
	std::set< std::string > schemaFields;
	propertyDocument.getFieldNames (schemaFields);

	std::set<std::string> documentFields;
	document.getFieldNames(documentFields);
	if (includes(documentFields.begin(), documentFields.end(), schemaFields.begin(), schemaFields.end()))
	{
		return true;
	}

	return false;
}
