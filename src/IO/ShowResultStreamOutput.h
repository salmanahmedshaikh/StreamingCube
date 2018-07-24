//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../IO/IStreamOutput.h"
class ShowResultStreamOutput : // show the query output element to the standard output by cout<<
	public IStreamOutput
{
public:
	ShowResultStreamOutput(void);
	~ShowResultStreamOutput(void);
	static int totalOutputElementsCounter;
	int getTotalOutputElementsCount(void);
	void pushNext(Element& element);
	bool isFull(void) ;
	void initial(void) ;
};

