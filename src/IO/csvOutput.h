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

class csvOutput : // show the query output element to the standard output by cout<<
	public IStreamOutput
{
public:
	csvOutput(std::string csvOutputFilePath);
	~csvOutput(void);
	static int totalOutputElementsCounter;
	int getTotalOutputElementsCount(void);
	void pushNext(Element& element);
	bool isFull(void) ;
	void initial(void) ;
	void openFile(int csvFileName);
	void closeFile(void);

private:
    std::ofstream csvFile;
    std::string csvFilePath;
    static int outputFileName;
};

