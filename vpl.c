/*
 * MIT License
 *
 * Copyright (c) 2020 vzvca
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <arpa/inet.h>

#include "vpl.h"

/* --------------------------------------------------------------------------
 *  Compile time assertions
 *  - if the compilation fails it means that some assumptions
 *  - made by the code are not valid, so better to stop, it won't work
 * --------------------------------------------------------------------------*/
void __compile_time_assertions__()
{
#define ARRAYDIFF(T) ((char*)&(((T*)0)[1]) - (char*)&(((T*)0)[0]))
  int __compile_time_assert_1__[ (sizeof(rawline_t) != 6) ? -1 : 0 ];
  int __compile_time_assert_2__[ (sizeof(rtphead_t) != (12+2+6)) ? -1 : 0 ];
  int __compile_time_assert_3__[ (ARRAYDIFF(rawline_t) != 6) ? -1 : 0 ];
  int __compile_time_assert_4__[ (ARRAYDIFF(rtphead_t) != (12+2+6)) ? -1 : 0 ];
  int __compile_time_assert_5__[ (htobe32(12345) == 12345) ? -1 : 0 ];
#undef ARRAYDIFF
}

/* pim is used globally */
image_t im1, *pim = &im1;

/* offset for video rendering to framebuffer */
extern int g_x, g_y; 

/* --------------------------------------------------------------------------
 *  Debug helper to dump RTP header
 * --------------------------------------------------------------------------*/
void dump( rtphead_t *rtph )
{
  rawline_t *praw;
  int done = 0;
  
  static char* bin4[16] =
    {
     "0000", "0001", "0010", "0011"
     "0100", "0101", "0110", "0111"
     "1000", "1001", "1010", "1011"
     "1100", "1101", "1110", "1111"
    };
  static char* bin2[4] = {"00", "01", "10", "11"};
  static char* bin1[2] = {"0", "1"};


  printf("---------------------------------------------\n");
  printf("%s.. ....\n", bin2[rtph->v]);
  printf("..%s. ....\n", bin1[rtph->p]);
  printf("...%s ....\n", bin1[rtph->x]);
  printf(".... %s\n", bin4[rtph->cc]);

  printf("%s... ....\n", bin1[rtph->m]);
  printf("Payload : %d\n", rtph->pt);

  printf("Sequence number : %d\n", ntohs(rtph->seq));
  printf("Timespamp : %d\n", ntohl(rtph->timestamp));
  printf("SSID : 0x%x\n", ntohl(rtph->ssrc) );

  printf("Extended sequence number : %d\n", ntohs(rtph->extseq));

  praw = &rtph->lines[0];
  do {
    printf("Length : %d\n", ntohs(praw->length));
    printf("Lineno : %d\n", ntohs((praw->lineno & 0xff7f)));
    printf("Flag f : %d\n", praw->lineno & 0x0080 );
    printf("Offset : %d\n", ntohs((praw->offset & 0xff7f)));
    printf("Flag c : %d\n", praw->offset & 0x0080 );
    
    done = ((praw->offset & 0x0080) == 0);

    praw++;
    
  } while( !done );
}




/* --------------------------------------------------------------------------
 * YCbCrToRGB converts a Y'CbCr triple to an RGB triple.
 *
 *  Stolen from Go language
 *
 * The JFIF specification says:
 *	R = Y' + 1.40200*(Cr-128)
 *	G = Y' - 0.34414*(Cb-128) - 0.71414*(Cr-128)
 *	B = Y' + 1.77200*(Cb-128)
 * https://www.w3.org/Graphics/JPEG/jfif3.pdf says Y but means Y'.
 *
 * Those formulae use non-integer multiplication factors. When computing,
 * integer math is generally faster than floating point math. We multiply
 * all of those factors by 1<<16 and round to the nearest integer:
 *	 91881 = roundToNearestInteger(1.40200 * 65536).
 *	 22554 = roundToNearestInteger(0.34414 * 65536).
 *	 46802 = roundToNearestInteger(0.71414 * 65536).
 *	116130 = roundToNearestInteger(1.77200 * 65536).
 *
 * Adding a rounding adjustment in the range [0, 1<<16-1] and then shifting
 * right by 16 gives us an integer math version of the original formulae.
 *	R = (65536*Y' +  91881 *(Cr-128)                  + adjustment) >> 16
 *	G = (65536*Y' -  22554 *(Cb-128) - 46802*(Cr-128) + adjustment) >> 16
 *	B = (65536*Y' + 116130 *(Cb-128)                  + adjustment) >> 16
 * A constant rounding adjustment of 1<<15, one half of 1<<16, would mean
 * round-to-nearest when dividing by 65536 (shifting right by 16).
 * Similarly, a constant rounding adjustment of 0 would mean round-down.
 *
 * Defining YY1 = 65536*Y' + adjustment simplifies the formulae and
 * requires fewer CPU operations:
 *	R = (YY1 +  91881 *(Cr-128)                 ) >> 16
 *	G = (YY1 -  22554 *(Cb-128) - 46802*(Cr-128)) >> 16
 *	B = (YY1 + 116130 *(Cb-128)                 ) >> 16
 *
 * The inputs (y, cb, cr) are 8 bit color, ranging in [0x00, 0xff]. In this
 * function, the output is also 8 bit color.
 *
 * As mentioned above, a constant rounding adjustment of 1<<15 is a natural
 * choice, but there is an additional constraint: if c0 := YCbCr{Y: y, Cb:
 * 0x80, Cr: 0x80} and c1 := Gray{Y: y} then c0.RGBA() should equal
 * c1.RGBA(). Specifically, if y == 0 then "R = etc >> 8" should yield
 * 0x0000 and if y == 0xff then "R = etc >> 8" should yield 0xffff. If we
 * used a constant rounding adjustment of 1<<15, then it would yield 0x0080
 * and 0xff80 respectively.
 *
 * Note that when cb == 0x80 and cr == 0x80 then the formulae collapse to:
 *	R = YY1 >> 16
 *	G = YY1 >> 16
 *	B = YY1 >> 16
 *
 * The solution is to make the rounding adjustment non-constant, and equal
 * to 257*Y', which ranges over [0, 1<<16-1] as Y' ranges over [0, 255].
 * YY1 is then defined as:
 *	YY1 = 65536*Y' + 257*Y'
 * or equivalently:
 *	YY1 = Y' * 0x10101
 * --------------------------------------------------------------------------*/
uint32_t rgb(uint8_t y, uint8_t cb, uint8_t cr)
{
  int32_t yy1 = ((int32_t) y) * 0x10101;
  int32_t cb1 = ((int32_t) cb) - 128;
  int32_t cr1 = ((int32_t) cr) - 128;

  /*
   * The bit twiddling below is equivalent to
   *
   * r := (yy1 + 91881*cr1) >> 16
   * if r < 0 {
   *     r = 0
   * } else if r > 0xff {
   *     r = ^int32(0)
   * }
   *
   * but uses fewer branches and is faster.
   * Note that the uint8 type conversion in the return
   * statement will convert ^int32(0) to 0xff.
   * The code below to compute g and b uses a similar pattern. 
   *
   */
  int32_t r = yy1 + 91881*cr1;
  if ( (r & 0xff000000) == 0 ) {
    r >>= 16;
  } else {
    r = ~(r >> 31);
  }
  
  int32_t g = yy1 - 22554*cb1 - 46802*cr1;
  if ( (g & 0xff000000) == 0 ) {
    g >>= 16;
  } else {
    g = ~(g >> 31);
  }

  int32_t b = yy1 + 116130*cb1;
  if ( (b & 0xff000000) == 0 ) {
    b >>= 16;
  } else {
    b = ~(b >> 31);
  }

  return ((r & 0xff) << 16) |  ((g & 0xff) << 8) |  (b & 0xff);
}



/* --------------------------------------------------------------------------
 *  Adds a line to the image
 *
 *  Data is expecting to be like this :
 *  Y00 Y01 Y10 Y11 Cb Cr
 *
 *  if __BLACK_AND_WHITE__ is defined decoding is BW
 * --------------------------------------------------------------------------*/
void fill_image(image_t *img, rawline_t *rtph, unsigned char *scanline)
{
  uint16_t lineno = ntohs(rtph->lineno & 0xff7f);
  uint16_t offset = ntohs(rtph->offset & 0xff7f);
  uint32_t i, length = ntohs(rtph->length);

  uint32_t *line0 = img->pixels + g_x + img->stride*(lineno + g_y + 0);
  uint32_t *line1 = img->pixels + g_x + img->stride*(lineno + g_y + 1);
  
  for( i = 0; i < length; i += 6, offset += 2 ) {
    unsigned char *data = scanline + i;

#ifdef __BLACK_AND_WHITE__
    int cbcr;
    *cbcr = (data[4] << 8) | (data[5] << 16) ;
    cbcr = 0;
    
    int pixel00 = cbcr | data[0];
    int pixel01 = cbcr | data[1];
    int pixel10 = cbcr | data[2];
    int pixel11 = cbcr | data[3];

    /* black and white */
    line0[offset+0] = pixel00 << 16 | pixel00 << 8 | pixel00;
    line0[offset+1] = pixel01 << 16 | pixel01 << 8 | pixel01;
    line1[offset+0] = pixel10 << 16 | pixel10 << 8 | pixel10;
    line1[offset+1] = pixel11 << 16 | pixel11 << 8 | pixel11;
#else
    line0[offset+0] = rgb( data[0], data[4], data[5] );
    line0[offset+1] = rgb( data[1], data[4], data[5] );
    line1[offset+0] = rgb( data[2], data[4], data[5] );
    line1[offset+1] = rgb( data[3], data[4], data[5] );
#endif
  }
}

/* --------------------------------------------------------------------------
 *  For debugging
 * --------------------------------------------------------------------------*/
void pktprocess( unsigned char *pkt, int pktlen )
{
  static int loop = 0;
  int i;
  loop++;
  if ( 1 || (loop % 100 == 0) ) {
    dump( (rtphead_t*) pkt );
    printf( "packet len %d\n", pktlen );
    for( i = 0; i < 20; ++i ) {
      printf( "%02x ", pkt[i]);
    }
    puts("");
  }
  if ( loop == 100 ) exit(0);
}

/* --------------------------------------------------------------------------
 *  video packet processing
 * --------------------------------------------------------------------------*/
void imgprocess( unsigned char *cpkt, int pktlen )
{
  rawline_t *praw;
  int i = 0, j, done = 0;
  unsigned char *data;
  rtphead_t *pkt = (rtphead_t*) cpkt;

  /* count lines in packet */
  praw = &pkt->lines[0];
  do {
    done = ((praw->offset & 0x0080) == 0);
    praw ++;
    i ++;
  }
  while( !done );

  /* compute start of data */
  data = (unsigned char *) &pkt->lines[0];
  data += i*sizeof(rawline_t);

  /* loop over lines in packet and add them to image */
  for( j = 0; j < i; ++j ) {
    fill_image( pim, pkt->lines + j, data );
    data += htons( pkt->lines[j].length );
  }
}

