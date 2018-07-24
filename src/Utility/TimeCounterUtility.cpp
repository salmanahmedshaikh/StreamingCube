//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Utility/TimeCounterUtility.h"
#ifdef WINDOWS
#include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

TimeCounterUtility* TimeCounterUtility::timeCounterUtility = NULL;

TimeCounterUtility::TimeCounterUtility(void)
{
#ifdef WINDOWS
	reset();
	LARGE_INTEGER  large_interger;
	QueryPerformanceFrequency(&large_interger);
	dff = large_interger.QuadPart;
#else
	reset();

#endif
}


TimeCounterUtility::~TimeCounterUtility(void)
{
}
 TimeCounterUtility* TimeCounterUtility::getInstance()
 {
	 if(timeCounterUtility==NULL)
	 {
		 timeCounterUtility = new TimeCounterUtility();
	 }
	 return timeCounterUtility;
 }
 void TimeCounterUtility::reset()
 {
#ifdef WINDOWS
	 totalTime = 0;
	 timeCounterStatus = initialStatus;
#else
	 totalTime = 0;
	 timeCounterStatus = initialStatus;
#endif

 }
 void TimeCounterUtility::start()
 {
#ifdef WINDOWS
	 assert(timeCounterStatus == initialStatus||timeCounterStatus==pauseStatus);
	 LARGE_INTEGER  large_interger;
	 QueryPerformanceCounter(&large_interger);
	 startTime = large_interger.QuadPart;
	 timeCounterStatus = runningStatus;
#else

	 assert(timeCounterStatus == initialStatus||timeCounterStatus==pauseStatus);
	 gettimeofday(&startTime,NULL);
	 timeCounterStatus = runningStatus;
#endif
 }
 void TimeCounterUtility::pause()
 {
#ifdef WINDOWS
	 assert(timeCounterStatus == runningStatus||timeCounterStatus == pauseStatus);
	 if(timeCounterStatus == runningStatus)
	 {
		 LARGE_INTEGER  large_interger;
		 QueryPerformanceCounter(&large_interger);
		 long long endTime;
		 endTime = large_interger.QuadPart;
		 double time=(endTime - startTime)  / dff;
		 totalTime += time;
		// std::cout<<totalTime<<std::endl;
		 timeCounterStatus = pauseStatus;
	 }
#else

	 assert(timeCounterStatus == runningStatus||timeCounterStatus == pauseStatus);
	 if(timeCounterStatus == runningStatus)
	 {
		 struct timeval endTime;
		 gettimeofday(&endTime,NULL);
		 double time = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec)/1000000.0;
		 totalTime += time;
//		 std::cout<<totalTime<<std::endl;
		 timeCounterStatus = pauseStatus;
	 }
#endif
 }

 void TimeCounterUtility::stop(double& time)
 {
#ifdef WINDOWS
	 assert(timeCounterStatus == pauseStatus||timeCounterStatus == runningStatus);
	 if(timeCounterStatus==runningStatus)
	 {
		 LARGE_INTEGER  large_interger;
		 QueryPerformanceCounter(&large_interger);
		 long long endTime;
		 endTime = large_interger.QuadPart;
		 double time=(endTime - startTime)  / dff;
		 totalTime += time;
	 }
	 time = totalTime;

	 reset();
#else

	 assert(timeCounterStatus == pauseStatus||timeCounterStatus == runningStatus);
	 if(timeCounterStatus==runningStatus)
	 {
		 struct timeval endTime;
		 gettimeofday(&endTime,NULL);
		 double time = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec)/1000000.0;
		 totalTime += time;
		 //std::cout<<totalTime<<std::endl;
		 timeCounterStatus = pauseStatus;
	 }
	 time = totalTime;
	 reset();
#endif

 }
