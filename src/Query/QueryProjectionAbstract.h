//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../Query/QueryIntermediateRepresentationInterpreter.h"
#include "../Query/QueryExpressionAbstract.h"
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include "../Common/Types.h"

class QueryExpression;
class QueryProjection;
enum ProjectionType
{
	_OBJECT, _ARRAY, _DIRECT,
};
class QueryProjectionAbstract
{
protected:
	bool needRename;
	std::string rename;
	Document bufferedDocument;//it is used when performProjection return documentelement
public:
	QueryProjectionAbstract(){};
	virtual ~QueryProjectionAbstract(){};
	virtual boost::any performProjection(Document& doc) = 0;
	virtual boost::any performProjection(Document& leftDoc, Document& rightDoc)=0;
	void setNeedRename(bool needRename)
	{
		this->needRename = needRename;
	}
	void setRename(std::string rename)
	{

		this->rename = rename;
	}
};
class ObjectQueryProjection: public QueryProjectionAbstract
{
private:
	std::vector<boost::shared_ptr<QueryProjectionAbstract> > fields;		//for object type

public:
	ObjectQueryProjection(){};
	~ObjectQueryProjection(){};
	void setProjectionFields(std::vector<boost::shared_ptr<QueryProjectionAbstract> > fields)
	{
		this->fields = fields;
	}
	std::vector<boost::shared_ptr<QueryProjectionAbstract> > getProjectionFields(){
		return this->fields;
	}
	//return Document
	//先看vector，里面得到的每一个都是documentelement，这些documentelement能拼成一个docuemnt，
	//如果rename是false，返回的就是这个object，如果rename是true，就和这个object拼成一个documentelement返回
	virtual boost::any performProjection(Document& doc)
	{
		DocumentBuilder documentBuilder;
		std::vector<boost::shared_ptr<QueryProjectionAbstract> >::iterator it;
		for(it = this->fields.begin(); it != this->fields.end(); it++)
		{
			boost::shared_ptr<QueryProjectionAbstract>  queryProjection = *it;
			boost::any projection = queryProjection->performProjection(doc);
			assert(projection.type() == typeid(DocumentElement));
			DocumentElement documentElement = boost::any_cast<DocumentElement>(projection);
			//std::cout<<documentElement<<std::endl;
			documentBuilder.append(documentElement);
		}
		Document retDocument = documentBuilder.obj();
		if(this->needRename == true)
		{// return documentelement
			DocumentBuilder retDocumentBuilder ;
			retDocumentBuilder.appendObject(this->rename,retDocument.objdata());
			bufferedDocument =  retDocumentBuilder.obj();
			return bufferedDocument.firstElement();
		}
		else
		{// return document
			return retDocument;
		}
	};
	virtual boost::any performProjection(Document& leftDoc, Document& rightDoc)
	{
		DocumentBuilder documentBuilder;
		std::vector<boost::shared_ptr<QueryProjectionAbstract> >::iterator it;
		for(it = this->fields.begin(); it != this->fields.end(); it++)
		{
			boost::shared_ptr<QueryProjectionAbstract>  queryProjection = *it;
			boost::any projection = queryProjection->performProjection(leftDoc,rightDoc);
			assert(projection.type() == typeid(DocumentElement));
			DocumentElement documentElement = boost::any_cast<DocumentElement>(projection);
			//std::cout<<documentElement<<std::endl;
			documentBuilder.append(documentElement);
		}
		Document retDocument = documentBuilder.obj();
		if(this->needRename == true)
		{// return documentelement
			DocumentBuilder retDocumentBuilder ;
			retDocumentBuilder.appendObject(this->rename,retDocument.objdata());
			bufferedDocument =  retDocumentBuilder.obj();
			return bufferedDocument.firstElement();
		}
		else
		{// return document
			return retDocument;
		}
	};
};


class ArrayQueryProjection: public QueryProjectionAbstract
{
private:
	std::vector<boost::shared_ptr<QueryProjectionAbstract> > arrayItems;		//for array type
public:
	ArrayQueryProjection(){};
	~ArrayQueryProjection(){};
	void setArrayItems(std::vector<boost::shared_ptr<QueryProjectionAbstract> > arrayItems)
	{
		this->arrayItems = arrayItems;
	}
	//return array
	virtual boost::any performProjection(Document& doc)
	{
		//ArrayBuilder arrayBuilder;
		//std::vector<boost::shared_ptr<QueryProjectionAbstract> >::iterator it;
		//for(it = this->arrayItems.begin(); it != this->arrayItems.end(); it++)
		//{
		//	boost::shared_ptr<QueryProjectionAbstract>  queryProjection = *it;
		//	boost::any projection = queryProjection->performProjection(doc);
		//	assert(projection.type() == typeid(DocumentElement));
		//	DocumentElement documentElement = boost::any_cast<DocumentElement>(projection);
		//	arrayBuilder.append(documentElement);
		//}
		//Document doc = documentBuilder.obj();
		//if(this->needRename == true)
		//{// return documentelement
		//	DocumentBuilder retDocumentBuilder ;
		//	retDocumentBuilder.appendObject(this->rename,doc.objdata());
		//	return retDocumentBuilder.obj().firstElement();
		//}
		//else
		//{// return document
		//	return doc;
		//}
	};
	boost::any performProjection(Document& leftDoc, Document& rightDoc)
	{

	}
};


class DirectQueryProjection: public QueryProjectionAbstract
{
private:
	boost::shared_ptr<QueryExpressionAbstract> expression;		//for direct type
public:
	DirectQueryProjection(){};
	~DirectQueryProjection(){};
	void setExpression(boost::shared_ptr<QueryExpressionAbstract> expression)
	{
		this->expression = expression;
	}

	const boost::shared_ptr<QueryExpressionAbstract> getExpression() const {
		return expression;
	}

	//return DocumentElement
	virtual boost::any performProjection(Document& doc)
	{
		if(this->needRename == true)
		{
			boost::any value = this->expression->getValue(doc);
			if(value.type() == typeid(int))
			{
				int i = boost::any_cast<int>(value);
				DocumentBuilder builder;
				builder.appendNumber(this->rename, i);
				this->bufferedDocument =  builder.obj();

				return this->bufferedDocument.firstElement();
			}
			else if(value.type() == typeid(double))
			{
				double dl = boost::any_cast<double>(value);
				DocumentBuilder builder;
				builder.appendNumber(this->rename, dl);
				this->bufferedDocument =  builder.obj();

				return this->bufferedDocument.firstElement();
			}
			else if(value.type() == typeid(std::string))
			{
				std::string str = boost::any_cast<std::string>(value);
				DocumentBuilder builder;
				builder.append(this->rename, str);

				this->bufferedDocument =  builder.obj();

				return this->bufferedDocument.firstElement();
			}
			else if(value.type() == typeid(const char*))
			{
				std::string str = boost::any_cast<const char*>(value);
				DocumentBuilder builder;
				builder.append(this->rename, str);

				this->bufferedDocument =  builder.obj();

				return this->bufferedDocument.firstElement();
			}
		}
		else if(this->needRename == false)
		{
			assert(typeid(*this->expression) == typeid(IdentifierQueryExpression));
			boost::shared_ptr<IdentifierQueryExpression> identifierQueryExpression = boost::dynamic_pointer_cast<IdentifierQueryExpression>(this->expression);
			std::string attributeName = identifierQueryExpression->getLastAttribute();
			boost::any value = this->expression->getValue(doc);
			if(value.type() == typeid(int))
			{
				int i = boost::any_cast<int>(value);
				DocumentBuilder builder;
				builder.appendNumber(attributeName, i);
				this->bufferedDocument =  builder.obj();

				return this->bufferedDocument.firstElement();
			}
			else if(value.type() == typeid(double))
			{
				double dl = boost::any_cast<double>(value);
				DocumentBuilder builder;
				builder.appendNumber(attributeName, dl);
				this->bufferedDocument =  builder.obj();

				return this->bufferedDocument.firstElement();
			}
			else if(value.type() == typeid(std::string))
			{
				std::string str = boost::any_cast<std::string>(value);
				DocumentBuilder builder;
				builder.append(attributeName, str);
				this->bufferedDocument =  builder.obj();

				return this->bufferedDocument.firstElement();
			}
			else if(value.type() == typeid(const char*))
			{
				std::string str = boost::any_cast<const char*>(value);
				DocumentBuilder builder;
				builder.append(attributeName, str);
				this->bufferedDocument =  builder.obj();

				return this->bufferedDocument.firstElement();
			}
		}
	};

	virtual boost::any performProjection(Document& leftDoc, Document& rightDoc)
	{

		assert(typeid(*this->expression) == typeid(IdentifierQueryExpression));
		boost::shared_ptr<IdentifierQueryExpression> identifierQueryExpression = boost::dynamic_pointer_cast<IdentifierQueryExpression>(this->expression);
		std::string attributeName = identifierQueryExpression->getLastAttribute();
		boost::any value = identifierQueryExpression->getValue(leftDoc,rightDoc);
		if(this->needRename == true)
		{
			if(value.type() == typeid(int))
			{
				int i = boost::any_cast<int>(value);
				DocumentBuilder builder;
				builder.appendNumber(this->rename, i);
				this->bufferedDocument =  builder.obj();

				return this->bufferedDocument.firstElement();
			}
			else if(value.type() == typeid(double))
			{
				double dl = boost::any_cast<double>(value);
				DocumentBuilder builder;
				builder.appendNumber(this->rename, dl);
				this->bufferedDocument =  builder.obj();

				return this->bufferedDocument.firstElement();
			}
			else if(value.type() == typeid(std::string))
			{
				std::string str = boost::any_cast<std::string>(value);
				DocumentBuilder builder;
				builder.append(this->rename, str);
				this->bufferedDocument =  builder.obj();

				return this->bufferedDocument.firstElement();
			}
			else if(value.type() == typeid(const char*))
			{
				std::string str = boost::any_cast<const char*>(value);
				DocumentBuilder builder;
				builder.append(this->rename, str);
				this->bufferedDocument =  builder.obj();

				return this->bufferedDocument.firstElement();
			}
		}
		else if(this->needRename == false)
		{
			if(value.type() == typeid(int))
			{
				int i = boost::any_cast<int>(value);
				DocumentBuilder builder;
				builder.appendNumber(attributeName, i);
				this->bufferedDocument =  builder.obj();

				return this->bufferedDocument.firstElement();
			}
			else if(value.type() == typeid(double))
			{
				double dl = boost::any_cast<double>(value);
				DocumentBuilder builder;
				builder.appendNumber(attributeName, dl);
				this->bufferedDocument =  builder.obj();

				return this->bufferedDocument.firstElement();
			}
			else if(value.type() == typeid(std::string))
			{
				std::string str = boost::any_cast<std::string>(value);
				DocumentBuilder builder;
				builder.append(attributeName, str);
				this->bufferedDocument =  builder.obj();

				return this->bufferedDocument.firstElement();
			}
			else if(value.type() == typeid(const char*))
			{
				std::string str = boost::any_cast<const char*>(value);
				DocumentBuilder builder;
				builder.append(attributeName, str);
				this->bufferedDocument =  builder.obj();

				return this->bufferedDocument.firstElement();
			}
		}
	};
};
