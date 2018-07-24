//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../Common/Types.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
/* generate the timestamp
   accuracy: microsec
*/
class TimestampGenerator:private boost::noncopyable
{
private:
	static boost::posix_time::ptime start_time;

	TimestampGenerator();
	~TimestampGenerator();
	static boost::mutex m_oMutex;

public:
	inline static Timestamp getCurrentTime()
	{
	//boost::mutex::scoped_lock lock(m_oMutex);
//	boost::posix_time::ptime currentTime;
//	currentTime = boost::posix_time::microsec_clock::local_time();
//	boost::posix_time::time_duration time_elapse;
//	time_elapse = currentTime - start_time;
//	return time_elapse.total_microseconds();

//	long long time;
 	struct timeval tv;
    	gettimeofday(&tv,NULL);
    	return  (long long)tv.tv_sec * 1000 * 1000 + (long long)tv.tv_usec;
    	}

};
