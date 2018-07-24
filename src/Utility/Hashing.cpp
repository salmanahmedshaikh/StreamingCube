//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Utility/Hashing.h"
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <cmath>
#include <math.h>

Hashing* Hashing::hashing = NULL;

Hashing::Hashing(void)
{
}

Hashing::~Hashing(void)
{
}

Hashing* Hashing::getInstance()
 {
	 if(hashing==NULL)
	 {
		 hashing = new Hashing();
	 }
	 return hashing;
 }

int Hashing::stringHashing(const char* groupByAttribute, int totalActiveWorkers)
{
    /*
    char ch[];
    ch = x.toCharArray();
    int xlength = x.length();

    int i, sum;
    for (sum=0, i=0; i < x.length(); i++)
        sum += ch[i];
    return sum % M;
    */

   const int A = 54059; /* a prime */
   const int B = 76963; /* another prime */
   const int C = 86969; /* yet another prime */

   int h = 31; /* also prime */
   while (*groupByAttribute)
   {
     h = (h * A) ^ (groupByAttribute[0] * B);
     groupByAttribute++;
   }

   return (std::abs(h) % totalActiveWorkers) + 1;
}

// Use folding on a string, sum 2 bytes at a time
// This function takes a string as input. It processes the string 2 bytes at a time, and interprets each of the 2-byte chunks as a single long integer value. The integer values for the four-byte chunks are added together.
// In the end, the resulting sum is converted to the range 0 to M-1 using the modulus operator.
// M is the hash table size
/*int Hashing::twoFoldStringHashing(std::string groupByAttribute, int totalActiveWorkers)
{
     /*
     int intLength = groupByAttribute.length() / 2;
     long sum = 0;

     for (int j = 0; j < intLength; j++)
     {
       char c[] = groupByAttribute.substring(j * 2, (j * 2) + 2).toCharArray();
       long mult = 1;

       for (int k = 0; k < c.length; k++)
       {
            sum += c[k] * mult;
            mult *= 256;
       }
     }

     char c[] = groupByAttribute.substring(intLength * 2).toCharArray();
     long mult = 1;

     for (int k = 0; k < c.length; k++)
     {
       sum += c[k] * mult;
       mult *= 256;
     }

     return round(std::abs(sum) % totalActiveWorkers);

   }
*/
