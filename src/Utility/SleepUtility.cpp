//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Utility/SleepUtility.h"
#include <iostream>
#include<unistd.h>
#include<sys/time.h>
SleepUtility::SleepUtility(void)
{
}


SleepUtility::~SleepUtility(void)
{
}
void SleepUtility::sleepMicroSecond(int microSecond)
{
	/*
	struct timespec slptm;
	slptm.tv_sec = 0;
	slptm.tv_nsec = 1000*microSecond;
	pselect(0, NULL, NULL, NULL, &slptm, NULL);
	*/

	usleep(microSecond);
}
