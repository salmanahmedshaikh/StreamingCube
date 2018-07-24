//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Utility/TimestampGenerator.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include "../Common/Types.h"

boost::posix_time::ptime TimestampGenerator::start_time ( boost::gregorian::date(1970,1,1));
boost::mutex TimestampGenerator::m_oMutex;
TimestampGenerator::TimestampGenerator()
{

}
TimestampGenerator::~TimestampGenerator()
{
}


