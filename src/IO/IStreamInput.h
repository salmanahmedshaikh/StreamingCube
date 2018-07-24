//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../Internal/Element/Element.h"
#include "../Schema/JsonSchema.h"
#include <boost/asio.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/shared_ptr.hpp>
/*
 * Any wrapper of information source should implement this interface
 */
class IStreamInput
{
private:
	bool isActive;
	std::string id;
	boost::shared_ptr<JsonSchema>jsonSchema;

	virtual void getNextElement(Element& element) = 0;
	virtual void initial() = 0;

public:
	IStreamInput(boost::shared_ptr<JsonSchema>jsonSchema);
	IStreamInput(void);

	bool getAndCheckNextElement(Element& element);

	void start();
	std::string getId(void);
	boost::shared_ptr<JsonSchema>getSchema(void);

	virtual ~IStreamInput(void);
	virtual bool isEmpty() = 0;
};

