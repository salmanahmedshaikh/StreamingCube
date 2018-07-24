//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../Query/QueryIntermediateRepresentation.h"
#include <boost/shared_ptr.hpp>

class ParserManager
{
private:
	static  ParserManager * parserManager;
	ParserManager(void);

	static const int parserServerPort = 3000;
	void init();
	void connectServer();
public:
	static ParserManager * getInstance();
	~ParserManager(void);
	boost::shared_ptr<QueryIntermediateRepresentation>processQuery(std::string jaqlQueryString);
};

