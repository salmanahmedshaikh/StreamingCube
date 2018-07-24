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
#include "../Internal/Synopsis/WindowSynopsis.h"
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>


// use can specify the input rate of the stream
class SpecifiedInputRateStreamInput: public IStreamInput
{
private:
	boost::shared_ptr<JsonSchema> schema;
	void getNextElement(Element& element);
	//boost::shared_ptr<WindowSynopsis> windowSynopsis;
	//boost::mutex m_oMutex;
	Document bufferedDocument;
	boost::recursive_mutex m_oMutex;
public:
	SpecifiedInputRateStreamInput(boost::shared_ptr<JsonSchema> schema);
	~SpecifiedInputRateStreamInput(void);
	bool isEmpty();
	void initial();
	static int ignoredInputNumber;
	int bufferedElementNumber;
	int totalElementNumber;
	void generateInput(int sleepMiliSeconds );
};

