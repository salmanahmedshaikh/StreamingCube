//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../BinaryJson/BinaryJsonBuilderBase.h"
#include "../BinaryJson/BinaryJsonObjectIterator.h"
#include "../BinaryJson/Builder.h"
#include "../BinaryJson/BinaryJsonElement.h"
#include <boost/noncopyable.hpp>
#include <boost/static_assert.hpp>
#include <limits.h>
class BinaryJsonObjectBuilder:public BinaryJsonBuilderBase, private boost::noncopyable
{
private:
	BufBuilder &_b;
	BufBuilder _buf;
	int _offset;
public:
	/** @param initsize this is just a hint as to the final size of the object */
	BinaryJsonObjectBuilder(int initsize=512)  : _b(_buf), _buf(initsize + sizeof(unsigned)), _offset( sizeof(unsigned)), _doneCalled(false)
	{
		_b.appendNum((unsigned)0); // ref-count
		_b.skip(4); /*leave room for size field and ref-count*/
	}
	/** @param baseBuilder construct a BinaryJsonObjectBuilder using an existing BufBuilder
	*  This is for more efficient adding of subobjects/arrays. See docs for subobjStart for example.
	*/
	BinaryJsonObjectBuilder( BufBuilder &baseBuilder ) : _b( baseBuilder ), _buf( 0 ), _offset( baseBuilder.len() ),  _doneCalled(false) {
		_b.skip( 4 );
	}



	~BinaryJsonObjectBuilder(void)
	{
		if ( !_doneCalled && _b.buf() && _buf.getSize() == 0 ) {
			_done();
		}
	}
	/** add all the fields from the object specified to this object */
	BinaryJsonObjectBuilder& appendElements(BinaryJsonObject x);

	/** add all the fields from the object specified to this object if they don't exist already */
	BinaryJsonObjectBuilder& appendElementsUnique( BinaryJsonObject x );

	/** append element to the object we are building */
	BinaryJsonObjectBuilder& append( const BinaryJsonElement& e) {
		assert( !e.eoo() ); // do not append eoo, that would corrupt us. the builder auto appends when done() is called.
		_b.appendBuf((void*) e.rawdata(), e.size());
		return *this;
	}
	/** append an element but with a new name */
	BinaryJsonObjectBuilder& appendAs(const BinaryJsonElement& e, const StringData& fieldName) {
		assert( !e.eoo() ); // do not append eoo, that would corrupt us. the builder auto appends when done() is called.
		_b.appendNum((char) e.type());
		_b.appendStr(fieldName);
		_b.appendBuf((void *) e.value(), e.valuesize());
		return *this;
	}

	/** add a subobject as a member */
	BinaryJsonObjectBuilder& append(const StringData& fieldName, BinaryJsonObject subObj) {
		_b.appendNum((char) Object);
		_b.appendStr(fieldName);
		_b.appendBuf((void *) subObj.objdata(), subObj.objsize());
		return *this;
	}

	/** add a subobject as a member */
	BinaryJsonObjectBuilder& appendObject(const StringData& fieldName, const char * objdata , int size = 0 ) {
		assert( objdata );
		if ( size == 0 ) {
			size = *((int*)objdata);
		}

		assert( size > 4 && size < 100000000 );

		_b.appendNum((char) Object);
		_b.appendStr(fieldName);
		_b.appendBuf((void*)objdata, size );
		return *this;
	}
	/** add header for a new subobject and return bufbuilder for writing to
	*  the subobject's body
	*
	*  example:
	*
	*  BinaryJsonObjectBuilder b;
	*  BinaryJsonObjectBuilder sub (b.subobjStart("fieldName"));
	*  // use sub
	*  sub.done()
	*  // use b and convert to object
	*/
	BufBuilder &subobjStart(const StringData& fieldName) {
		_b.appendNum((char) Object);
		_b.appendStr(fieldName);
		return _b;
	}
	/** add a subobject as a member with type Array.  Thus arr object should have "0", "1", ...
	style fields in it.
	*/
	BinaryJsonObjectBuilder& appendArray(const StringData& fieldName, const BinaryJsonObject &subObj) {
		_b.appendNum((char) JSONArray);
		_b.appendStr(fieldName);
		_b.appendBuf((void *) subObj.objdata(), subObj.objsize());
		return *this;
	}
	BinaryJsonObjectBuilder& append(const StringData& fieldName, BinaryJsonArray arr) {
		return appendArray(fieldName, arr);
	}

	/** add header for a new subarray and return bufbuilder for writing to
	the subarray's body */
	BufBuilder &subarrayStart(const StringData& fieldName) {
		_b.appendNum((char) JSONArray);
		_b.appendStr(fieldName);
		return _b;
	}
	/** add a subobject as a member with type Array.  Thus arr object should have "0", "1", ...
	style fields in it.
	*/
	/** Append a boolean element */
	BinaryJsonObjectBuilder& appendBool(const StringData& fieldName, int val) {
		_b.appendNum((char) JSONBool);
		_b.appendStr(fieldName);
		_b.appendNum((char) (val?1:0));
		return *this;
	}

	/** Append a boolean element */
	BinaryJsonObjectBuilder& append(const StringData& fieldName, bool val) {
		_b.appendNum((char) JSONBool);
		_b.appendStr(fieldName);
		_b.appendNum((char) (val?1:0));
		return *this;
	}

	/** Append a 32 bit integer element */
	BinaryJsonObjectBuilder& append(const StringData& fieldName, int n) {
		_b.appendNum((char) NumberInt);
		_b.appendStr(fieldName);
		_b.appendNum(n);
		return *this;
	}

	/** Append a 32 bit unsigned element - cast to a signed int. */
	BinaryJsonObjectBuilder& append(const StringData& fieldName, unsigned n) {
		return append(fieldName, (int) n);
	}

	/** Append a NumberLong */
	BinaryJsonObjectBuilder& append(const StringData& fieldName, long long n) {
		_b.appendNum((char) NumberLong);
		_b.appendStr(fieldName);
		_b.appendNum(n);
		return *this;
	}

	/** appends a number.  if n < max(int)/2 then uses int, otherwise long long */
	BinaryJsonObjectBuilder& appendIntOrLL( const StringData& fieldName , long long n ) {
		long long x = n;
		if ( x < 0 )
			x = x * -1;
#ifdef WINDOWS
		if ( x < ( (std::numeric_limits<int>::max)() / 2 ) ) // extra () to avoid max macro on windows
#else

		if ( x < (  INT_MAX  / 2 ) ) // extra () to avoid max macro on windows
#endif
			append( fieldName , (int)n );
		else
			append( fieldName , n );
		return *this;
	}

	/**
	* appendNumber is a series of method for appending the smallest sensible type
	* mostly for JS
	*/
	BinaryJsonObjectBuilder& appendNumber( const StringData& fieldName , int n ) {
		return append( fieldName , n );
	}

	BinaryJsonObjectBuilder& appendNumber( const StringData& fieldName , double d ) {
		return append( fieldName , d );
	}

	BinaryJsonObjectBuilder& appendNumber( const StringData& fieldName , size_t n ) {
		static const size_t maxInt = ( 1 << 30 );

		if ( n < maxInt )
			append( fieldName, static_cast<int>( n ) );
		else
			append( fieldName, static_cast<long long>( n ) );
		return *this;
	}

	BinaryJsonObjectBuilder& appendNumber( const StringData& fieldName, long long llNumber ) {
		static const long long maxInt = ( 1LL << 30 );
		static const long long maxDouble = ( 1LL << 40 );

		long long nonNegative = llNumber >= 0 ? llNumber : -llNumber;
		if ( nonNegative < maxInt )
			append( fieldName, static_cast<int>( llNumber ) );
		else if ( nonNegative < maxDouble )
			append( fieldName, static_cast<double>( llNumber ) );
		else
			append( fieldName, llNumber );
		return *this;
	}

	/** Append a double element */
	BinaryJsonObjectBuilder& append(const StringData& fieldName, double n) {
		_b.appendNum((char) NumberDouble);
		_b.appendStr(fieldName);
		_b.appendNum(n);
		return *this;
	}

	/** tries to append the data as a number
	* @return true if the data was able to be converted to a number
	*/
	bool appendAsNumber( const StringData& fieldName , const std::string& data );

	/** Append a string element.
	@param sz size includes terminating null character */
	BinaryJsonObjectBuilder& append(const StringData& fieldName, const char *str, int sz) {
		_b.appendNum((char) JSONString);
		_b.appendStr(fieldName);
		_b.appendNum((int)sz);
		_b.appendBuf(str, sz);
		return *this;
	}
	/** Append a string element */
	BinaryJsonObjectBuilder& append(const StringData& fieldName, const char *str) {
		return append(fieldName, str, (int) strlen(str)+1);
	}
	/** Append a string element */
	BinaryJsonObjectBuilder& append(const StringData& fieldName, const std::string& str) {
		return append(fieldName, str.c_str(), (int) str.size()+1);
	}
	/** Implements builder interface but no-op in ObjBuilder */
	void appendNull() {
		throw runtime_error("Invalid call to appendNull in BinaryJsonObject Builder.");
	}


	/** Append a Null element to the object */
	BinaryJsonObjectBuilder& appendNull( const StringData& fieldName ) {
		_b.appendNum( (char) jstNULL );
		_b.appendStr( fieldName );
		return *this;
	}

	/** Append an array of values. */
	template < class T >
	BinaryJsonObjectBuilder& append( const StringData& fieldName, const std::vector< T >& vals );

	template < class T >
	BinaryJsonObjectBuilder& append( const StringData& fieldName, const std::list< T >& vals );

	/** Append a set of values. */
	template < class T >
	BinaryJsonObjectBuilder& append( const StringData& fieldName, const std::set< T >& vals );


	BinaryJsonObject obj() {

		bool own = owned();
		//wangyan massert( 10335 , "builder does not own memory", own );
		doneFast();
		BinaryJsonObject::Holder* h = (BinaryJsonObject::Holder*)_b.buf();
		decouple(); // sets _b.buf() to NULL
		return BinaryJsonObject(h);
	}
	/** Fetch the object we have built.
	BinaryJsonObjectBuilder still frees the object when the builder goes out of
	scope -- very important to keep in mind.  Use obj() if you
	would like the BinaryJsonObject to last longer than the builder.
	*/
	BinaryJsonObject done() {
		return BinaryJsonObject(_done());
	}

	// Like 'done' above, but does not construct a BinaryJsonObject to return to the caller.
	void doneFast() {
		(void)_done();
	}




	/* assume ownership of the buffer - you must then free it (with free()) */
	char* decouple(int& l) {
		char *x = _done();
		assert( x );
		l = _b.len();
		_b.decouple();
		return x;
	}
	void decouple() {
		_b.decouple();    // post done() call version.  be sure jsobj frees...
	}
	BinaryJsonObjectBuilder& operator<<( const BinaryJsonElement& e ){
		append( e );
		return *this;
	}
	static std::string numStr( int i ) {

		StringBuilder o;
		o << i;
		return o.str();
	}

	bool isArray() const {
		return false;
	}
	/** @return true if we are using our own bufbuilder, and not an alternate that was given to us in our constructor */
	bool owned() const { return &_b == &_buf; }

	BinaryJsonObjectIterator iterator() const ;

	bool hasField( const StringData& name ) const ;

	int len() const { return _b.len(); }

	BufBuilder& bb() { return _b; }
private:
	bool _doneCalled;

	char* _done() {
		if ( _doneCalled )
			return _b.buf() + _offset;

		_doneCalled = true;
		//wangyan _s.endField();
		_b.appendNum((char) EOO);
		char *data = _b.buf() + _offset;
		int size = _b.len() - _offset;
		*((int*)data) = size;

		return data;
	}
};



class BinaryJsonArrayBuilder : public BinaryJsonBuilderBase, private boost::noncopyable {
public:
	BinaryJsonArrayBuilder() : _i(0), _b() {}
	BinaryJsonArrayBuilder( BufBuilder &_b ) : _i(0), _b(_b) {}
	BinaryJsonArrayBuilder( int initialSize ) : _i(0), _b(initialSize) {}

	template <typename T>
	BinaryJsonArrayBuilder& append(const T& x) {
		_b.append(num(), x);
		return *this;
	}

	BinaryJsonArrayBuilder& append(const BinaryJsonElement& e) {
		_b.appendAs(e, num());
		return *this;
	}

	BinaryJsonArrayBuilder& operator<<(const BinaryJsonElement& e) {
		return append(e);
	}

	template <typename T>
	BinaryJsonArrayBuilder& operator<<(const T& x) {
		return append(x);
	}

	void appendNull() {
		_b.appendNull(num());
	}


	/**
	* destructive - ownership moves to returned BSONArray
	* @return owned BSONArray
	*/
	//BinaryJsonArray arr() { return BinaryJsonArray(_b.obj()); }
	BinaryJsonObject obj() { return _b.obj(); }

	BinaryJsonObject done() { return _b.done(); }

	void doneFast() { _b.doneFast(); }

	BinaryJsonArrayBuilder& append(const StringData& name, int n) {
		fill( name );
		append( n );
		return *this;
	}

	BinaryJsonArrayBuilder& append(const StringData& name, long long n) {
		fill( name );
		append( n );
		return *this;
	}

	BinaryJsonArrayBuilder& append(const StringData& name, double n) {
		fill( name );
		append( n );
		return *this;
	}

	template <typename T>
	BinaryJsonArrayBuilder& append(const StringData& name, const T& x) {
		fill( name );
		append( x );
		return *this;
	}

	template < class T >
	BinaryJsonArrayBuilder& append( const std::list< T >& vals );

	template < class T >
	BinaryJsonArrayBuilder& append( const std::set< T >& vals );

	// These two just use next position
	BufBuilder &subobjStart() { return _b.subobjStart( num() ); }
	BufBuilder &subarrayStart() { return _b.subarrayStart( num() ); }

	// These fill missing entries up to pos. if pos is < next pos is ignored
	BufBuilder &subobjStart(int pos) {
		fill(pos);
		return _b.subobjStart( num() );
	}
	BufBuilder &subarrayStart(int pos) {
		fill(pos);
		return _b.subarrayStart( num() );
	}

	// These should only be used where you really need interface compatability with BinaryJsonObjectBuilder
	// Currently they are only used by update.cpp and it should probably stay that way
	BufBuilder &subobjStart( const StringData& name ) {
		fill( name );
		return _b.subobjStart( num() );
	}

	BufBuilder &subarrayStart( const StringData& name ) {
		fill( name );
		return _b.subarrayStart( num() );
	}

	BinaryJsonArrayBuilder& appendArray( const StringData& name, const BinaryJsonObject& subObj ) {
		fill( name );
		_b.appendArray( num(), subObj );
		return *this;
	}

	BinaryJsonArrayBuilder& appendAs( const BinaryJsonElement &e, const StringData& name) {
		fill( name );
		append( e );
		return *this;
	}

	BinaryJsonObjectIterator iterator() const ;
	bool isArray() const {
		return true;
	}

	int len() const { return _b.len(); }
	int arrSize() const { return _i; }

private:
	// These two are undefined privates to prevent their accidental
	// use as we don't support unsigned ints in BSON
	BinaryJsonArrayBuilder& append(const StringData& fieldName, unsigned int val);
	BinaryJsonArrayBuilder& append(const StringData& fieldName, unsigned long long val);

	void fill( const StringData& name ) {
		char *r;
		long int n = strtol( name.data(), &r, 10 );
		if ( *r )
			//wangyan uasserted( 13048, (std::string)"can't append to array using string field name [" + name.data() + "]" );
			fill(n);
	}

	void fill (int upTo){
		// if this is changed make sure to update error message and jstests/set7.js
		const int maxElems = 1500000;
		BOOST_STATIC_ASSERT(maxElems < (BSONObjMaxUserSize/10));
		//wangyan uassert(15891, "can't backfill array to larger than 1,500,000 elements", upTo <= maxElems);

		while( _i < upTo )
			appendNull();
	}

	std::string num() { return _b.numStr(_i++); }
	int _i;
	BinaryJsonObjectBuilder _b;
};

template < class T >
inline BinaryJsonObjectBuilder& BinaryJsonObjectBuilder::append( const StringData& fieldName, const std::vector< T >& vals ) {
	BinaryJsonObjectBuilder arrBuilder;
	for ( unsigned int i = 0; i < vals.size(); ++i )
		arrBuilder.append( numStr( i ), vals[ i ] );
	appendArray( fieldName, arrBuilder.done() );
	return *this;
}
template < class L >
inline BinaryJsonObjectBuilder& _appendIt( BinaryJsonObjectBuilder& _this, const StringData& fieldName, const L& vals ) {
	BinaryJsonObjectBuilder arrBuilder;
	int n = 0;
	for( typename L::const_iterator i = vals.begin(); i != vals.end(); i++ )
		arrBuilder.append( BinaryJsonObjectBuilder::numStr(n++), *i );
	_this.appendArray( fieldName, arrBuilder.done() );
	return _this;
}

template < class T >
inline BinaryJsonObjectBuilder& BinaryJsonObjectBuilder::append( const StringData& fieldName, const std::list< T >& vals ) {
	return _appendIt< std::list< T > >( *this, fieldName, vals );
}

template < class T >
inline BinaryJsonObjectBuilder& BinaryJsonObjectBuilder::append( const StringData& fieldName, const std::set< T >& vals ) {
	return _appendIt< std::set< T > >( *this, fieldName, vals );
}
