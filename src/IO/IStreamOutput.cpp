//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../IO/IStreamOutput.h"


IStreamOutput::IStreamOutput(void)
{
}

IStreamOutput::~IStreamOutput(void)
{
}

void IStreamOutput::setId(std::string Id)
{
	this->id = id;
}
std::string IStreamOutput::getId(void)
{
	return this->id;
}

void IStreamOutput::setStreamOutputQueryID(int streamOutputQueryID)
{
	this->streamOutputQueryID = streamOutputQueryID;
}

int IStreamOutput::getStreamOutputQueryID(void)
{
	return this->streamOutputQueryID;
}

