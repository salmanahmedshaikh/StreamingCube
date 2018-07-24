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
#include <boost/noncopyable.hpp>
/*
 generate identity for query
 currently the id is an incrementally added integer
*/
class QueryIdentityGenerator
{
private:
	static DocumentId queryIdentity;

	QueryIdentityGenerator(void);
	~QueryIdentityGenerator(void);

public:

	static DocumentId generateNewQueryIdentity(void);

};

