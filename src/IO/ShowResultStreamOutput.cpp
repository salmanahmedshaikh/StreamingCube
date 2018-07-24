//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../IO/ShowResultStreamOutput.h"
#include "../Operator/RootOperator.h"

int ShowResultStreamOutput::totalOutputElementsCounter = 0;

ShowResultStreamOutput::ShowResultStreamOutput(void)
{
}

ShowResultStreamOutput::~ShowResultStreamOutput(void)
{
}

void ShowResultStreamOutput::pushNext(Element& element)
{

	//wangyan
	//std::cout<<"________system output start__________"<<std::endl;
	//std::cout << "Output of QueryID from showResultStreamOutput: " << getStreamOutputQueryID() << std::endl;
	totalOutputElementsCounter++;
	//std::cout << "totalOutputElementsCounter" << totalOutputElementsCounter << std::endl;
	std::cout<< "Output" << std::endl << element<<std::endl;
	//sleep(1);
	//std::cout<<"________system output finish_________"<<std::endl;
}

bool ShowResultStreamOutput::isFull(void)
{
	return false;
}
void ShowResultStreamOutput::initial(void)
{
}

int ShowResultStreamOutput::getTotalOutputElementsCount(void)
{
    return totalOutputElementsCounter;
}
