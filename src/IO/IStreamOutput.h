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
#include <boost/asio.hpp>
#include <boost/asio/streambuf.hpp>
/*
 * Any implementaion of stream output should implement this interface
 */
class IStreamOutput
{
private:
	std::string id;
	int streamOutputQueryID;

public:
	IStreamOutput(void);
	virtual ~IStreamOutput(void);
	//push one element to the output
	virtual void pushNext(Element& element) = 0;
	//check if the output buffer is full, if not , pushNext could be called
	virtual bool isFull(void) = 0;
	//initial the implementation of the IStreamOutput class
	virtual void initial(void) = 0;

	void setId(std::string Id);
	std::string getId(void);

	void setStreamOutputQueryID(int);
	int getStreamOutputQueryID(void);

};

