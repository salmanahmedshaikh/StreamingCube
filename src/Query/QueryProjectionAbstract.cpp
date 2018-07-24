//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
//
//QueryProjection::QueryProjection(Document& document){
//	std::string type = document.getField(QUERY_PROJECTION_TYPE).String();
//	if(type.compare("object") == 0) this->projectionType = _OBJECT;
//	else if(type.compare("array") == 0) this->projectionType = _ARRAY;
//	else if(type.compare("direct") == 0) this->projectionType = _DIRECT;
//	else assert(false);
//
//	if(! document.getField(QUERY_NEED_RENAME).isNull()){
//		this->needRename = document.getField(QUERY_NEED_RENAME).Bool();
//		if(this->needRename)
//			this->rename = document.getField(QUERY_RENAME).String();
//	}
//
//	switch(this->projectionType){
//	case _OBJECT:
//		{
//			std::vector<DocumentElement> vec = document.getField(QUERY_FIELDS).Array();
//			for(int i=0;i<vec.size();i++)
//				this->fields.push_back(boost::shared_ptr<QueryProjection>(new QueryProjection(vec[i].embeddedObject())));
//
//			break;
//		}
//	case _ARRAY:
//		{
//			std::vector<DocumentElement> vec = document.getField(QUERY_ARRAY_ITEMS).Array();
//			for(int i=0;i<vec.size();i++)
//			//	this->arrayItems.push_back(boost::shared_ptr<QueryProjection>(new QueryProjection(vec[i].embeddedObject())));
//
//			break;
//		}
//	case _DIRECT:
//		//this->expression = boost::shared_ptr<QueryExpression>(new QueryExpression(document.getField(QUERY_EXPRESSION).embeddedObject()));
//		break;
//	}
//}
//
//QueryProjection::~QueryProjection(){}
