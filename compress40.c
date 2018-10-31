/*David Ngetich and James Garijo */



#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "pnm.h"
#include "a2methods.h"
#include "a2plain.c"
#include "compress40.h"
#include "bitpack.h"
#include "compress_helper.h"

const int BLOCK_SIZE = 2; /* 2 by 2 blocks */

/* reads PPM, writes compressed image */
void compress40(FILE *input)
{
        A2Methods_T methods = uarray2_methods_plain;
        assert(methods);
        Pnm_ppm pic = Pnm_ppmread(input, methods);
        assert(pic);

        dimen_check(pic);      
        
        block_compress(pic);

        Pnm_ppmfree(&pic);
}


/* reads compressed image, writes PPM */
void decompress40(FILE *input)
{
        unsigned height, width;
        int read = fscanf(input, "COMP40 Compressed image format 2\n%u %u",
                &width, &height);
        assert(read == 2);
        int c = getc(input);
        assert(c == '\n');

        unsigned denominator = 255;

        A2Methods_T methods = uarray2_methods_plain;
        assert(methods);

        A2Methods_UArray2 array = methods->new(width, height,
                sizeof(struct Pnm_rgb));

        struct Pnm_ppm pixmap = { .width = width, .height = height
                                , .denominator = denominator, .pixels = array
                                , .methods = methods
                                };

        for(unsigned block_c = 0; block_c < width/BLOCK_SIZE; block_c++)
        {
                /*iterate through the columns */
                for(unsigned block_r = 0; block_r < height/BLOCK_SIZE;
                        block_r++)
                {
                        /* read in a word */
                        uint32_t word = 0;
                        for (int i = 24; i >= 0; i-=8) {
                                word = Bitpack_newu(word, 8, i, fgetc(input));
                        }


                        Seq_T block = convert_to_cpnt(word);

                        int i = 0;

                        for (int inner_row = 0; inner_row < BLOCK_SIZE;
                                inner_row++) {
                        for (int inner_col = 0; inner_col < BLOCK_SIZE;
                                inner_col++) {
                          unsigned r_index = block_r * BLOCK_SIZE + inner_row;
                          unsigned c_index = block_c * BLOCK_SIZE + inner_col;

                          if (r_index < height && c_index < width) {
                            float y =
                              (*(struct Component *)Seq_get(block, i)).y;
                            float pb =
                              (*(struct Component *)Seq_get(block, i)).pb;
                            float pr =
                              (*(struct Component *)Seq_get(block, i)).pr;
                        *(struct Pnm_rgb *)methods->at(pixmap.pixels, c_index,
                                r_index) = convert_to_rgb(denominator, y, pb,
                                  pr);
                            i++;
                          }
                        }
                        }
                        for(int l = 0; l < 4; l++)
                                free(Seq_remhi(block));

                        Seq_free(&block);
                }
        }

        Pnm_ppmwrite(stdout, &pixmap);
        methods->free(&(pixmap.pixels));
}
