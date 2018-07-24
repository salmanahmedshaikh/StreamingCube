//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstring>
#include <string>
#include "../Common/stdafx.h"
using namespace std;

class StringData {
public:
	/** Construct a StringData, for the case where the length of
	* string is not known. 'c' must be a pointer to a null-terminated string.
	*/
	StringData( const char* c )
		: _data(c), _size(string::npos){}

	/** Construct a StringData explicitly, for the case where the length of the string
	* is already known. 'c' must be a pointer to a null-terminated string, and strlenOfc
	* must be the length that strlen(c) would return, a.k.a the index of the
	* terminator in c.
	*/
	StringData( const char* c, size_t len )
		: _data(c), _size(len) {}

	/** Construct a StringData, for the case of a string. */
	StringData( const std::string& s )
		: _data(s.c_str()), _size(s.size()) {}

	// Construct a StringData explicitly, for the case of a literal whose size is
	// known at compile time.
	struct LiteralTag {};
	template<size_t N>
	StringData( const char (&val)[N], LiteralTag )
		: _data(&val[0]), _size(N-1) {}

	// accessors
	const char* data() const { return _data; }
	size_t size() const {
		if ( _size == string::npos )
			_size = strlen( _data );
		return _size;
	}

private:
	const char* const _data;  // is always null terminated
	mutable size_t _size;  // 'size' does not include the null terminator
};
