//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../IO/IStreamInput.h"

class TokyoPeopleFlowStreamInputShort:
	public IStreamInput
{
private:
	std::vector<std::string> dataFolderVector;// the folders containing the tokyo people flow data
	std::list<std::string> peopleFlowFileList;// the list containing all the file names in the specified folders
	std::list<std::string>::iterator currentPeopleFlowFileListIterator; // the iterator used to scan the file list
	std::ifstream currentFileStream; // the current file
	void getNextElement(Element& element);
	bool isFirstFileElementSent;
	bool canReadStreamTupels;
	int streamArrivalRate;
	int tokyoPeopleTuplesCounter;
	struct timeval currentTime,stoppingTime;

	//boost::shared_ptr<char> fileBuffer; //(new char[FILEBUFF]);
	//char* fileBuffer; //=new char[size];

public:
	TokyoPeopleFlowStreamInputShort(std::vector<std::string> dataFolderVector,boost::shared_ptr<JsonSchema>jsonSchema);
	void createCommandElement(Element& element, int peopleFlowFileNameInt, std::string elementType);
	int getIntFileName(std::string peopleFlowFile);
	~TokyoPeopleFlowStreamInputShort(void);
	bool isEmpty() ;
	void initial() ;
};

