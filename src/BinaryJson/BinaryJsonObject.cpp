//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../BinaryJson/BinaryJsonObject.h"
#include "../BinaryJson/BinaryJsonElement.h"
#include "../BinaryJson/BinaryJsonObjectIterator.h"
#include "../BinaryJson/BinaryJsonObjectBuilder.h"
#include <limits.h>
BinaryJsonElement eooElement;
BinaryJsonObject::BinaryJsonObject() {
	/* little endian ordering here, but perhaps that is ok regardless as BSON is spec'd
	to be little endian external to the system. (i.e. the rest of the implementation of bson,
	not this part, fails to support big endian)
	*/
	static char p[] = { /*size*/5, 0, 0, 0, /*eoo*/0 };
	_objdata = p;
}
bool BinaryJsonObject::isValid() const {
	int x = objsize();
	return x > 0 && x <= BSONObjMaxInternalSize;
}

BinaryJsonObject BinaryJsonObject::getOwned() const {
	if ( isOwned() )
		return *this;
	return copy();
}
/* the idea with NOINLINE_DECL here is to keep this from inlining in the
getOwned() method.  the presumption being that is better.
*/
NOINLINE_DECL BinaryJsonObject BinaryJsonObject::copy() const {
	Holder *h = (Holder*) malloc(objsize() + sizeof(unsigned));
	h->zero();
	memcpy(h->data, objdata(), objsize());
	return BinaryJsonObject(h);
}

/*
returns n added not counting _id unless requested.
*/
int BinaryJsonObject::addFields(BinaryJsonObject& from, set<string>& fields) {
	assert( isEmpty() && !isOwned() ); /* partial implementation for now... */

	BinaryJsonObjectBuilder b;

	int N = fields.size();
	int n = 0;
	BinaryJsonObjectIterator i(from);
	//wangyan bool gotId = false;
	while ( i.moreWithEOO() ) {
		BinaryJsonElement e = i.next();
		const char *fname = e.fieldName();
		if ( fields.find(fname)!=fields.end() ) {
			b.append(e);
			++n;
			//gotId = gotId || strcmp(fname, "_id")==0;
			//if ( n == N && gotId )
			//	break;
		}
		//else if ( strcmp(fname, "_id")==0 ) {
		//	b.append(e);
		//	gotId = true;
		//	if ( n == N && gotId )
		//		break;
		//}
	}

	if ( n ) {
		*this = b.obj();
	}

	return n;
}


BinaryJsonObject BinaryJsonObject::removeField(const StringData& name) const {
	BinaryJsonObjectBuilder b;
	BinaryJsonObjectIterator i(*this);
	while ( i.more() ) {
		BinaryJsonElement e = i.next();
		const char *fname = e.fieldName();
		if( strcmp(name.data(), fname) )
			b.append(e);
	}
	return b.obj();
}
int BinaryJsonObject::nFields() const {
	int n = 0;
	BinaryJsonObjectIterator i(*this);
	while ( i.moreWithEOO() ) {
		BinaryJsonElement e = i.next();
		if ( e.eoo() )
			break;
		n++;
	}
	return n;
}

/* grab names of all the fields in this object */
int BinaryJsonObject::getFieldNames(set<string>& fields) const {
	int n = 0;
	BinaryJsonObjectIterator i(*this);
	while ( i.moreWithEOO() ) {
		BinaryJsonElement e = i.next();
		if ( e.eoo() )
			break;
		fields.insert(e.fieldName());
		n++;
	}
	return n;
}
/* return has eoo() true if no match
supports "." notation to reach into embedded objects
*/
BinaryJsonElement BinaryJsonObject::getFieldDotted(const char *name) const {
	BinaryJsonElement e = getField( name );
	if ( e.eoo() ) {
		const char *p = strchr(name, '.');
		if ( p ) {
			std::string left(name, p-name);
			BinaryJsonObject sub = getObjectField(left.c_str());
			return sub.isEmpty() ? BinaryJsonElement() : sub.getFieldDotted(p+1);
		}
	}

	return e;
}

BinaryJsonElement BinaryJsonObject::getFieldDottedOrArray(const char *&name) const {
	const char *p = strchr(name, '.');

	BinaryJsonElement sub;

	if ( p ) {
		sub = getField( string(name, p-name) );
		name = p + 1;
	}
	else {
		sub = getField( name );
		name = name + strlen(name);
	}

	if ( sub.eoo() )
		return eooElement;
	else if ( sub.type() == JSONArray || name[0] == '\0' )
		return sub;
	else if ( sub.type() == Object )
		return sub.embeddedObject().getFieldDottedOrArray( name );
	else
		return eooElement;
}
BinaryJsonElement BinaryJsonObject::getField(const StringData& name) const {
	BinaryJsonObjectIterator i(*this);
	while ( i.more() ) {
		BinaryJsonElement e = i.next();
		if ( strcmp(e.fieldName(), name.data()) == 0 )
			return e;
	}

	return BinaryJsonElement();
}
void BinaryJsonObject::getFields(unsigned n, const char **fieldNames, BinaryJsonElement *fields) const {
	BinaryJsonObjectIterator i(*this);
	while ( i.more() ) {
		BinaryJsonElement e = i.next();
		const char *p = e.fieldName();
		for( unsigned i = 0; i < n; i++ ) {
			if( strcmp(p, fieldNames[i]) == 0 ) {
				fields[i] = e;
				break;
			}
		}
	}
}

const char * BinaryJsonObject::getStringField(const char *name) const {
	BinaryJsonElement e = getField(name);
	return e.type() == JSONString ? e.valuestr() : "";
}
BinaryJsonObject BinaryJsonObject::getObjectField(const char *name) const {
	BinaryJsonElement e = getField(name);
	BinaryJsonType t = e.type();
	return t == Object || t == JSONArray ? e.embeddedObject() : BinaryJsonObject();
}


//int BinaryJsonObject::getIntField(const char *name) const {
//	BinaryJsonElement e = getField(name);
//	return e.isNumber() ? (int) e.number() : (INT_MIN);
//}

bool BinaryJsonObject::getBoolField(const char *name) const {
	BinaryJsonElement e = getField(name);
	return e.type() == JSONBool ? e.boolean() : false;
}



BinaryJsonObject BinaryJsonObject::extractFieldsUnDotted(const BinaryJsonObject& pattern) const {
	BinaryJsonObjectBuilder b;
	BinaryJsonObjectIterator i(pattern);
	while ( i.moreWithEOO() ) {
		BinaryJsonElement e = i.next();
		if ( e.eoo() )
			break;
		BinaryJsonElement x = getField(e.fieldName());
		if ( !x.eoo() )
			b.appendAs(x, "");
	}
	return b.obj();
}

BinaryJsonObject BinaryJsonObject::extractFields(const BinaryJsonObject& pattern , bool fillWithNull ) const {
	BinaryJsonObjectBuilder b(32); // scanandorder.h can make a zillion of these, so we start the allocation very small
	BinaryJsonObjectIterator i(pattern);
	while ( i.moreWithEOO() ) {
		BinaryJsonElement e = i.next();
		if ( e.eoo() )
			break;
		BinaryJsonElement x = getFieldDotted(e.fieldName());
		if ( ! x.eoo() )
			b.appendAs( x, e.fieldName() );
		else if ( fillWithNull )
			b.appendNull( e.fieldName() );
	}
	return b.obj();
}

BinaryJsonObject BinaryJsonObject::filterFieldsUndotted( const BinaryJsonObject &filter, bool inFilter ) const {
	BinaryJsonObjectBuilder b;
	BinaryJsonObjectIterator i( *this );
	while( i.moreWithEOO() ) {
		BinaryJsonElement e = i.next();
		if ( e.eoo() )
			break;
		BinaryJsonElement x = filter.getField( e.fieldName() );
		if ( ( x.eoo() && !inFilter ) ||
			( !x.eoo() && inFilter ) )
			b.append( e );
	}
	return b.obj();
}

BinaryJsonElement BinaryJsonObject::getFieldUsingIndexNames(const char *fieldName, const BinaryJsonObject &indexKey) const {
	BinaryJsonObjectIterator i( indexKey );
	int j = 0;
	while( i.moreWithEOO() ) {
		BinaryJsonElement f = i.next();
		if ( f.eoo() )
			return BinaryJsonElement();
		if ( strcmp( f.fieldName(), fieldName ) == 0 )
			break;
		++j;
	}
	BinaryJsonObjectIterator k( *this );
	while( k.moreWithEOO() ) {
		BinaryJsonElement g = k.next();
		if ( g.eoo() )
			return BinaryJsonElement();
		if ( j == 0 ) {
			return g;
		}
		--j;
	}
	return BinaryJsonElement();
}

int BinaryJsonObject::woCompare(const BinaryJsonObject &r, const BinaryJsonObject &idxKey,
	bool considerFieldName) const {
		if ( isEmpty() )
			return r.isEmpty() ? 0 : -1;
		if ( r.isEmpty() )
			return 1;

		bool ordered = !idxKey.isEmpty();

		BinaryJsonObjectIterator i(*this);
		BinaryJsonObjectIterator j(r);
		BinaryJsonObjectIterator k(idxKey);
		while ( 1 ) {
			// so far, equal...

			BinaryJsonElement l = i.next();
			BinaryJsonElement r = j.next();
			BinaryJsonElement o;
			if ( ordered )
				o = k.next();
			if ( l.eoo() )
				return r.eoo() ? 0 : -1;
			if ( r.eoo() )
				return 1;

			int x;
			/*
			if( ordered && o.type() == String && strcmp(o.valuestr(), "ascii-proto") == 0 &&
			l.type() == String && r.type() == String ) {
			// note: no negative support yet, as this is just sort of a POC
			x = _stricmp(l.valuestr(), r.valuestr());
			}
			else*/ {
				x = l.woCompare( r, considerFieldName );
				if ( ordered && o.number() < 0 )
					x = -x;
			}
			if ( x != 0 )
				return x;
		}
		return -1;
}
