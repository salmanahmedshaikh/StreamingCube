//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma  once
#include "../Common/stdafx.h"
#include <limits>
#include <boost/lexical_cast.hpp>
//////////////BinaryJsonElement///////////////////////////
inline BinaryJsonObject BinaryJsonElement::Obj() const
{
	return embeddedObjectUserCheck();
}


inline BinaryJsonObject BinaryJsonElement::embeddedObject() const {
	assert( isABSONObj() );
	return BinaryJsonObject(value());


}
inline BinaryJsonObject BinaryJsonElement::embeddedObjectUserCheck() const {
	if ( isABSONObj() )
		return BinaryJsonObject(value());
	std::stringstream ss;
	ss << "invalid parameter: expected an object (" << fieldName() << ")";
	throw runtime_error( ss.str() );
	return BinaryJsonObject(); // never reachable
}



inline BinaryJsonElement BinaryJsonElement::operator[] (const std::string& field) const {
	BinaryJsonObject o = Obj();
	return o[field];
}


inline int BinaryJsonElement::canonicalType() const {
	BinaryJsonType t = type();
	switch ( t ) {

	case EOO:

		return 0;
	case jstNULL:
		return 5;
	case NumberDouble:
	case NumberInt:
	case NumberLong:
		return 10;
	case JSONString:
		return 15;
	case Object:
		return 20;
	case JSONArray:
		return 25;

	case JSONBool:
		return 40;
	default:
		assert(0);
		return -1;
	}
}


/** Like numberLong() but with well-defined behavior for doubles that
*  are NaNs, or too large/small to be represented as long longs.
*  NaNs -> 0
*  very large doubles -> LLONG_MAX
*  very small doubles -> LLONG_MIN  */
inline long long BinaryJsonElement::safeNumberLong() const {
	double d;
	switch( type() ) {
	case NumberDouble:
		d = numberDouble();
		if ( d!=d ){ //wangyan
			return 0;
		}
		if ( d > (double) (std::numeric_limits<long long>::max)() ){
			return (std::numeric_limits<long long>::max)();
		}
		if ( d < (std::numeric_limits<long long>::min)() ){
			return (std::numeric_limits<long long>::min)();
		}
	default:
		return numberLong();
	}
}
inline	std::vector<BinaryJsonElement> BinaryJsonElement::Array() const // see implementation for detailed comments
{
	assert(isArrayType());
	BinaryJsonObject object = embeddedObject();
	std::vector<BinaryJsonElement> binaryJsonElementVector;

	BinaryJsonObjectIterator i(object);
	while ( i.more() )
	{
		BinaryJsonElement e = i.next();
		binaryJsonElementVector.push_back(e);
	}
	return binaryJsonElementVector;



}
inline std::string BinaryJsonElement::toString( bool includeFieldName, bool full ) const {
	StringBuilder s;
	toString(s, includeFieldName, full);
	return s.str();
}
inline void BinaryJsonElement::toString( StringBuilder& s, bool includeFieldName, bool full, int depth ) const {

	if ( depth > BinaryJsonObject::maxToStringRecursionDepth ) {
		// check if we want the full/complete string
		if ( full ) {
			StringBuilder s;
			s << "Reached maximum recursion depth of ";
			s << BinaryJsonObject::maxToStringRecursionDepth;
			//wangyan uassert(16150, s.str(), full != true);
		}
		s << "...";
		return;
	}

	if ( includeFieldName && type() != EOO )
		s << fieldName() << ": ";
	switch ( type() ) {
	case EOO:
		s << "EOO";
		break;

	case NumberDouble:
		s.appendDoubleNice( number() );
		break;
	case NumberLong:
		s << _numberLong();
		break;
	case NumberInt:
		s << _numberInt();
		break;
	case JSONBool:
		s << ( boolean() ? "true" : "false" );
		break;
	case Object:
		embeddedObject().toString(s, false, full, depth+1);
		break;
	case JSONArray:
		embeddedObject().toString(s, true, full, depth+1);
		break;

	case jstNULL:
		s << "null";
		break;


	case JSONString:
		s << '"';
		if ( !full &&  valuestrsize() > 160 ) {
			s.write(valuestr(), 150);
			s << "...\"";
		}
		else {
			s.write(valuestr(), valuestrsize()-1);
			s << '"';
		}
		break;



	default:
		s << "?type=" << type();
		break;
	}
}





/** Retrieve int value for the element safely.  Zero returned if not a number. Converted to int if another numeric type. */
inline int BinaryJsonElement::numberInt() const {
	switch( type() ) {
	case NumberDouble:
		return (int) _numberDouble();
	case NumberInt:
		return _numberInt();
	case NumberLong:
		return (int) _numberLong();
	default:
		return 0;
	}
}

/** Retrieve long value for the element safely.  Zero returned if not a number. */
inline long long BinaryJsonElement::numberLong() const {
	switch( type() ) {
	case NumberDouble:
		return (long long) _numberDouble();
	case NumberInt:
		return _numberInt();
	case NumberLong:
		return _numberLong();
	default:
		return 0;
	}
}




// wrap this element up as a singleton object.
inline BinaryJsonObject BinaryJsonElement::wrap() const {
	BinaryJsonObjectBuilder b(size()+6);
	b.append(*this);
	return b.obj();
}

inline BinaryJsonObject BinaryJsonElement::wrap( const char * newName ) const {
	BinaryJsonObjectBuilder b(size()+6+(int)strlen(newName));
	b.appendAs(*this,newName);
	return b.obj();
}

inline bool BinaryJsonElement::trueValue() const {
	// NOTE Behavior changes must be replicated in Value::coerceToBool().
	switch( type() ) {
	case NumberLong:
		return *reinterpret_cast< const long long* >( value() ) != 0;
	case NumberDouble:
		return (reinterpret_cast < const double* >(value ())) != 0;
	case NumberInt:
		return *reinterpret_cast< const int* >( value() ) != 0;
	case JSONBool:
		return boolean();
	case EOO:
	case jstNULL:

		return false;

	default:
		;
	}
	return true;
}


/** @return true if element is of a numeric type. */
inline bool BinaryJsonElement::isNumber() const {
	switch( type() ) {
	case NumberLong:
	case NumberDouble:
	case NumberInt:
		return true;
	default:
		return false;
	}
}
inline bool BinaryJsonElement::isInteger() const {
	switch( type() ) {
	case NumberInt:
		return true;
	default:
		return false;
	}
}
inline bool BinaryJsonElement::isLong() const {
	switch( type() ) {
	case NumberLong:
		return true;
	default:
		return false;
	}
}
inline bool BinaryJsonElement::isDouble() const {
	switch( type() ) {
	case NumberDouble:
		return true;
	default:
		return false;
	}
}

inline  bool BinaryJsonElement::isSimpleType() const {
	switch( type() ) {
	case NumberLong:
	case NumberDouble:
	case NumberInt:
	case JSONString:
	case JSONBool:
		return true;
	default:
		return false;
	}
}
inline void BinaryJsonElement::validate() const {
	const BinaryJsonType t = type();

	switch( t ) {

	case JSONString: {
		unsigned x = (unsigned) valuestrsize();
		bool lenOk = x > 0 && x < (unsigned) BSONObjMaxInternalSize;
		if( lenOk && valuestr()[x-1] == 0 )
			return;
		StringBuilder buf;
		buf <<  "Invalid dbref/code/string/symbol size: " << x;
		if( lenOk )
			buf << " strnlen:"<<x; //<< mongo::strnlen( valuestr() , x );//wangyan
		//wangyan  msgasserted( 10321 , buf.str() );
		break;
					 }

	case Object:
		// We expect Object size validation to be handled elsewhere.
	default:
		break;
	}
}

inline int BinaryJsonElement::size( int maxLen ) const {
	if ( totalSize >= 0 )
		return totalSize;

	int remain = maxLen - fieldNameSize() - 1;

	int x = 0;
	switch ( type() ) {
	case EOO:
	case jstNULL:

		break;
	case JSONBool: //wangyan
		x = 1;
		break;
	case NumberInt:
		x = 4;
		break;
	case NumberDouble:
	case NumberLong:
		x = 8;
		break;
	case JSONString: //wangyan
		//wangyan massert( 10313 ,  "Insufficient bytes to calculate element size", maxLen == -1 || remain > 3 );
		x = valuestrsize() + 4;
		break;

	case Object:
	case JSONArray: //wangyan
		//wangyan massert( 10316 ,  "Insufficient bytes to calculate element size", maxLen == -1 || remain > 3 );
		x = objsize();
		break;

	default: {
		StringBuilder ss;
		ss << "BSONElement: bad type " << (int) type();
		std::string msg = ss.str();
		//wangyan massert( 13655 , msg.c_str(),false);
			 }
	}
	totalSize =  x + fieldNameSize() + 1; // BSONType

	return totalSize;
}

inline int BinaryJsonElement::size(void) const {
	if ( totalSize >= 0 )
		return totalSize;

	int x = 0;
	switch ( type() ) {
	case EOO:

	case jstNULL:

		break;
	case JSONBool: //wangyan
		x = 1;
		break;
	case NumberInt:
		x = 4;
		break;

	case NumberDouble:
	case NumberLong:
		x = 8;
		break;

	case JSONString: //wangyan
		x = valuestrsize() + 4;
		break;

	case Object:
	case JSONArray: //wangyan
		x = objsize();
		break;


	default:
		{
			StringBuilder ss;
			ss << "BSONElement: bad type " << (int) type();
			std::string msg = ss.str();
			//wangyan massert(10320 , msg.c_str(),false);
		}
	}
	totalSize =  x + fieldNameSize() + 1; // BSONType

	return totalSize;
}



/////////////////////////BinaryJsonObject///////////////////


inline std::string BinaryJsonObject::toString( bool isArray, bool full ) const {
	if ( isEmpty() ) return "{}";
	StringBuilder s;
	toString(s, isArray, full);
	return s.str();
}
inline void BinaryJsonObject::toString( StringBuilder& s,  bool isArray, bool full, int depth ) const {
	if ( isEmpty() ) {
		s << "{}";
		return;
	}

	s << ( isArray ? "[ " : "{ " );
	BinaryJsonObjectIterator i(*this);
	bool first = true;
	while ( 1 ) {
		//wangyan massert( 10327 ,  "Object does not end with EOO", i.moreWithEOO() );
		BinaryJsonElement e = i.next( true );
		//wangyan massert( 10328 ,  "Invalid element size", e.size() > 0 );
		//wangyan massert( 10329 ,  "Element too large", e.size() < ( 1 << 30 ) );
		int offset = (int) (e.rawdata() - this->objdata());
		//wangyan massert( 10330 ,  "Element extends past end of object",
		//	 e.size() + offset <= this->objsize() );
		e.validate();
		bool end = ( e.size() + offset == this->objsize() );
		if ( e.eoo() ) {
			//wangyan massert( 10331 ,  "EOO Before end of object", end );
			break;
		}
		if ( first )
			first = false;
		else
			s << ", ";
		e.toString( s, !isArray, full, depth );
	}
	s << ( isArray ? " ]" : " }" );
}


inline std::ostream& operator<<( std::ostream &s, const BinaryJsonObject &o ) {
	return s << o.toString();
}

inline std::ostream& operator<<( std::ostream &s, const BinaryJsonElement &e ) {
	return s << e.toString();
}

inline StringBuilder& operator<<( StringBuilder &s, const BinaryJsonObject &o ) {
	o.toString( s );
	return s;
}
inline StringBuilder& operator<<( StringBuilder &s, const BinaryJsonElement &e ) {
	e.toString( s );
	return s;
}
/////////////////////////BinaryJsonObjectBuilder/////////////////////

inline BinaryJsonObjectBuilder& BinaryJsonObjectBuilder::appendElements(BinaryJsonObject x) {
	BinaryJsonObjectIterator it(x);
	while ( it.moreWithEOO() ) {
		BinaryJsonElement e = it.next();
		if ( e.eoo() ) break;
		append(e);
	}
	return *this;
}

inline BinaryJsonObjectBuilder& BinaryJsonObjectBuilder::appendElementsUnique(BinaryJsonObject x) {
	std::set<std::string> have;
	{
		BinaryJsonObjectIterator i = iterator();
		while ( i.more() )
			have.insert( i.next().fieldName() );
	}

	BinaryJsonObjectIterator it(x);
	while ( it.more() ) {
		BinaryJsonElement e = it.next();
		if ( have.count( e.fieldName() ) )
			continue;
		append(e);
	}
	return *this;
}
inline bool BinaryJsonObjectBuilder::appendAsNumber( const StringData& fieldName , const string& data ) {
	if ( data.size() == 0 || data == "-" || data == ".")
		return false;

	unsigned int pos=0;
	if ( data[0] == '-' )
		pos++;

	bool hasDec = false;

	for ( ; pos<data.size(); pos++ ) {
		if ( isdigit(data[pos]) )
			continue;

		if ( data[pos] == '.' ) {
			if ( hasDec )
				return false;
			hasDec = true;
			continue;
		}

		return false;
	}

	if ( hasDec ) {
		double d = atof( data.c_str() );
		append( fieldName , d );
		return true;
	}

	if ( data.size() < 8 ) {
		append( fieldName , atoi( data.c_str() ) );
		return true;
	}

	try {
		long long num = boost::lexical_cast<long long>( data );
		append( fieldName , num );
		return true;
	}
	catch(boost::bad_lexical_cast &) {
		return false;
	}
}
inline BinaryJsonObjectIterator BinaryJsonObjectBuilder::iterator() const {
	const char * s = _b.buf() + _offset;
	const char * e = _b.buf() + _b.len();
	return BinaryJsonObjectIterator( s , e );
}

