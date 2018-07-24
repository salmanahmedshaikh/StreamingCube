//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Utility/EncodingUtility.h"
//#include <boost/locale/encoding.hpp>

std::string EncodingUtility::currentSystemStringEncoding = "GB2312";
EncodingUtility::EncodingUtility(void)
{
}


EncodingUtility::~EncodingUtility(void)
{
}


std::string EncodingUtility::getCurrentSystemStringEncoding()
{
	return currentSystemStringEncoding;
}

std::string EncodingUtility::convertEncoding(std::string const& text, std::string const& to_encoding, std::string const& from_encoding)
{
	//return boost::locale::conv::between(text,to_encoding,from_encoding);
	//to do ...
	//wang yan
	return text;
}
