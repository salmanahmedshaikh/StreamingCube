//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../BinaryJson/BinaryJsonTypes.h"
/* take a BSONType and return the name of that type as a char* */
const char* typeName (BinaryJsonType type) {
	switch (type) {

	case EOO: return "EOO";
	case NumberDouble: return "NumberDouble";
	case JSONString: return "String";
	case Object: return "Object";
	case JSONArray: return "Array";
	case JSONBool: return "Bool";
	case jstNULL: return "NULL";
	case NumberInt: return "NumberInt32";
	case NumberLong: return "NumberLong64";
	default: return "Invalid";

	}
}
