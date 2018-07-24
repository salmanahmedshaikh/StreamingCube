//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Operator/CollectStream.h"
#include "../Utility/TimeCounterUtility.h"
#include "../Scheduler/Scheduler.h"
#include "../Utility/TimestampGenerator.h"
#include "../Command/CommandManager.h"
#include "../Utility/MyStatus.h"
#include "../Utility/StreamDistribution.h"
#include "../BinaryJson/json.h"
#include "../Query/QueryIntermediateRepresentation.h"
#include "../Configure/ConfigureManager.h"
#include "../Server/JsonStreamServer.h"
#include "../IO/IOManager.h"
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>

CollectStream::CollectStream(void)
{
}

CollectStream::~CollectStream(void)
{
}

void CollectStream::execute()
{
}

void CollectStream::processElement(Element& element)
{
    //std::cout<<element.document.getField("sex").valuestr();
    //std::cout<<element<<std::endl;
}

