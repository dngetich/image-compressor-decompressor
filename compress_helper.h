#include <stdbool.h>
#include <stdint.h>
#include "except.h"
#include "pnm.h"
#include "arith40.h"
#include "bitpack.h"
#include "seq.h"

#ifndef COMPRESS_HELPER_INCLUDED
#define COMPRESS_HELPER_INCLUDED

#define A2 A2Methods_UArray2

struct Component {
        float y;
        float pb;
        float pr;
};

struct closure {
        Seq_T seq;
        unsigned denom;
};

Pnm_ppm dimen_check(Pnm_ppm image);

struct Component *rgb_to_cpnt(unsigned denom, unsigned r, unsigned g, 
        unsigned b);

void block_compress(Pnm_ppm image);

Seq_T convert_to_cpnt(int64_t word);

struct Pnm_rgb convert_to_rgb(unsigned denom, float y, float pb, float pr);

void apply_map_block(int i, int j, A2 array, A2Methods_Object *ptr, void *cl);

#endif