//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

/*
 * RelationInput.h
 *
 *  Created on: May 21, 2015
 *      Author: root
 */

#pragma once
#include "../Common/stdafx.h"
#include "../Internal/Element/Element.h"
#include "../IO/IStreamInput.h"
#include "../Schema/JsonSchema.h"
#include <boost/asio.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include "../Internal/Synopsis/WindowSynopsis.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <stdexcept>
#include <string>

//#include <occi.h>

using namespace std;

class RelationInput {

private:
	std::string ip;
	std::string port;
	std::string userName;
	std::string userPassword;
	std::string databaseName;
	std::string tableName;

	boost::shared_ptr<JsonSchema> schema;

	std::string id;

public:
	RelationInput();
	RelationInput(std::string ip, std::string port, std::string userName,
			std::string userPassword, std::string databaseName,
			std::string tableName, boost::shared_ptr<JsonSchema> schema);
	virtual ~RelationInput();

	list<Element> getAllElementsFromDatabase();

	const std::string& getId() const {
		return id;
	}

	const std::string& getDatabaseName() const {
		return databaseName;
	}

	void setDatabaseName(const std::string& databaseName) {
		this->databaseName = databaseName;
	}

	void setId(const std::string& id) {
		this->id = id;
	}

	const std::string& getIp() const {
		return ip;
	}

	void setIp(const std::string& ip) {
		this->ip = ip;
	}

	const std::string& getPort() const {
		return port;
	}

	void setPort(const std::string& port) {
		this->port = port;
	}

	const boost::shared_ptr<JsonSchema>& getSchema() const {
		return schema;
	}

	void setSchema(const boost::shared_ptr<JsonSchema>& schema) {
		this->schema = schema;
	}

	const std::string& getUserName() const {
		return userName;
	}

	void setUserName(const std::string& userName) {
		this->userName = userName;
	}

	const std::string& getUserPassword() const {
		return userPassword;
	}

	void setUserPassword(const std::string& userPassword) {
		this->userPassword = userPassword;
	}
};

