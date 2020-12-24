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

#ifndef __VPL_H__
#define __VPL_H__

#include <stdint.h>

/*
 * --------------------------------------------------------------------------
 *  Definition d'une image
 *  On part du principe que les pixels occupent 4 octets
 *  ce qui est le cas sur les cibles matérielles que nous utilisont
 * --------------------------------------------------------------------------
 */
struct image_s {
  uint32_t w, h, stride;
  uint32_t  *pixels;
};
typedef struct image_s image_t;

/*
 * --------------------------------------------------------------------------
 *  Header d'une scanline du payload RAW
 * --------------------------------------------------------------------------
 */
struct rawline_s {
  uint16_t length;
  uint16_t lineno;
  uint16_t offset;
};
typedef struct rawline_s rawline_t;

/*
 * --------------------------------------------------------------------------
 *  RTP header with RAW RTP payload header appended 
 * --------------------------------------------------------------------------
 */
struct rtphead_s {
  /* RTP header */
  /* byte 1     */
  uint8_t cc : 4;
  uint8_t x  : 1;
  uint8_t p  : 1;
  uint8_t v  : 2;
  /* byte 2     */
  uint8_t pt : 7;
  uint8_t m  : 1;
  /* byte 3,4   */
  uint16_t seq;
  /* byte 5-8   */
  uint32_t timestamp;
  /* byte 9-12  */
  uint32_t ssrc;

  /* RAW payload header */
  /* lines array might hold more that one line */
  uint16_t extseq;
  rawline_t lines[1];
};
typedef struct rtphead_s rtphead_t;


/*
 * --------------------------------------------------------------------------
 *  Function de processing d'un paquet UDP
 *  1er param = données du paquet
 *  2nd param = taille du paquet
 * --------------------------------------------------------------------------
 */
typedef void (*pktproc_t)( unsigned char *, int );


#endif

