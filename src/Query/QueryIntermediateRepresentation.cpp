//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Query/QueryIntermediateRepresentation.h"
#include "../Common/Types.h"

QueryIntermediateRepresentation::QueryIntermediateRepresentation(Document intermediateRepDocument)
{
	this->intermediateRepDocument = intermediateRepDocument;
}


QueryIntermediateRepresentation::~QueryIntermediateRepresentation(void)
{
}

Document& QueryIntermediateRepresentation::getDocument()
{
	return this->intermediateRepDocument;
}
