//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include "../Common/Types.h"
class QueryAttribute {
private:
	std::vector<boost::any> idNameVector;
	boost::any getValue(Document& document, boost::any& idName) {
		if (idName.type() == typeid(std::string)) {
			DocumentElement documentElement = document[boost::any_cast<
					std::string>(idName)];
			if (documentElement.isABSONObj()) {
				return documentElement.embeddedObject();
			} else if (documentElement.isBoolean()) {
				return documentElement.boolean();
			} else if (documentElement.isNull()) {
				return NULL;
			} else if (documentElement.isInteger()) {
				return documentElement.numberInt();
			} else if (documentElement.isLong()) {
				return documentElement.numberLong();
			} else if (documentElement.isDouble()) {
				return documentElement.numberDouble();
			} else if (documentElement.isStringType()) {
				return (std::string) documentElement.valuestr();
			}
		} else if (idName.type() == typeid(int)) {
			DocumentElement documentElement = document[boost::any_cast<int>(
					idName)];
			if (documentElement.isABSONObj()) {
				return documentElement.embeddedObject();
			} else if (documentElement.isBoolean()) {
				return documentElement.boolean();
			} else if (documentElement.isNull()) {
				return NULL;
			} else if (documentElement.isInteger()) {
				return documentElement.numberInt();
			} else if (documentElement.isLong()) {
				return documentElement.numberLong();
			} else if (documentElement.isDouble()) {
				return documentElement.numberDouble();
			} else if (documentElement.isStringType()) {
				return (std::string) documentElement.valuestr();
			}
		} else if (idName.type() == typeid(vector<int> )) {
			//to do wangyan
			//append the int first, the calculate? dongchao todo
		} else {
			assert(false);
		}
	}

public:
	QueryAttribute(void);
	~QueryAttribute(void);

	void setIdNameVector(std::vector<boost::any> idNameVector) {
		this->idNameVector = idNameVector;
	}
	std::string getLastAttribute(void) {
		boost::any back = this->idNameVector.back();
		assert(back.type() == typeid(std::string));
		std::string ret(boost::any_cast<std::string>(back));
		return ret;
	}

	boost::any getValue(Document& document) {
		std::vector<boost::any>::iterator it;
		boost::any ret = document;
		for (it = idNameVector.begin(); it != idNameVector.end(); it++) {
			boost::any idName = *it;
			Document doc = boost::any_cast<Document>(ret);
			ret = getValue(doc, idName);
		}
		return ret;
	}

	boost::any getLastAttributeAny(void) {
		return this->idNameVector.back();
	}

	Document getValueDoc(Document& document) {
		std::vector<boost::any>::iterator it;
		boost::any value;
		DocumentBuilder documentBuilder;
		for (it = idNameVector.begin(); it != idNameVector.end(); it++) {
			boost::any idName = *it;
			value = getValue(document, idName);
			string str = boost::any_cast<std::string>(idName);
			documentBuilder.append(document.getField(str));

		}
		return documentBuilder.obj();
	}

	//dongchao
	//for one idName?
	boost::any getValueDirectByIdName(Document& document, boost::any& idName) {
		std::vector<boost::any>::iterator it;
		boost::any ret = document;

		Document doc = boost::any_cast<Document>(ret);
		ret = getValue(doc, idName);
		return ret;
	}
	;
};

