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
#include <cstdlib>
#include <boost/noncopyable.hpp>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
//#include "../IO/IOManager.h"




class DispatcherManager:private boost::noncopyable
{
    private:
        static DispatcherManager* dispatcherManager;
        DispatcherManager(void);
        void initialize(void);


    public:
        static DispatcherManager* getInstance(void);
        ~DispatcherManager(void);
        void execute(std::map<std::string, int> workerIPIDMap, std::vector<std::string> dispatcherQueryVec);
};
