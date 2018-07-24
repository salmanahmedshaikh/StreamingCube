//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
enum BinaryJsonType {

	/** end of object */
	EOO=0,
	/** double precision floating point value */
	NumberDouble=1,
	/** character string, stored in utf8 */
	JSONString=2,
	/** an embedded object */
	Object=3,
	/** an embedded array */
	JSONArray=4,
	/** boolean type */
	JSONBool=5,
	/** null type */
	jstNULL=6,
	/** 32 bit signed integer */
	NumberInt = 7,
	/** 64 bit integer */
	NumberLong = 8,

};

/**
* returns the name of the argument's type
* defined in jsobj.cpp
*/
const char* typeName (BinaryJsonType type);
