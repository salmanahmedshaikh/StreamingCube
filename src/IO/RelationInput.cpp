//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

/*
 * RelationInput.cpp
 *
 *  Created on: May 21, 2015
 *      Author: root
 */

#include "RelationInput.h"

#include <boost/smart_ptr/shared_ptr.hpp>

#include "../BinaryJson/BinaryInline.h"
#include "../BinaryJson/BinaryJsonElement.h"
#include "../BinaryJson/BinaryJsonObjectIterator.h"
#include "../Common/Types.h"
#include "../Schema/SchemaInterpreter.h"
#include "../Utility/DocumentIdentifierGenerator.h"
#include "../Utility/TimestampGenerator.h"

#include <iostream>

//using namespace oracle::occi;
using namespace std;

RelationInput::RelationInput() {

}

RelationInput::~RelationInput() {
	// TODO Auto-generated destructor stub
}

/*
// todo this method should be in SchemaInterpreter?
Document generateDocumentFromResult(Document propertiesDocument, ResultSet * res)
{

	DocumentBuilder documentBuilder;
	DocumentIterator it(propertiesDocument);
	int i = 1;
	while (it.more()) {
		DocumentElement documentElement = it.next();
		std::string fieldName = documentElement.fieldName();
		Document fieldDocument = documentElement.embeddedObject();
		std::string fieldType = std::string(
				fieldDocument.getField(SCHEMA_TYPE).valuestr());
		JSONTYPE jsonType = generateJsonType(fieldType);

		if (jsonType == JSON_TRUE) {
			documentBuilder.appendBool(fieldName, true);
		} else if (jsonType == JSON_FALSE) {
			documentBuilder.appendBool(fieldName, false);
		} else if (jsonType == JSON_ARRAY) {
			Document nestedDocument;
			Document nestedItemsDocument =
					fieldDocument.getField(SCHEMA_ITEMS).embeddedObject();
			documentBuilder.append(fieldName, nestedDocument);
		} else if (jsonType == JSON_NULL) {
			documentBuilder.appendBool(fieldName, NULL);
		} else if (jsonType == JSON_NUMBER) {
			int value = res->getInt(i++);
			documentBuilder.append(fieldName, value);
		} else if (jsonType == JSON_OBJECT) {
			Document nestedDocument;
			Document nestedPropertiesDocument = fieldDocument.getField(
					SCHEMA_PROPERTIES).embeddedObject();
			documentBuilder.append(fieldName, nestedDocument);
		} else if (jsonType == JSON_STRING) {
			string str = res->getString(i++);
			documentBuilder.append(fieldName, str);
		}

	}
	return documentBuilder.obj();
}
*/

RelationInput::RelationInput(std::string ip, std::string port,
		std::string userName, std::string userPassword,
		std::string databaseName, std::string tableName,
		boost::shared_ptr<JsonSchema> schema) {

	this->ip = ip;
	this->port = port;
	this->userName = userName;
	this->userPassword = userPassword;
	this->databaseName = databaseName;
	this->tableName = tableName;
	this->schema = schema;

	this->id = schema->getId();

}

std::string getSqlFromSchema(boost::shared_ptr<JsonSchema> schema, std::string tableName)
{
	Document schemaDocument = schema->getSchemaDocument();
	Document propertiesDocument =
			schemaDocument.getField(SCHEMA_PROPERTIES).embeddedObject();

	DocumentIterator it(propertiesDocument);

	std::string sql = "select ";

	while (it.more()) {
		DocumentElement documentElement = it.next();
		std::string fieldName = documentElement.fieldName();
		sql += fieldName + ", ";
	}
	sql.erase(sql.find_last_of(","));
	sql += " from " + tableName;

	return sql;

}

list<Element> RelationInput::getAllElementsFromDatabase()
{
	// todo	make it general by using schema

	/*
	Environment *env;
	Connection *conn;
	Statement *stmt;
	env = Environment::createEnvironment(Environment::DEFAULT);
	conn = env->createConnection(this->userName, this->userPassword,
			databaseName);
    */

	list<Element> elements;

	/*
	Document schemaDocument = this->schema->getSchemaDocument();
	Document propertiesDocument =
			schemaDocument.getField(SCHEMA_PROPERTIES).embeddedObject();

	string sqlStmt = getSqlFromSchema(this->schema, this->tableName);

	stmt = conn->createStatement(sqlStmt);
	ResultSet *rset = stmt->executeQuery();

	while (rset->next()) {

		Document document = generateDocumentFromResult(propertiesDocument,
				rset);
		Element element;
		element.id =
				DocumentIdentifierGenerator::generateNewDocumentIdentifier();
		element.mark = PLUS_MARK;
		element.timestamp = TimestampGenerator::getCurrentTime();
		element.document = document;
		element.document.getOwned();

		elements.push_back(element);
	}
	*/


	return elements;
}

