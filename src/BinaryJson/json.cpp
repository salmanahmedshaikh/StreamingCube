//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"


#define BOOST_SPIRIT_THREADSAFE
#define BOOST_SPIRIT_USE_OLD_NAMESPACE
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_loops.hpp>
#include <boost/spirit/include/classic_lists.hpp>
#include <boost/noncopyable.hpp>
//#include <boost/spirit/utility/grammar_def.hpp>
#include <algorithm>
#include <string>
#include <boost/algorithm/string/erase.hpp>
#include <limits>
#include "../BinaryJson/BinaryJsonObject.h"
#include "../BinaryJson/BinaryJsonElement.h"
#include "../BinaryJson/BinaryJsonObjectBuilder.h"
#include "../BinaryJson/json.h"

using namespace boost::spirit;
using namespace std;


struct ObjectBuilder : boost::noncopyable {
	~ObjectBuilder() {

		unsigned i = builders.size();
		if ( i ) {
			i--;
			for ( ; i>=1; i-- ) {
				if ( builders[i] ) {
					builders[i]->done();
				}
			}
		}

	}
	BinaryJsonObjectBuilder *back() {
		return builders.back().get();
	}
	// Storage for field names of elements within builders.back().
	const char *fieldName() {
		return fieldNames.back().c_str();
	}
	bool empty() const {
		return builders.size() == 0;
	}
	void init() {
		boost::shared_ptr< BinaryJsonObjectBuilder > b( new BinaryJsonObjectBuilder() );
		builders.push_back( b );
		fieldNames.push_back( "" );
		indexes.push_back( 0 );
	}
	void pushObject( const char *fieldName ) {
		boost::shared_ptr< BinaryJsonObjectBuilder > b( new BinaryJsonObjectBuilder( builders.back()->subobjStart( fieldName ) ) );
		builders.push_back( b );
		fieldNames.push_back( "" );
		indexes.push_back( 0 );
	}
	void pushArray( const char *fieldName ) {
		boost::shared_ptr< BinaryJsonObjectBuilder > b( new BinaryJsonObjectBuilder( builders.back()->subarrayStart( fieldName ) ) );
		builders.push_back( b );
		fieldNames.push_back( "" );
		indexes.push_back( 0 );
	}
	BinaryJsonObject pop() {
		BinaryJsonObject ret;
		if ( back()->owned() )
			ret = back()->obj();
		else
			ret = back()->done();
		builders.pop_back();
		fieldNames.pop_back();
		indexes.pop_back();
		return ret;
	}
	void nameFromIndex() {
		fieldNames.back() = BinaryJsonObjectBuilder::numStr( indexes.back() );
	}
	string popString() {
		string ret = ss.str();
		ss.str( "" );
		return ret;
	}
	// Cannot use auto_ptr because its copy constructor takes a non const reference.
	vector< boost::shared_ptr< BinaryJsonObjectBuilder > > builders;
	vector< string > fieldNames;
	vector< int > indexes;
	stringstream ss;
	string ns;


};

struct objectStart {
	objectStart( ObjectBuilder &_b ) : b( _b ) {}
	void operator() ( const char &c ) const {
		if ( b.empty() )
			b.init();
		else
			b.pushObject( b.fieldName() );
	}
	ObjectBuilder &b;
};

struct arrayStart {
	arrayStart( ObjectBuilder &_b ) : b( _b ) {}
	void operator() ( const char &c ) const {
		b.pushArray( b.fieldName() );
		b.nameFromIndex();
	}
	ObjectBuilder &b;
};

struct arrayNext {
	arrayNext( ObjectBuilder &_b ) : b( _b ) {}
	void operator() ( const char &c ) const {
		++b.indexes.back();
		b.nameFromIndex();
	}
	ObjectBuilder &b;
};

struct ch {
	ch( ObjectBuilder &_b ) : b( _b ) {}
	void operator() ( const char c ) const {
		b.ss << c;
	}
	ObjectBuilder &b;
};

struct chE {
	chE( ObjectBuilder &_b ) : b( _b ) {}
	void operator() ( const char c ) const {
		char o = '\0';
		switch ( c ) {
		case '\"':
			o = '\"';
			break;
		case '\'':
			o = '\'';
			break;
		case '\\':
			o = '\\';
			break;
		case '/':
			o = '/';
			break;
		case 'b':
			o = '\b';
			break;
		case 'f':
			o = '\f';
			break;
		case 'n':
			o = '\n';
			break;
		case 'r':
			o = '\r';
			break;
		case 't':
			o = '\t';
			break;
		case 'v':
			o = '\v';
			break;
		default:
			assert( false );
		}
		b.ss << o;
	}
	ObjectBuilder &b;
};


struct chClear {
	chClear( ObjectBuilder &_b ) : b( _b ) {}
	void operator() ( const char c ) const {
		b.popString();
	}
	ObjectBuilder &b;
};

struct fieldNameEnd {
	fieldNameEnd( ObjectBuilder &_b ) : b( _b ) {}
	void operator() ( const char *start, const char *end ) const {
		string name = b.popString();
		//wangyan massert( 10338 ,  "Invalid use of reserved field name: " + name,
		/*name != "$oid" &&
		name != "$binary" &&
		name != "$type" &&
		name != "$date" &&
		name != "$timestamp" &&
		name != "$regex" &&
		name != "$options" );*/
		b.fieldNames.back() = name;
	}
	ObjectBuilder &b;
};

struct unquotedFieldNameEnd {
	unquotedFieldNameEnd( ObjectBuilder &_b ) : b( _b ) {}
	void operator() ( const char *start, const char *end ) const {
		string name( start, end );
		b.fieldNames.back() = name;
	}
	ObjectBuilder &b;
};

struct stringEnd {
	stringEnd( ObjectBuilder &_b ) : b( _b ) {}
	void operator() ( const char *start, const char *end ) const {
		b.back()->append( b.fieldName(), b.popString() );
	}
	ObjectBuilder &b;
};

struct numberValue {
	numberValue( ObjectBuilder &_b ) : b( _b ) {}
	void operator() ( const char *start, const char *end ) const {
		string raw(start);
		double val;

		// strtod isn't able to deal with NaN and inf in a portable way.
		// Correspondingly, we perform the conversions explicitly.

		if ( ! raw.compare(0, 3, "NaN" ) ) {
			val = std::numeric_limits<double>::quiet_NaN();
		}
		else if ( ! raw.compare(0, 8, "Infinity" ) ) {
			val = std::numeric_limits<double>::infinity();
		}
		else if ( ! raw.compare(0, 9, "-Infinity" ) ) {
			val = -std::numeric_limits<double>::infinity();
		}
		else {
			// We re-parse the numeric string here because spirit parsing of strings
			// to doubles produces different results from strtod in some cases and
			// we want to use strtod to ensure consistency with other string to
			// double conversions in our code.

			val = strtod( start, 0 );
		}

		b.back()->append( b.fieldName(), val );
	}
	ObjectBuilder &b;
};

struct intValue {
	intValue( ObjectBuilder &_b ) : b( _b ) {}
	void operator() ( long long num ) const {
		if (num >= (numeric_limits<int>::min)() && num <= (numeric_limits<int>::max)())
			b.back()->append( b.fieldName(), (int)num );
		else
			b.back()->append( b.fieldName(), num );
	}
	ObjectBuilder &b;
};

struct subobjectEnd {
	subobjectEnd( ObjectBuilder &_b ) : b( _b ) {}
	void operator() ( const char *start, const char *end ) const {
		b.pop();
	}
	ObjectBuilder &b;
};

struct arrayEnd {
	arrayEnd( ObjectBuilder &_b ) : b( _b ) {}
	void operator() ( const char *start, const char *end ) const {
		b.pop();
	}
	ObjectBuilder &b;
};

struct trueValue {
	trueValue( ObjectBuilder &_b ) : b( _b ) {}
	void operator() ( const char *start, const char *end ) const {
		b.back()->appendBool( b.fieldName(), true );
	}
	ObjectBuilder &b;
};

struct falseValue {
	falseValue( ObjectBuilder &_b ) : b( _b ) {}
	void operator() ( const char *start, const char *end ) const {
		b.back()->appendBool( b.fieldName(), false );
	}
	ObjectBuilder &b;
};

struct nullValue {
	nullValue( ObjectBuilder &_b ) : b( _b ) {}
	void operator() ( const char *start, const char *end ) const {
		b.back()->appendNull( b.fieldName() );
	}
	ObjectBuilder &b;
};





// One gotcha with this parsing library is probably best ilustrated with an
// example.  Say we have a production like this:
// z = ( ch_p( 'a' )[ foo ] >> ch_p( 'b' ) ) | ( ch_p( 'a' )[ foo ] >> ch_p( 'c' ) );
// On input "ac", action foo() will be called twice -- once as the parser tries
// to match "ab", again as the parser successfully matches "ac".  Sometimes
// the grammar can be modified to eliminate these situations.  Here, for example:
// z = ch_p( 'a' )[ foo ] >> ( ch_p( 'b' ) | ch_p( 'c' ) );
// However, this is not always possible.  In my implementation I've tried to
// stick to the following pattern: store fields fed to action callbacks
// temporarily as ObjectBuilder members, then append to a BinaryJsonObjectBuilder once
// the parser has completely matched a nonterminal and won't backtrack.  It's
// worth noting here that this parser follows a short-circuit convention.  So,
// in the original z example on line 3, if the input was "ab", foo() would only
// be called once.
struct JsonGrammar : public grammar< JsonGrammar > {
public:
	JsonGrammar( ObjectBuilder &_b ) : b( _b ) {}

	template < typename ScannerT >
	struct definition {
		definition( JsonGrammar const &self ) {
			object = ch_p( '{' )[ objectStart( self.b ) ] >> !members >> '}';
			members = list_p((fieldName >> ':' >> value) , ',');
			fieldName =
				str[ fieldNameEnd( self.b ) ] |
				singleQuoteStr[ fieldNameEnd( self.b ) ] |
				unquotedFieldName[ unquotedFieldNameEnd( self.b ) ];
			array = ch_p( '[' )[ arrayStart( self.b ) ] >> !elements >> ']';
			elements = list_p(value, ch_p(',')[arrayNext( self.b )]);
			value =
				str[ stringEnd( self.b ) ] |
				number[ numberValue( self.b ) ] |
				integer |
				array[ arrayEnd( self.b ) ] |
				lexeme_d[ str_p( "true" ) ][ trueValue( self.b ) ] |
				lexeme_d[ str_p( "false" ) ][ falseValue( self.b ) ] |
				lexeme_d[ str_p( "null" ) ][ nullValue( self.b ) ] |
				//lexeme_d[ str_p( "undefined" ) ][ undefinedValue( self.b ) ] |
				singleQuoteStr[ stringEnd( self.b ) ] |
				//date[ dateEnd( self.b ) ] |
				//oid[ oidEnd( self.b ) ] |
				//bindata[ binDataEnd( self.b ) ] |
				//dbref[ dbrefEnd( self.b ) ] |
				//timestamp[ timestampEnd( self.b ) ] |
				//regex[ regexEnd( self.b ) ] |
				object[ subobjectEnd( self.b ) ] ;
			// NOTE lexeme_d and rules don't mix well, so we have this mess.
			// NOTE We use range_p rather than cntrl_p, because the latter is locale dependent.
			str = lexeme_d[ ch_p( '"' )[ chClear( self.b ) ] >>
				*( ( ch_p( '\\' ) >>
				(
				ch_p( 'b' )[ chE( self.b ) ] |
				ch_p( 'f' )[ chE( self.b ) ] |
				ch_p( 'n' )[ chE( self.b ) ] |
				ch_p( 'r' )[ chE( self.b ) ] |
				ch_p( 't' )[ chE( self.b ) ] |
				ch_p( 'v' )[ chE( self.b ) ] |
				//( ch_p( 'u' ) >> ( repeat_p( 4 )[ xdigit_p ][ chU( self.b ) ] ) ) |
				( ~ch_p('x') & (~range_p('0','9'))[ ch( self.b ) ] ) // hex and octal aren't supported
				)
				) |
				( ~range_p( 0x00, 0x1f ) & ~ch_p( '"' ) & ( ~ch_p( '\\' ) )[ ch( self.b ) ] ) ) >> '"' ];

			singleQuoteStr = lexeme_d[ ch_p( '\'' )[ chClear( self.b ) ] >>
				*( ( ch_p( '\\' ) >>
				(
				ch_p( 'b' )[ chE( self.b ) ] |
				ch_p( 'f' )[ chE( self.b ) ] |
				ch_p( 'n' )[ chE( self.b ) ] |
				ch_p( 'r' )[ chE( self.b ) ] |
				ch_p( 't' )[ chE( self.b ) ] |
				ch_p( 'v' )[ chE( self.b ) ] |
				//( ch_p( 'u' ) >> ( repeat_p( 4 )[ xdigit_p ][ chU( self.b ) ] ) ) |
				( ~ch_p('x') & (~range_p('0','9'))[ ch( self.b ) ] ) // hex and octal aren't supported
				)
				) |
				( ~range_p( 0x00, 0x1f ) & ~ch_p( '\'' ) & ( ~ch_p( '\\' ) )[ ch( self.b ) ] ) ) >> '\'' ];

			// real_p accepts numbers with nonsignificant zero prefixes, which
			// aren't allowed in JSON.  Oh well.
			number = strict_real_p | str_p( "NaN" ) | str_p( "Infinity" ) | str_p( "-Infinity" );

			static int_parser<long long, 10,  1, numeric_limits<long long>::digits10 + 1> long_long_p;
			integer = long_long_p[ intValue(self.b) ];

			// We allow a subset of valid js identifier names here.
			unquotedFieldName = lexeme_d[ ( alpha_p | ch_p( '$' ) | ch_p( '_' ) ) >> *( ( alnum_p | ch_p( '$' ) | ch_p( '_'  )) ) ];

			//dbref = dbrefS | dbrefT;
			//dbrefS = ch_p( '{' ) >> "\"$ref\"" >> ':' >>
			//	str[ dbrefNS( self.b ) ] >> ',' >> "\"$id\"" >> ':' >> quotedOid >> '}';
			//dbrefT = str_p( "Dbref" ) >> '(' >> str[ dbrefNS( self.b ) ] >> ',' >>
			//	quotedOid >> ')';

			//timestamp = ch_p( '{' ) >> "\"$timestamp\"" >> ':' >> '{' >>
			//	"\"t\"" >> ':' >> uint_parser<unsigned long long, 10, 1, -1>()[ timestampSecs(self.b) ] >> ',' >>
			//	"\"i\"" >> ':' >> uint_parser<unsigned int, 10, 1, -1>()[ timestampInc(self.b) ] >> '}' >>'}';

			//oid = oidS | oidT;
			//oidS = ch_p( '{' ) >> "\"$oid\"" >> ':' >> quotedOid >> '}';
			//oidT = str_p( "ObjectId" ) >> '(' >> quotedOid >> ')';

			//quotedOid = lexeme_d[ '"' >> ( repeat_p( 24 )[ xdigit_p ] )[ oidValue( self.b ) ] >> '"' ];

			//bindata = ch_p( '{' ) >> "\"$binary\"" >> ':' >>
			//	lexeme_d[ '"' >> ( *( range_p( 'A', 'Z' ) | range_p( 'a', 'z' ) | range_p( '0', '9' ) | ch_p( '+' ) | ch_p( '/' ) ) >> *ch_p( '=' ) )[ binDataBinary( self.b ) ] >> '"' ] >> ',' >> "\"$type\"" >> ':' >>
			//	lexeme_d[ '"' >> ( repeat_p( 2 )[ xdigit_p ] )[ binDataType( self.b ) ] >> '"' ] >> '}';

			//// TODO: this will need to use a signed parser at some point
			//date = dateS | dateT;
			//dateS = ch_p( '{' ) >> "\"$date\"" >> ':' >> uint_parser< Date_t >()[ dateValue( self.b ) ] >> '}';
			//dateT = !str_p("new") >> str_p( "Date" ) >> '(' >> uint_parser< Date_t >()[ dateValue( self.b ) ] >> ')';

			//regex = regexS | regexT;
			//regexS = ch_p( '{' ) >> "\"$regex\"" >> ':' >> str[ regexValue( self.b ) ] >> ',' >> "\"$options\"" >> ':' >> lexeme_d[ '"' >> ( *( alpha_p ) )[ regexOptions( self.b ) ] >> '"' ] >> '}';
			//// FIXME Obviously it would be nice to unify this with str.
			//regexT = lexeme_d[ ch_p( '/' )[ chClear( self.b ) ] >>
			//	*( ( ch_p( '\\' ) >>
			//	( ch_p( '"' )[ chE( self.b ) ] |
			//	ch_p( '\\' )[ chE( self.b ) ] |
			//	ch_p( '/' )[ chE( self.b ) ] |
			//	ch_p( 'b' )[ chE( self.b ) ] |
			//	ch_p( 'f' )[ chE( self.b ) ] |
			//	ch_p( 'n' )[ chE( self.b ) ] |
			//	ch_p( 'r' )[ chE( self.b ) ] |
			//	ch_p( 't' )[ chE( self.b ) ] |
			//	( ch_p( 'u' ) >> ( repeat_p( 4 )[ xdigit_p ][ chU( self.b ) ] ) ) ) ) |
			//	( ~range_p( 0x00, 0x1f ) & ~ch_p( '/' ) & ( ~ch_p( '\\' ) )[ ch( self.b ) ] ) ) >> str_p( "/" )[ regexValue( self.b ) ]
			//>> ( *( ch_p( 'i' ) | ch_p( 'g' ) | ch_p( 'm' ) ) )[ regexOptions( self.b ) ] ];
		}
		rule< ScannerT > object, members, array, elements, value, str, number, integer,
			dbref, dbrefS, dbrefT, timestamp, timestampS, timestampT, oid, oidS, oidT,
			bindata, date, dateS, dateT, regex, regexS, regexT, quotedOid, fieldName,
			unquotedFieldName, singleQuoteStr;
		const rule< ScannerT > &start() const {
			return object;
		}
	};
	ObjectBuilder &b;
};

BinaryJsonObject fromjson( const char *str , int* len) {
	if ( str[0] == '\0' ) {
		if (len) *len = 0;
		return BinaryJsonObject();
	}

	ObjectBuilder b;
	JsonGrammar parser( b );
	parse_info<> result = parse( str, parser, space_p );
	if (len) {
		*len = result.stop - str;
	}
	else if ( !result.full ) {
		int limit = strnlen(result.stop , 10);
		if (limit == -1) limit = 10;
		std::cout<<"the parsing string is : " << std::string(str) << std::endl;
		throw runtime_error("Failure parsing JSON string near: " + string( result.stop, limit ));
	}
	BinaryJsonObject ret = b.pop();
	assert( b.empty() );
	return ret;
}

BinaryJsonObject fromjson( const string &str ) {
	std::string s = str;
	 boost::algorithm::erase_all(s, "\n");
	return fromjson( s.c_str() );
}
