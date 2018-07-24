//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <assert.h>
#include <list>
#include <map>
#include <algorithm>
#include <strstream>
#include <iostream>
#include <fstream>
#include <queue>
//#define DEBUG  // the system will run in debug mode

//#define LOG    // the output will be write into a file
//#define SMART //the system will turns into smart mode, or will turns into naive mode.
#define SHARE_NO_OPERATOR // The system will not share any operator even if SHARE_RANGE_WINDOW and SHARE_SMART_WINDOW are defined
//#define SHARE_RANGE_WINDOW // The system shares range/time window and other operators except the smart window
//#define SHARE_SMART_WINDOW // The system shares smart/row window and other operators except the range/time window

//#define TOKYO_PEOPLE_FLOW_DEMO
//#define PROFILER // the system will runs in a performance test mode, after the execution, the profiler file would be generated.
//#define CHECKINPUTSCHEMA // the system will checks the schema of each input tuples

#ifdef LOG
static std::ofstream logFile("server1_out.txt");
static std::streambuf *outbuf = std::cout.rdbuf(logFile.rdbuf());
#endif
