/* *compress_helper.c 

* DAVID NGETICH AND JAMES GARIJO-GARDE

*/

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <seq.h>
#include <assert.h>
#include "compress_helper.h"

const int BSIZE = 2; /* 2 by 2 blocks */

Pnm_ppm dimen_check(Pnm_ppm image)
{
        if((image->height % 2 == 1) || (image->width % 2 == 1))
        {
                if ((image->height) % 2 == 1) {
                        image->height = image->height-1;
                        
                }
                if (image->width % 2 == 1) {
                        image->width = image->width-1;
                }
                A2Methods_UArray2 array =
                        image->methods->new_with_blocksize(image->width, 
                        image->height, image->methods->size(image->pixels), 2);
                for (unsigned h = 0; h < image->height; h++) {
                        for (unsigned w = 0; w < image->width; w++) {
                                *(Pnm_rgb)(image->methods->at(array, w, h)) =
                                *(Pnm_rgb)(image->methods->at(image->pixels, w,
                                        h));
                        }
                }
                image->methods->free(&(image->pixels));
                image->pixels = array;
        }

        return image;
}

struct Component *rgb_to_cpnt(unsigned denom, unsigned r, unsigned g,
        unsigned b)
{
        float r_d = ((float)r)/((float)denom);  // look into math
        if (r_d > 1) {
                r_d = 1;
        }
        if (r_d < 0) {
                r_d = 0;
        }
        float g_d = ((float)g)/((float)denom);
        if (g_d > 1) {
                g_d = 1;
        }
        if (g_d < 0) {
                g_d = 0;
        }
        float b_d = ((float)b)/((float)denom);
        if (b_d > 1) {
                b_d = 1;
        }
        if (b_d < 0) {
                b_d = 0;
        }

        struct Component *output = malloc(sizeof(struct Component));
        output->y =  ((0.299 * r_d) + (0.587 * g_d) + (0.114 * b_d));
        output->pb = (-0.168736 * r_d) - (0.331264 * g_d) + (0.5 * b_d);
        output->pr = (0.5 * r_d) - (0.418688 * g_d) - (0.081312 * b_d);
        return output;
}

void block_compress(Pnm_ppm image)
{
        printf("COMP40 Compressed image format 2\n%u %u\n", image->width,
                image->height);

        /* step a */

        /*iterate through the entire row */
        for(unsigned block_c = 0; block_c < image->width/BSIZE; block_c++)
        {
                /*iterate through the columns */
                for(unsigned block_r = 0; block_r < image->height/BSIZE;
                        block_r++)
                {
                    
                    int count = 0;
                    struct Component *pixel1 = malloc(sizeof(struct Component));
                    struct Component *pixel2 = malloc(sizeof(struct Component));
                    struct Component *pixel3 = malloc(sizeof(struct Component));
                    struct Component *pixel4 = malloc(sizeof(struct Component));
                    
                    Seq_T pixel = Seq_seq(pixel1, pixel2, pixel3, pixel4,
                        NULL);

                     for (int inner_row = 0; inner_row < BSIZE; inner_row++) {
                        for (int inner_col = 0; inner_col < BSIZE; inner_col++)
                        {
                                unsigned r_index = block_r * BSIZE + inner_row;
                                unsigned c_index = block_c * BSIZE + inner_col;
                                
                                if (r_index < image->height &&
                                        c_index < image->width) {

                                        Pnm_rgb spot =
                                     (Pnm_rgb)(image->methods->at(image->pixels,
                                                c_index, r_index));

                                        Seq_put(pixel, count,
                                                rgb_to_cpnt(image->denominator,
                                        spot->red, spot->green, spot->blue));
                                        count++;
                                }
                        }
                     }
                     count = 0;
                     /* the averaging */

                     float pb_avg = 0;
                     float pr_avg = 0;
                     for (int i = 0; i < BSIZE*BSIZE; i++) {
                        pb_avg = pb_avg +
                        (*(struct Component *)Seq_get(pixel, i)).pb;
                        pr_avg = pr_avg +
                        (*(struct Component *)Seq_get(pixel, i)).pr;
                     }
                     pb_avg = pb_avg/(BSIZE*BSIZE);
                     pr_avg = pr_avg/(BSIZE*BSIZE);

                     /* step b */
                     unsigned pb4 = Arith40_index_of_chroma(pb_avg);
                     unsigned pr4 = Arith40_index_of_chroma(pr_avg);

                     float y1 = (*(struct Component *)Seq_get(pixel, 0)).y;
                     float y2 = (*(struct Component *)Seq_get(pixel, 1)).y;
                     float y3 = (*(struct Component *)Seq_get(pixel, 2)).y;
                     float y4 = (*(struct Component *)Seq_get(pixel, 3)).y;

                     /* step c */
                     float a = ((y4 + y3 + y2 + y1) * 31) / 4.0;
                     float b = ((y4 + y3 - y2 - y1) * 103.3) / 4.0;
                     float c = ((y4 - y3 + y2 - y1) * 103.3) / 4.0;
                     float d = ((y4 - y3 - y2 +  y1) * 103.3) / 4.0;

                     /* step d */
                     uint16_t a_q = round(a);
                     int16_t b_q = round(b);
                     if(b_q < -15)
                     {
                        b_q = -15;
                     } else if (b_q > 15) {
                        b_q = 15;
                     }
                     int16_t c_q = round(c);
                     if(c_q < -15)
                     {
                        c_q = -15;
                     } else if (c_q > 15) {
                        c_q = 15;
                     }
                     int16_t d_q = round(d);
                     if(d_q < -15) {
                        d_q = -15;
                     } else if (d_q > 15) {
                        d_q = 15;
                     }
                    
                     /* step e */
                     uint32_t packed = 0;

                     packed = Bitpack_newu(packed, 6, 26, a_q);
                     packed = Bitpack_news(packed, 6, 20, b_q);
                     packed = Bitpack_news(packed, 6, 14, c_q);
                     packed = Bitpack_news(packed, 6, 8, d_q);
                     packed = Bitpack_newu(packed, 4, 4, pb4);
                     packed = Bitpack_newu(packed, 4, 0, pr4);
                     
   
                     for (int i = 24; i >= 0; i = i - 8) {
                        putchar(Bitpack_getu(packed, 8, i));
                     }

                     free(pixel1);
                     free(pixel2);
                     free(pixel3);
                     free(pixel4);
                     for(int l = 0; l < 4; l++)
                         free(Seq_remhi(pixel));

                     Seq_free(&pixel);
                }
        }
}

Seq_T convert_to_cpnt(int64_t word)
{
        float a = (float)Bitpack_getu(word, 6, 26);
        float b = (float)Bitpack_gets(word, 6, 20);
        float c = (float)Bitpack_gets(word, 6, 14);
        float d = (float)Bitpack_gets(word, 6, 8);
        float pb = Arith40_chroma_of_index(Bitpack_getu(word, 4,4));
        float pr = Arith40_chroma_of_index(Bitpack_getu(word, 4,0));

        a = a/31;
        b = (b/103.3);
        c = (c/103.3);
        d = (d/103.3);

        float Y1 = a - b -c + d;
        float Y2 = a - b + c - d;
        float Y3 = a + b - c - d;
        float Y4 = a + b + c + d;

        struct Component *pixel1 = malloc(sizeof(struct Component));
        struct Component *pixel2 = malloc(sizeof(struct Component));
        struct Component *pixel3 = malloc(sizeof(struct Component));
        struct Component *pixel4 = malloc(sizeof(struct Component));

        pixel1->y = Y4;
        pixel1->pb = pb; 
        pixel1->pr = pr;

        pixel2->y = Y3;
        pixel2->pb = pb; 
        pixel2->pr = pr;

        pixel3->y = Y2;
        pixel3->pb = pb; 
        pixel3->pr = pr;

        pixel4->y = Y1;
        pixel4->pb = pb; 
        pixel4->pr = pr;


        Seq_T pixel = Seq_seq(pixel1, pixel2, pixel3, pixel4, NULL);
        assert(pixel);

        return pixel;
}

struct Pnm_rgb convert_to_rgb(unsigned denom, float y, float pb, float pr)
{       
        if (y > 1) {
                y = 1;
        }
        if (y < 0) {
                y = 0;
        }
        if (pb > 0.5) {
                pb = 0.5;
        }
        if (pb < -0.5) {
                pb = -0.5;
        }
        if (pr > 0.5) {
                pr = 0.5;
        }
        if (pr < -0.5) {
                pr = -0.5;
        }
        struct Pnm_rgb output /*= malloc(sizeof(struct Pnm_rgb))*/;

        float r = ((1.0 * y) + (0.0 * pb) + (1.402 * pr))*denom;
        float g = ((1.0 * y) - (0.344136 * pb) - (0.714136 * pr))*denom;
        float b = ((1.0 * y) + (1.772 * pb) + (0.0 * pr))*denom;

        output.red = r;
        output.green = g;
        output.blue = b;

        if (r > denom) {
                output.red = denom;
        }
        if (g > denom) {
                output.green = denom;
        }
        if (b > denom) {
                output.blue = denom;
        }


        if (r < 0) {
                output.red = 0;
        }
        if (g < 0) {
                output.green = 0;
        }
        if (b < 0) {
                output.blue = 0;
        }

        return output;
}