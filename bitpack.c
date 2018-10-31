/* David Ngetich and James Garijo

* bitpack.c

 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "except.h"
#include "bitpack.h"

extern Except_T Bitpack_Overflow;
Except_T Bitpack_Overflow = {"Overflow packing bits"};

/*  this function tells if the argument passed in, n, can be represented in
*   width bits of unsigned integers in that range 
*/
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
        if (n < pow(2, width)) {
                return true;
        } else {
                return false;
        }
}


/* this function tells if the argument passed in, n, can be represented as  
*  in width bits of signed integers in that range 
*/
bool Bitpack_fitss( int64_t n, unsigned width)
{

        if (n >= (pow(2, width)/-2) && n < (pow(2, width)/2)) {
                return true;
        } else {
                return false;
        }

}

/*  extracts the unsigned field from a word given the width and the field 
*   and the locations of the field’s least significant bit 
*/
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        assert(width <= 64);
        assert((width + lsb) <= 64);

        if(width == 0){
                return 0;
        }

        uint64_t mask = ~0;
        mask = mask << width;
        mask = ~mask;
        word = word >> lsb;
        word = word & mask;

        return word;
}

/*  extracts the signed field from a word given the width and the 
*   field and the locations of the field’s least significant bit 
*/
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{

        assert(width <= 64);
        assert((width + lsb) <= 64);

        if(width == 0){
                return 0;
        }

        uint64_t mask = ~0;
        mask = mask << (64 - (width + lsb));
        mask = mask >> (64 - width) << lsb;
        int64_t w2 = word & mask;
        w2 = w2 << (64 - (width + lsb));
        //word2 = word << (64 - (width + lsb));
        w2 = w2 >> (64 - width);


        return w2;

}

/*  function returns a new unsigned word that is identical to the original word 
* except that the field of width, w, with least significant bits at lsb will 
* have been replaced by a width-bit representation of value
*/
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, uint64_t 
        value)
{
        assert(width <= 64);
        assert((width + lsb) <= 64);

        if(!Bitpack_fitsu(value, width))
        {
            RAISE(Bitpack_Overflow);
        }

        uint64_t original, mask;
        original = word;
        word = value << lsb;
        mask = ~0;
        mask = mask << width;
        mask = ~mask;
        mask = mask << lsb;
        mask = ~mask;
        original = original & mask;
        word = original | word;

        return word;
}

/*  function returns a signed word that is identical to the original word except
* that the field of width, width, with least significant bits at lsb will have
* been replaced by a width-bit representation of value 
*/
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb,  int64_t 
        value)
{       

        assert(width <= 64);
        assert((width + lsb) <= 64);

        if(!Bitpack_fitss(value, width))
        {
            RAISE(Bitpack_Overflow);
        }

        value = abs(value);

        uint64_t original, mask;
        original = word;
        word = value << lsb;
        mask = ~0;
        mask = mask << width;
        mask = ~mask;
        mask = mask << lsb;
        mask = ~mask;
        original = original & mask;
        word = original | word;

        return word;

}






