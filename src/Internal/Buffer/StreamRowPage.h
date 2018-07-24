//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../../Common/stdafx.h"
#include "../../Internal/Buffer/BufferConstraint.h"
#include "../../Internal/Row/Row.h"
#include "../../Internal/Memory/MemoryManager.h"
#include "../../Internal/Buffer/BufferConstraint.h"

/*
A StreamRowPage is a part of continuous memory
this page is obtained from the buffer manager
one page is divided into row_chunks
all row_chunks are connected by pointers
Pointer is saved at the chunk header inside each chunk

each chunk contains two parts:
(1)the first is "chunk header", save the position of the next chunk
(2)the second is the remaining part of the chunk, save any information
*/

class StreamRowPage
{
private:
	char* buffer;

public:
	StreamRowPage(void)
	{
		this->buffer = MemoryManager::getInstance()->getOnePage();
	}

	~StreamRowPage(void){}

	inline void setPointerOfNextChunk(CHUNK_INDEX_TYPE chunk, MemoryPosition& memoryPosition)
	{
		char* buffer = this->buffer;
		int p = 0;
		*(PAGE_INDEX_TYPE*)(buffer + chunk* ROW_CHUNK_SIZE + p) = memoryPosition.pageIndex;

		//memcpy(buffer + chunk* CHUNK_SIZE + p, &memoryPosition.pageIndex, sizeof(PAGE_INDEX_TYPE));
		p += sizeof(memoryPosition.pageIndex);

		*(PAGE_INDEX_TYPE*)(buffer + chunk* ROW_CHUNK_SIZE + p) = memoryPosition.chunkIndex;
		//memcpy(buffer+ chunk* CHUNK_SIZE + p, &memoryPosition.chunkIndex, sizeof(CHUNK_INDEX_TYPE));
	}

	inline void getPointerOfNextChunk(CHUNK_INDEX_TYPE chunk, MemoryPosition& memoryPosition)
	{
		char* buffer = this->buffer;
		int p = 0;

		memoryPosition.pageIndex = *(PAGE_INDEX_TYPE*)(buffer + chunk* ROW_CHUNK_SIZE + p);
		p += sizeof(PAGE_INDEX_TYPE);
		memoryPosition.chunkIndex = *(CHUNK_INDEX_TYPE*)(buffer + chunk* ROW_CHUNK_SIZE + p);
	}

	inline void setValue( CHUNK_INDEX_TYPE chunkIndex, Row& row)
	{
		assert(row.getSize() <= ROW_CHUNK_SIZE);
		assert(chunkIndex <= NUMBER_OF_ROW_CHUNKS_PER_PAGE);

		char* chunkPostion = this->buffer + chunkIndex*ROW_CHUNK_SIZE;

		int p= sizeof(CHUNK_INDEX_TYPE)+sizeof(PAGE_INDEX_TYPE);

		*(Timestamp*)(chunkPostion + p) = row.timestamp;
		//memcpy(chunkPostion + p , &element.timestamp,TIMESTAMP_SIZE);
		p += TIMESTAMP_SIZE;
		//memcpy(chunkPostion + p, &element.id, DOCUMENT_IDENTIFIER_SIZE);
		//*(DocumentId*)(chunkPostion + p) = row.id;
		//p += DOCUMENT_IDENTIFIER_SIZE;
		//memcpy(chunkPostion + p, &element.mark, MARK_SIZE);
		*(FactValue*)(chunkPostion + p) = row.factValue;
		p += FACT_VALUE_SIZE;

        //Storing the dimensionkey vector size
        *(int*)(chunkPostion + p) = row.dimensionKey.size();
        p += sizeof(int);

		for(int i = 0; i < row.dimensionKey.size(); i++)
		{
            *(DimensionKey*)(chunkPostion + p) = row.dimensionKey[i];
            p += DIMENSION_KEY_SIZE;
		}
		//memcpy(chunkPostion + p, element.document.objdata(),element.document.objsize());
		//memcpy(chunkPostion + p, row.dimensionKey, sizeof(std::vector<DimensionKey>) + (DIMENSION_KEY_SIZE * row.dimensionKey.size()));
	}

	inline void getValue( CHUNK_INDEX_TYPE chunkIndex, Row& row)
	{
        row.dimensionKey.clear();
		assert(chunkIndex<=NUMBER_OF_ROW_CHUNKS_PER_PAGE);

		char* chunkPostion = this->buffer + chunkIndex*ROW_CHUNK_SIZE;
		int p= sizeof(CHUNK_INDEX_TYPE)+sizeof(PAGE_INDEX_TYPE);

		row.timestamp = *(Timestamp*)(chunkPostion+p);
		p += TIMESTAMP_SIZE;

		//row.id = *(DocumentId*)(chunkPostion+p);
		//p += DOCUMENT_IDENTIFIER_SIZE;

		row.factValue = *(FactValue*) (chunkPostion+p);
		p += FACT_VALUE_SIZE;

		//First getting the dimension key vector size
		int dimKeyVectorSize = *(int*)(chunkPostion + p);
		p += sizeof(int);

		//Next pipulating the dimension key vector
		for(int i = 0; i < dimKeyVectorSize; i++)
		{
            row.dimensionKey.push_back(*(DimensionKey*)(chunkPostion + p));
            p += DIMENSION_KEY_SIZE;
		}

		//std::cout << "Get Value called and returned row " << row << std::endl;
		//element.document = Document(chunkPostion+p);
		//row.dimensionKey = *(FactValue*)(chunkPostion+p);
	}
};

