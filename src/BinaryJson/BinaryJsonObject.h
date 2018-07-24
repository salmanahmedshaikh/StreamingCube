//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include <boost/intrusive_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <set>
#include <list>
#include <string>
#include <vector>
#include "../BinaryJson/Builder.h"
#include "../BinaryJson/BinaryJsonElement.h"
#include "../BinaryJson/BinaryJsonTypes.h"
#include "../BinaryJson/atomic_int.h"
class BinaryJsonObject
{

public:

	/** Construct a BinaryJsonObject from data in the proper format.
	*  Use this constructor when something else owns msgdata's buffer
	*/
	explicit BinaryJsonObject(const char *msgdata) {
		init(msgdata);
	}

	/** Construct a BinaryJsonObject from data in the proper format.
	*  Use this constructor when you want BinaryJsonObject to free(holder) when it is no longer needed
	*  BinaryJsonObject::Holder has an extra 4 bytes for a ref-count before the start of the object
	*/
	class Holder;
	explicit BinaryJsonObject(Holder* holder) {
		init(holder);
	}
	~BinaryJsonObject() {
		_objdata = 0; // defensive
	}
	BinaryJsonObject(const BinaryJsonObject &rO):
	_objdata(rO._objdata), _holder(rO._holder) {
	}

	BinaryJsonObject &operator=(const BinaryJsonObject &rRHS) {
		if (this != &rRHS) {
			_objdata = rRHS._objdata;
			_holder = rRHS._holder;
		}
		return *this;
	}
	/** Construct an empty BinaryJsonObject -- that is, {}. */
	BinaryJsonObject();

	bool isOwned() const { return _holder.get() != 0; }

	/** assure the data buffer is under the control of this BinaryJsonObject and not a remote buffer
	@see isOwned()
	*/
	BinaryJsonObject getOwned() const;

	/** @return a new full (and owned) copy of the object. */
	BinaryJsonObject copy() const;
	/** Readable representation of a BSON object in an extended JSON-style notation.
	This is an abbreviated representation which might be used for logging.
	*/
	enum { maxToStringRecursionDepth = 100 };

	std::string toString( bool isArray = false, bool full=false ) const;
	void toString( StringBuilder& s, bool isArray = false, bool full=false, int depth=0 ) const;


	/** note: addFields always adds _id even if not specified */
	int addFields(BinaryJsonObject& from, std::set<std::string>& fields); /* returns n added */

	/** remove specified field and return a new object with the remaining fields.
	slowish as builds a full new object
	*/
	BinaryJsonObject removeField(const StringData& name) const;

	/** returns # of top level fields in the object
	note: iterates to count the fields
	*/
	int nFields() const;

	/** adds the field names to the fields set.  does NOT clear it (appends). */
	int getFieldNames(std::set<std::string>& fields) const;

	/** @return the specified element.  element.eoo() will be true if not found.
	@param name field to find. supports dot (".") notation to reach into embedded objects.
	for example "x.y" means "in the nested object in field x, retrieve field y"
	*/
	BinaryJsonElement getFieldDotted(const char *name) const;
	/** @return the specified element.  element.eoo() will be true if not found.
	@param name field to find. supports dot (".") notation to reach into embedded objects.
	for example "x.y" means "in the nested object in field x, retrieve field y"
	*/
	BinaryJsonElement getFieldDotted(const std::string& name) const {
		return getFieldDotted( name.c_str() );
	}


	/** Like getFieldDotted(), but returns first array encountered while traversing the
	dotted fields of name.  The name variable is updated to represent field
	names with respect to the returned element. */
	BinaryJsonElement getFieldDottedOrArray(const char *&name) const;

	/** Get the field of the specified name. eoo() is true on the returned
	element if not found.
	*/
	BinaryJsonElement getField(const StringData& name) const;

	/** Get several fields at once. This is faster than separate getField() calls as the size of
	elements iterated can then be calculated only once each.
	@param n number of fieldNames, and number of elements in the fields array
	@param fields if a field is found its element is stored in its corresponding position in this array.
	if not found the array element is unchanged.
	*/
	void getFields(unsigned n, const char **fieldNames, BinaryJsonElement *fields) const;


	/** @return true if field exists */
	bool hasField( const char * name ) const { return !getField(name).eoo(); }
	/** @return true if field exists */
	bool hasElement(const char *name) const { return hasField(name); }

	/** @return "" if DNE or wrong type */
	const char * getStringField(const char *name) const;

	/** @return subobject of the given name */
	BinaryJsonObject getObjectField(const char *name) const;

	/** @return INT_MIN if not present - does some type conversions */
	//int getIntField(const char *name) const;

	/** @return false if not present
	@see BSONElement::trueValue()
	*/
	bool getBoolField(const char *name) const;

	/** @param pattern a BSON obj indicating a set of (un-dotted) field
	*  names.  Element values are ignored.
	*  @return a BSON obj constructed by taking the elements of this obj
	*  that correspond to the fields in pattern. Field names of the
	*  returned object are replaced with the empty string. If field in
	*  pattern is missing, it is omitted from the returned object.
	*
	*  Example: if this = {a : 4 , b : 5 , c : 6})
	*    this.extractFieldsUnDotted({a : 1 , c : 1}) -> {"" : 4 , "" : 6 }
	*    this.extractFieldsUnDotted({b : "blah"}) -> {"" : 5}
	*
	*/
	BinaryJsonObject extractFieldsUnDotted(const BinaryJsonObject& pattern) const;

	/** extract items from object which match a pattern object.
	e.g., if pattern is { x : 1, y : 1 }, builds an object with
	x and y elements of this object, if they are present.
	returns elements with original field names
	*/
	BinaryJsonObject extractFields(const BinaryJsonObject &pattern , bool fillWithNull=false) const;

	BinaryJsonObject filterFieldsUndotted(const BinaryJsonObject &filter, bool inFilter) const;

	BinaryJsonElement getFieldUsingIndexNames(const char *fieldName, const BinaryJsonObject &indexKey) const;

	/** @return first field of the object */
	BinaryJsonElement firstElement() const { return BinaryJsonElement(objdata() + 4); }

	/** faster than firstElement().fieldName() - for the first element we can easily find the fieldname without
	computing the element size.
	*/
	const char * firstElementFieldName() const {
		const char *p = objdata() + 4;
		return *p == EOO ? "" : p+1;
	}

	BinaryJsonType firstElementType() const {
		const char *p = objdata() + 4;
		return (BinaryJsonType) *p;
	}

	/** @return the raw data of the object */
	const char *objdata() const {
		return _objdata;
	}
	/** @return total size of the BSON object in bytes */
	int objsize() const { return *(reinterpret_cast<const int*>(objdata())); }

	/** performs a cursory check on the object's size only. */
	bool isValid() const;


	/** @return true if object is empty -- i.e.,  {} */
	bool isEmpty() const { return objsize() <= 5; }

	/** Get the field of the specified name. eoo() is true on the returned
	element if not found.
	*/
	BinaryJsonElement operator[] (const char *field) const {
		return getField(field);
	}

	BinaryJsonElement operator[] (const std::string& field) const {
		return getField(field);
	}

	BinaryJsonElement operator[] (int field) const {
		StringBuilder ss;
		ss << field;
		std::string s = ss.str();
		return getField(s.c_str());
	}


	/**wo='well ordered'.  fields must be in same order in each object.
	Ordering is with respect to the signs of the elements
	and allows ascending / descending key mixing.
	@return  <0 if l<r. 0 if l==r. >0 if l>r
	*/
	int woCompare(const BinaryJsonObject& r, const BinaryJsonObject &ordering = BinaryJsonObject(),
		bool considerFieldName=true) const;

	bool operator<( const BinaryJsonObject& other ) const { return woCompare( other ) < 0; }
	bool operator<=( const BinaryJsonObject& other ) const { return woCompare( other ) <= 0; }
	bool operator>( const BinaryJsonObject& other ) const { return woCompare( other ) > 0; }
	bool operator>=( const BinaryJsonObject& other ) const { return woCompare( other ) >= 0; }


public:
	class Holder : boost::noncopyable {
	private:
		Holder(); // this class should never be explicitly created
		AtomicUInt refCount;
	public:
		char data[4]; // start of object

		void zero() { refCount.zero(); }

		// these are called automatically by boost::intrusive_ptr
		friend void intrusive_ptr_add_ref(Holder* h) { h->refCount++; }
		friend void intrusive_ptr_release(Holder* h) {
#if defined(_DEBUG) // cant use dassert or DEV here
			assert((int)h->refCount > 0); // make sure we haven't already freed the buffer
#endif
			if(--(h->refCount) == 0){
#if defined(_DEBUG)
				unsigned sz = (unsigned&) *h->data;
				assert(sz < BSONObjMaxInternalSize * 3);
				memset(h->data, 0xdd, sz);
#endif
				free(h);
			}
		}
	};
private:
	const char *_objdata;
	boost::intrusive_ptr< Holder > _holder;

	void init(Holder *holder) {
		_holder = holder; // holder is now managed by intrusive_ptr
		init(holder->data);
	}
	void init(const char *data) {
		_objdata = data;
		if ( !isValid() )
			assert(0);
	}

};
std::ostream& operator<<( std::ostream &s, const BinaryJsonObject &o );
std::ostream& operator<<( std::ostream &s, const BinaryJsonElement &e );

StringBuilder& operator<<( StringBuilder &s, const BinaryJsonObject &o );
StringBuilder& operator<<( StringBuilder &s, const BinaryJsonElement &e );

struct BinaryJsonArray : BinaryJsonObject {
	// Don't add anything other than forwarding constructors!!!
	BinaryJsonArray(): BinaryJsonObject() {}
	explicit BinaryJsonArray(const BinaryJsonObject& obj): BinaryJsonObject(obj) {}
};
