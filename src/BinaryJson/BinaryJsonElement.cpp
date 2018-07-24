//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"

#include "../BinaryJson/BinaryJsonElement.h"
#include "../BinaryJson/BinaryJsonObject.h"
#include "../BinaryJson/BinaryJsonTypes.h"
#include "../BinaryJson/Builder.h"
#include "../BinaryJson/BinaryJsonObjectBuilder.h"
#include "../BinaryJson/BinaryJsonObjectIterator.h"
#include "../BinaryJson/BinaryInline.h"






 int compareElementValues(const BinaryJsonElement& l, const BinaryJsonElement& r) {
        int f;

        switch ( l.type() ) {
        case EOO:

        case jstNULL:

                f = l.canonicalType() - r.canonicalType();
                if ( f<0 ) return -1;
                return f==0 ? 0 : 1;
        case JSONBool:
                return *l.value() - *r.value();


        case NumberLong:
                if( r.type() == NumberLong ) {
                        long long L = l._numberLong();
                        long long R = r._numberLong();
                        if( L < R ) return -1;
                        if( L == R ) return 0;
                        return 1;
                }

        case NumberInt:
                if( r.type() == NumberInt ) {
                        int L = l._numberInt();
                        int R = r._numberInt();
                        if( L < R ) return -1;
                        return L == R ? 0 : 1;
                }
                // else fall through
        case NumberDouble:
                {
                        double left = l.number();
                        double right = r.number();
                        if( left < right )
                                return -1;
                        if( left == right )
                                return 0;
                        if( left!=left )
                                return right!=right ? 0 : -1;
                        return 1;
                }

        case JSONString:
                /* todo: a utf sort order version one day... */
                {
                        // we use memcmp as we allow zeros in UTF8 strings
                        int lsz = l.valuestrsize();
                        int rsz = r.valuestrsize();
                        int common = (std::min)(lsz, rsz);
                        int res = memcmp(l.valuestr(), r.valuestr(), common);
                        if( res )
                                return res;
                        // longer string is the greater one
                        return lsz-rsz;
                }
        case Object:
        case JSONArray:
                return l.embeddedObject().woCompare( r.embeddedObject() );

        default:
                assert( false);
        }
        return -1;
}



 int BinaryJsonElement::woCompare(  BinaryJsonElement const &e,
        bool considerFieldName ) const {
                int lt = (int) canonicalType();
                int rt = (int) e.canonicalType();
                int x = lt - rt;
                if( x != 0 && (!isNumber() || !e.isNumber()) )
                        return x;
                if ( considerFieldName ) {
                        x = strcmp(fieldName(), e.fieldName());
                        if ( x != 0 )
                                return x;
                }
                x = compareElementValues(*this, e);
                return x;
}
double BinaryJsonElement::numberDouble() const {
	switch( type() ) {
	case NumberDouble:
		return _numberDouble();
	case NumberInt:
		return *reinterpret_cast< const int* >( value() );
	case NumberLong:
		return (double) *reinterpret_cast< const long long* >( value() );
	default:
		return 0;
	}
}
