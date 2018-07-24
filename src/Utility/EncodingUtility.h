//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
class EncodingUtility
{
private:
	static std::string currentSystemStringEncoding;//unicode
public:
	EncodingUtility(void);
	~EncodingUtility(void);
	static std::string convertEncoding(std::string const& text, std::string const& to_encoding, std::string const& from_encoding);

	static std::string getCurrentSystemStringEncoding();
};

