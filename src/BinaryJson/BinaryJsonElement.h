//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"

#include "../BinaryJson/BinaryJsonTypes.h"
#include "../BinaryJson/Builder.h"
class BinaryJsonObject;
class BinaryJsonElement;
class BinaryJsonObjectBuilder;

class BinaryJsonElement
{
public:
	BinaryJsonElement() {
		static char z = 0;
		data = &z;
		fieldNameSize_ = 0;
		totalSize = 1;
	}
	// @param maxLen don't scan more than maxLen bytes
	explicit BinaryJsonElement(const char *d, int maxLen) : data(d) {
		if ( eoo() ) {
			totalSize = 1;
			fieldNameSize_ = 0;
		}
		else {
			totalSize = -1;
			fieldNameSize_ = -1;
			if ( maxLen != -1 ) {
				int size = (int) strnlen( fieldName(), maxLen - 1 );
				//wangyan uassert( 10333 ,  "Invalid field name", size != -1 );
				fieldNameSize_ = size + 1;
			}
		}
	}

	explicit BinaryJsonElement(const char *d) : data(d) {
		fieldNameSize_ = -1;
		totalSize = -1;
		if ( eoo() ) {
			fieldNameSize_ = 0;
			totalSize = 1;
		}
	}


	/** These functions, which start with a capital letter, throw a UserException if the
	element is not of the required type. Example:

	std::string foo = obj["foo"].String(); // std::exception if not a std::string type or DNE
	*/
	std::string String()        const { return valuestr(); }//wangyan
	double Number()             const { return chk(isNumber()).number(); }
	double Double()             const { return _numberDouble(); }
	long long Long()            const { return _numberLong(); }
	int Int()                   const { return _numberInt(); }
	bool Bool()                 const { return boolean(); }//wangyan
	std::vector<BinaryJsonElement> Array() const; // see implementation for detailed comments

	void Null()                 const { chk(isNull()); } // throw UserException if not null
	void OK()                   const { chk(ok()); }     // throw UserException if element DNE

	/** @return the embedded object associated with this field.
	Note the returned object is a reference to within the parent bson object. If that
	object is out of scope, this pointer will no longer be valid. Call getOwned() on the
	returned BinaryJsonObject if you need your own copy.
	throws UserException if the element is not of type object.
	*/
	BinaryJsonObject Obj()               const;

	BinaryJsonObject embeddedObjectUserCheck() const;
	BinaryJsonObject embeddedObject() const;

	/** populate v with the value of the element.  If type does not match, throw exception.
	useful in templates -- see also BinaryJsonObject::Vals().
	*/

	void Val(long long& v)      const { v = Long(); }
	void Val(bool& v)           const { v = Bool(); }
	void Val(BinaryJsonObject& v)        const;
	void Val(int& v)            const { v = Int(); }
	void Val(double& v)         const { v = Double(); }
	void Val(std::string& v)    const { v = String(); }

	/** Use ok() to check if a value is assigned:
	if( myObj["foo"].ok() ) ...
	*/
	bool ok() const { return !eoo(); }

	std::string toString( bool includeFieldName = true, bool full=false) const;
	void toString(StringBuilder& s, bool includeFieldName = true, bool full=false, int depth=0) const;
	operator std::string() const { return toString(); }

	/** Returns the type of the element */
	BinaryJsonType type() const
	{
		return (BinaryJsonType) *reinterpret_cast< const signed char * >(data);
	}

	/** retrieve a field within this element
	throws exception if *this is not an embedded object
	*/
	BinaryJsonElement operator[] (const std::string& field) const;

	/** returns the tyoe of the element fixed for the main type
	the main purpose is numbers.  any numeric type will return NumberDouble
	Note: if the order changes, indexes have to be re-built or than can be corruption
	*/
	int canonicalType() const;

	/** Indicates if it is the end-of-object element, which is present at the end of
	every BSON object.
	*/
	bool eoo() const { return type() == EOO; }


	/** Size of the element.
	@param maxLen If maxLen is specified, don't scan more than maxLen bytes to calculate size.
	*/
	int size( int maxLen ) const;
	int size() const;

	/** Wrap this element up as a singleton object. */
	BinaryJsonObject wrap() const;

	/** Wrap this element up as a singleton object with a new name. */
	BinaryJsonObject wrap( const char* newName) const;


	int fieldNameSize() const {
		if ( fieldNameSize_ == -1 )
			fieldNameSize_ = (int)strlen( fieldName() ) + 1;
		return fieldNameSize_;
	}
	/** field name of the element.  e.g., for
	name : "Joe"
	"name" is the fieldname
	*/
	const char * fieldName() const {
		if ( eoo() ) return ""; // no fieldname for it.
		return data + 1;
	}

	/** raw data of the element's value (so be careful). */
	const char * value() const {
		return (data + fieldNameSize() + 1);
	}
	const char * rawdata() const { return data; }




	/** size in bytes of the element's value (when applicable). */
	int valuesize() const {
		return size() - fieldNameSize() - 1;
	}
	/** @return value of a boolean element.
	You must assure element is a boolean before
	calling. */
	bool boolean() const {
		return *value() ? true : false;
	}

	bool booleanSafe() const { return isBoolean() && boolean(); }


	/** Convert the value to boolean, regardless of its type, in a javascript-like fashion
	(i.e., treats zero and null and eoo as false).
	*/
	bool trueValue() const;

	/** True if number, string, bool, date, OID */
	bool isSimpleType() const;

	/** True if element is of a numeric type. */
	bool isNumber() const;

	bool isInteger() const;

	bool isLong() const;

	bool isDouble() const;
	/** Return double value for this field. MUST be NumberDouble type. */
	double _numberDouble() const {return *(double*)( value() ); }//wangyan
	/** Return int value for this field. MUST be NumberInt type. */
	int _numberInt() const {return *reinterpret_cast< const int* >( value() ); }
	/** Return long long value for this field. MUST be NumberLong type. */
	long long _numberLong() const {return *reinterpret_cast< const long long* >( value() ); }

	/** Retrieve int value for the element safely.  Zero returned if not a number. */
	int numberInt() const;
	/** Retrieve long value for the element safely.  Zero returned if not a number.
	*  Behavior is not defined for double values that are NaNs, or too large/small
	*  to be represented by long longs */
	long long numberLong() const;


	/** Like numberLong() but with well-defined behavior for doubles that
	*  are NaNs, or too large/small to be represented as long longs.
	*  NaNs -> 0
	*  very large doubles -> LLONG_MAX
	*  very small doubles -> LLONG_MIN  */
	long long safeNumberLong() const;

	/** Retrieve the numeric value of the element.  If not of a numeric type, returns 0.
	Note: casts to double, data loss may occur with large (>52 bit) NumberLong values.
	*/
	double numberDouble() const;
	/** Retrieve the numeric value of the element.  If not of a numeric type, returns 0.
	Note: casts to double, data loss may occur with large (>52 bit) NumberLong values.
	*/
	double number() const { return numberDouble(); }

	/** True if element is null. */
	bool isNull() const {
		return type() == jstNULL;
	}
	/** True if this element can be a BSONObj */
	bool isABSONObj() const {
		switch( type() ) {
		case Object:
		case JSONArray:
			return true;
		default:
			return false;
		}
	}
	bool isObjectType() const {
		switch( type() ) {
		case Object:
			return true;
		default:
			return false;
		}
	}
	bool isArrayType() const {
		switch( type() ) {
		case JSONArray:
			return true;
		default:
			return false;
		}
	}
	bool isStringType() const {
		switch( type() ) {
		case JSONString:
			return true;
		default:
			return false;
		}
	}
	/** Size (length) of a string element.
	You must assure of type String first.
	@return string size including terminating null
	*/
	int valuestrsize() const {
		return *reinterpret_cast< const int* >( value() );
	}
	// for objects the size *includes* the size of the size field
	int objsize() const {
		return *reinterpret_cast< const int* >( value() );
	}

	/** Check that data is internally consistent. */
	void validate() const;


	/** Get a string's value.  Also gives you start of the real data for an embedded object.
	You must assure data is of an appropriate type first -- see also valuestrsafe().
	*/
	const char * valuestr() const {
		return value() + 4;
	}
	/** Get the string value of the element.  If not a string returns "". */
	const char *valuestrsafe() const {
		return type() == JSONString ? valuestr() : "";
	}

	/** Get the string value of the element.  If not a string returns "". */
	std::string str() const {
		return type() == JSONString ? std::string(valuestr(), valuestrsize()-1) : std::string();
	}

	/** like operator== but doesn't check the fieldname,
	just the value.
	*/
	bool valuesEqual(const BinaryJsonElement& r) const {
		return woCompare( r , false ) == 0;
	}

	/** Returns true if elements are equal. */
	bool operator==(const BinaryJsonElement& r) const {
		return woCompare( r , true ) == 0;
	}
	/** Returns true if elements are unequal. */
	bool operator!=(const BinaryJsonElement& r) const { return !operator==(r); }

	/** Well ordered comparison.
	@return <0: l<r. 0:l==r. >0:l>r
	order by type, field name, and field value.
	If considerFieldName is true, pay attention to the field name.
	*/
	int woCompare( const BinaryJsonElement &e, bool considerFieldName = true ) const;





	bool isBoolean() const { return type() ==  JSONBool; }





private:
	const char *data;
	mutable int fieldNameSize_; // cached value

	mutable int totalSize; /* caches the computed size */
	friend class BinaryJsonObjectIterator;
	friend class BinaryJsonObject;

	const BinaryJsonElement& chk(bool expr) const {
		//wangyan massert(13118, "unexpected or missing type value in BSON object", expr);
		return *this;
	}
};

