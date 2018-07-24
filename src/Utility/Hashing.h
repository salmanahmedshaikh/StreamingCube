//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
class Hashing
{
private:
    static Hashing* hashing;

public:

	Hashing(void);
	~Hashing(void);
	static Hashing* getInstance();

	//int twoFoldStringHashing(std::string groupByAttribute, int totalActiveWorkers);
	int stringHashing(const char* groupByAttribute, int totalActiveWorkers);
};

