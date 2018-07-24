//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Common/Types.h"
JSONTYPE generateJsonType(std::string type)
{
	JSONTYPE jsonType;
	if(type=="string")
	{
		jsonType = JSON_STRING;
	}
	else if (type == "number")
	{
		jsonType = JSON_NUMBER;
	}
	else if (type == "int")
	{
		jsonType = JSON_INT;
	}
	else if (type == "float")
	{
		jsonType = JSON_FLOAT;
	}
	/*
	else if (type == "double")
	{
		jsonType = JSON_double;
	}
	*/
	else if (type == "object")
	{
		jsonType = JSON_OBJECT;
	}
	else if (type == "array")
	{
		jsonType = JSON_ARRAY;
	}
	else if (type == "true")
	{
		jsonType = JSON_TRUE;
	}
	else if (type == "false")
	{
		jsonType = JSON_FALSE;
	}
	else if (type == "null")
	{
		jsonType = JSON_NULL;
	}
	else
	{
		throw runtime_error("no such json types");
	}
	return jsonType;
}
