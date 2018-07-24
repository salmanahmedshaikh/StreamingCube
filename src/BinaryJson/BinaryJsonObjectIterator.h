//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../BinaryJson/inline_decls.h"
#include "../BinaryJson/BinaryJsonObject.h"
#include "../BinaryJson/BinaryJsonElement.h"
class BinaryJsonObjectIterator
{
private:
	const char* _pos;
	const char* _theend;
public:
	BinaryJsonObjectIterator(const BinaryJsonObject& jso) {
		int sz = jso.objsize();
		if ( MONGO_unlikely(sz == 0) ) {
			_pos = _theend = 0;
			return;
		}
		_pos = jso.objdata() + 4;
		_theend = jso.objdata() + sz - 1;
	}

	BinaryJsonObjectIterator( const char * start , const char * end ) {
		_pos = start + 4;
		_theend = end - 1;
	}
	/** @return true if more elements exist to be enumerated. */
	bool more() { return _pos < _theend; }

	/** @return true if more elements exist to be enumerated INCLUDING the EOO element which is always at the end. */
	bool moreWithEOO() { return _pos <= _theend; }


	BinaryJsonElement next() {
		assert( _pos <= _theend );
		BinaryJsonElement e(_pos);
		_pos += e.size();
		return e;
	}



	BinaryJsonElement next( bool checkEnd ) {
		assert( _pos <= _theend );
		BinaryJsonElement e( _pos, checkEnd ? (int)(_theend + 1 - _pos) : -1 );
		_pos += e.size( checkEnd ? (int)(_theend + 1 - _pos) : -1 );
		return e;
	}

	void operator++() { next(); }
	void operator++(int) { next(); }

	BinaryJsonElement operator*() {
		assert( _pos <= _theend );
		return BinaryJsonElement(_pos);
	}
};

