//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../BinaryJson/BinaryJsonObject.h"
#include "../BinaryJson/BinaryJsonElement.h"
#include "../BinaryJson/StringData.h"

class BinaryJsonBuilderBase
{
public:
public:
	virtual ~BinaryJsonBuilderBase() {}

	virtual BinaryJsonObject obj() = 0;  // get the document


	virtual BinaryJsonBuilderBase& append( const BinaryJsonElement& e) = 0;

	virtual BinaryJsonBuilderBase& append( const StringData&  fieldName , int n ) = 0;

	virtual BinaryJsonBuilderBase& append( const StringData&  fieldName , long long n ) = 0;

	virtual BinaryJsonBuilderBase& append( const StringData&  fieldName , double n ) = 0;

	virtual BinaryJsonBuilderBase& appendArray( const StringData&  fieldName , const BinaryJsonObject& subObj ) = 0;

	virtual BinaryJsonBuilderBase& appendAs( const BinaryJsonElement& e , const StringData&  filedName ) = 0;

	virtual void appendNull( ) = 0;

	virtual BinaryJsonBuilderBase& operator<<( const BinaryJsonElement& e ) = 0;

	virtual bool isArray() const = 0;
};

