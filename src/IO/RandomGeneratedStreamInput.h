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
#include "../IO/IStreamInput.h"
#include "../Schema/JsonSchema.h"
class RandomGeneratedStreamInput: public IStreamInput
{
private:
	boost::shared_ptr<JsonSchema> schema;
	void getNextElement(Element& element);
	Document bufferedDocument;
public:
	RandomGeneratedStreamInput(boost::shared_ptr<JsonSchema> schema);
	~RandomGeneratedStreamInput(void);
	bool isEmpty();
	void initial();
	static int throughout;
};

