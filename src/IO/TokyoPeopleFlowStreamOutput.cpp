
//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////
#include "../Common/stdafx.h"
#include "../IO/TokyoPeopleFlowStreamOutput.h"


TokyoPeopleFlowStreamOutput::TokyoPeopleFlowStreamOutput(void)
{
}


TokyoPeopleFlowStreamOutput::~TokyoPeopleFlowStreamOutput(void)
{
}

void TokyoPeopleFlowStreamOutput::pushNext(Element& element)
{

	//wangyan
	//std::cout<<"________system output start__________"<<std::endl;
	std::cout<<element<<std::endl;
	//std::cout<<"________system output finish_________"<<std::endl;
}

bool TokyoPeopleFlowStreamOutput::isFull(void)
{
	return false;
}

void TokyoPeopleFlowStreamOutput::initial(void)
{
}
