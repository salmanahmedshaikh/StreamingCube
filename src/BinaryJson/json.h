//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "../Common/stdafx.h"
#include <string>
#include "../BinaryJson/BinaryJsonObject.h"

/* build binary JSON object by JSON character string*/
BinaryJsonObject fromjson(const std::string &str);

/** len will be size of JSON object in text chars. */
BinaryJsonObject fromjson(const char *str, int* len=NULL);
