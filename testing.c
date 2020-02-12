#include <endian.h>
#include "vpl.h"


//--------------------------------------------------------------------------
//  Si la compilation des lignes suivantes échoue
//  alors il y a un problème sur les tailles des types...
//  Il est inutile de poursuivre le code ne se comportera pas correctement
//  car il prend pour hypothèse que les tests suivants sont OK
//--------------------------------------------------------------------------
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


static unsigned char sample1[] =
  {
   // RTP header standard
   0x80, 0x60, 0x3c, 0xc2, 0x61, 0x51, 0xf5, 0xb7,
   0x99, 0x99, 0x99, 0x99,

   // extension du header RTP pour RAW video
   0x00, 0x00, 0x04, 0x38, 0x00, 0x80, 0x01, 0x68
  };

static unsigned char sample2[] =
  {
   // RTP header standard
   0x80, 0x60, 0x3c, 0xc3, 0x61, 0x51, 0xf5, 0xb7,
   0x99, 0x99, 0x99, 0x99,

   // extension du header RTP pour RAW video
   0x00, 0x00, 0x04, 0x38, 0x00, 0x82, 0x00, 0x00
  };

static unsigned char sample3[] =
  {
   // RTP header standard
   0x80, 0xe0, 0x3e, 0x80, 0x61, 0x51, 0xf5, 0xb7,
   0x99, 0x99, 0x99, 0x99,

   // extension du header RTP pour RAW video
   0x00, 0x00, 0x04, 0x38, 0x02, 0x3e, 0x01, 0x68
  };

static unsigned char sample4[] =
  {
   // RTP header standard
   0x80, 0x60, 0x3e, 0x81, 0x61, 0x52, 0x06, 0x10,
   0x99, 0x99, 0x99, 0x99,

   // extension du header RTP pour RAW video
   0x00, 0x00, 0x04, 0x38, 0x00, 0x00, 0x00, 0x00
  };

static unsigned char sample5[] =
  {
   // RTP header standard
   0x80, 0x60, 0x3e, 0x82, 0x61, 0x52, 0x06, 0x10,
   0x99, 0x99, 0x99, 0x99,

   // extension du header RTP pour RAW video
   0x00, 0x00, 0x04, 0x38, 0x00, 0x00, 0x01, 0x68
  };


// fonction de test
int test()
{
  extern int dump( rtphead_t * );
  dump( (rtphead_t*) &(sample1[0]) );
  dump( (rtphead_t*) &(sample2[0]) );
  dump( (rtphead_t*) &(sample3[0]) );
  dump( (rtphead_t*) &(sample4[0]) );
  dump( (rtphead_t*) &(sample5[0]) );
}
