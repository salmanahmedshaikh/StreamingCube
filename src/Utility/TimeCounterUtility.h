//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include <boost/noncopyable.hpp>
enum TimeCounterStatus
{
	initialStatus,
	runningStatus,
	pauseStatus,
};
class TimeCounterUtility:
	public boost::noncopyable
{
private:
	static TimeCounterUtility* timeCounterUtility;
	TimeCounterUtility(void);
#ifdef WINDOWS
	double totalTime;
	 double dff;
	 long long startTime ;
#else
	double totalTime;
	struct timeval startTime;
#endif
	 TimeCounterStatus timeCounterStatus;
public:
	~TimeCounterUtility(void);
	static TimeCounterUtility* getInstance();
	 void reset();
	 void start();
	 void pause();
	 void stop(double& time);
};

