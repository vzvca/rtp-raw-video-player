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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "vpl.h"

#define M_PORT 5004
#define M_GROUP "239.192.77.10"
#define M_NIC  "eth0.10"
#define FBDEV "/dev/fb0"

char *g_mgroup = M_GROUP;
char *g_nic = M_NIC;
int   g_mport = M_PORT;
char *g_fbdev = FBDEV;
int   g_width = -1;
int   g_height = -1;
char *g_out = NULL;
int   g_x = 0;
int   g_y = 0;

/* packets processors */
extern void pktprocess( unsigned char *pkt, int pktlen );
extern void imgprocess( unsigned char *pkt, int pktlen );

extern int initout( char *file, image_t *img );
extern int initfb( char *fbdev, image_t *img );
extern int test();

/* this is where th video will be stored */
extern image_t *pim;


/*
 * --------------------------------------------------------------------------
 *   Get IP address of network interface
 * --------------------------------------------------------------------------
 */
int get_nic_addr(const char * nic,  struct sockaddr_in *addr)
{
  struct ifreq ifr;
  int fd, r;
  
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) {
      return -1;
  }

  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name, nic, IFNAMSIZ);
  r = ioctl(fd, SIOCGIFADDR, &ifr);

  if (r < 0) {
    fprintf( stderr, "get_nic_addr : ioctl could not request device %s\n", nic);
  }

  close(fd);

  *addr = *(struct sockaddr_in *)&ifr.ifr_addr;

  return r;
}

/*
 * --------------------------------------------------------------------------
 *   Tune socket timeout
 * --------------------------------------------------------------------------
 */
static int set_recv_timeout( int socket, unsigned int timeout_us)
{
  struct timeval l_timeout;
  l_timeout.tv_sec = 0;
  l_timeout.tv_usec = timeout_us;
  if( setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &l_timeout, sizeof(l_timeout)) == -1) {
      fprintf( stderr, "setRecvTimeout : RCVTIMEO not set\n");
      return -1;
    }
  return 0;
}

/*
 * --------------------------------------------------------------------------
 *   Connect to multicast group
 * --------------------------------------------------------------------------
 */
static int connect_to_multicast_group( int socket, char *mgroup, char *nic, int mport)
{
  int reuse = 1; /* value to bind several sock on a specific port */
  struct ip_mreq lmreq = {0, 0};
  struct sockaddr_in nic_addr  = {0, 0, 0, 0};
  struct sockaddr_in laddr = {0, 0, 0, 0};
  
  /* struct that allows to specify to which IP to subscribe */
  lmreq.imr_multiaddr.s_addr = inet_addr(mgroup);

  if (!get_nic_addr( nic, &nic_addr )) {
    lmreq.imr_interface.s_addr = nic_addr.sin_addr.s_addr;
    printf( "%s interface -> ip : %s\n", nic, inet_ntoa(lmreq.imr_interface));
  }
  else {
    fprintf( stderr, "could not get interface address %s\n", nic);
    return -1;
  }
  
  /* to be able to bind several IP on port */
  if ( setsockopt( socket, SOL_SOCKET, SO_REUSEADDR, (int *)&reuse, sizeof(reuse) ) == -1) {
    fprintf( stderr, "connect_to_multicast_group : REUSEADDR not set\n");
    return -1;
  }

  printf("connect_to_multicast_group : set REUSEADDR\n");

  /* subscribe to multicast group */
  if ( setsockopt(socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &lmreq, sizeof(struct ip_mreq)) == -1) {
    fprintf( stderr, "connect_to_multicast_group : error subscribing to multicast group %s", mgroup );
    return -1;
  }

  printf("connect_to_multicast_group : SUBSCRIBE To MulticastGroup\n");

  laddr.sin_family = AF_INET;
  laddr.sin_port = htons(mport);
  laddr.sin_addr.s_addr = /*inet_addr(mgroup);*/ htonl(INADDR_ANY);

  /* bind socket to the right port to receive data */
  if ( bind(socket, (struct sockaddr *)&laddr, sizeof(struct sockaddr_in)) == -1) {
    fprintf( stderr, "connect_to_multicast_group : error binding\n");
    return -1;
  }

  printf( "connect_to_multicast_group : Bind socket\n");

  return 0;
}


/*
 * --------------------------------------------------------------------------
 *   Loop over incoming packets sending them to pfun for handling
 * --------------------------------------------------------------------------
 */
void netloop( pktproc_t pfun )
{
  struct sockaddr_in addr;
  int addrlen, sock, cnt;
  struct ip_mreq mreq;
  unsigned char message[1500];

  /* set up socket */
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("socket");
    exit(1);
  }

  connect_to_multicast_group( sock, g_mgroup, g_nic, g_mport);
  
  while (1) {
    cnt = recvfrom(sock, message, sizeof(message), 0, (struct sockaddr *) &addr, &addrlen);
    if (cnt < 0) {
      perror("recvfrom");
      exit(1);
    } else if (cnt == 0) {
      break;
    }
    pfun(message, cnt);
  }
}


/*
 * --------------------------------------------------------------------------
 *   Usage
 * --------------------------------------------------------------------------
 */
void usage( int argc, char *argv[], int optind )
{
  char *what = (optind > 0) ? "error" : "usage";
  fprintf( stderr, "%s %s :\n%s [-h] [-i nic] [-m group] [-p port] [-d fbdev] [-o file] [-w width] [-h height]\n", argv[0], what, argv[0]);
  fprintf( stderr, "\t-?\t\tPrints this message.\n");
  fprintf( stderr, "\t-i\t\tSets network interface to bind to (default %s).\n", g_nic);
  fprintf( stderr, "\t-g\t\tSets multicast group to join (default %s).\n", g_mgroup);
  fprintf( stderr, "\t-p\t\tSets port to use (default %d).\n", g_mport);
  fprintf( stderr, "\t-d\t\tSets the framebuffer device (default %s).\n", g_fbdev);
  fprintf( stderr, "\t-o\t\tSets the image output file.\n");
  fprintf( stderr, "\t-w\t\tSets the width of the image.\n");
  fprintf( stderr, "\t-h\t\tSets the height of the image.\n");
  fprintf( stderr, "\t-x\t\tSets the x offset of the image when rendering to framebuffer.\n");
  fprintf( stderr, "\t-y\t\tSets the y offset of the image when rendering to framebuffer.\n");
  fprintf( stderr, "Options -o -w -h must be used together and are incompatible with -d.");
  /* exit with error only if option parsng failed */
  exit(optind > 0);
}

/* --------------------------------------------------------------------------
 *   Programme principal
 * --------------------------------------------------------------------------*/
int main( int argc, char *argv[] )
{
  int opt;
  
  while ( (opt = getopt( argc, argv, "?i:g:p:d:o:w:h:x:y:")) != -1 ) {
    switch( opt ) {
    case '?':  usage( argc, argv, 0);
    case 'i':  g_nic = optarg; break;
    case 'g':  g_mgroup = optarg; break;
    case 'p':  g_mport = atoi(optarg); break;
    case 'd':  g_fbdev = optarg; break;
    case 'w':  g_width = atoi(optarg); break;
    case 'h':  g_height = atoi(optarg); break;
    case 'o':  g_out = optarg; break;
    case 'x':  g_x = atoi(optarg); break;
    case 'y':  g_y = atoi(optarg); break;
    default:
      usage(argc, argv, optind);
    }
  }

  /* Check arguments validity */
  if ( g_out != NULL && (g_width < 0 || g_height < 0))  {
    fprintf( stderr, "options '-w' and '-h' are mandatory when '-o' is used.\n");
    exit(1);
  }
  if ( g_out == NULL && (g_width >= 0 || g_height >= 0)) {
    fprintf( stderr, "options '-w' and '-h' cannot be used when rendering to framebuffer.\n");
    exit(1);
  }
  if ( g_out != NULL && (g_x != 0 || g_y != 0)) {
    fprintf( stderr, "options '-x' and '-y' cannot be used when '-o' is used.\n");
    exit(1);
  }

  /* Create image for rendering */
  if ( g_out == NULL ) {
    int fbfd = initfb( g_fbdev, pim );
  }
  else {
    int imfd = initout( g_out, pim );
  }
  
  /* rendering */
  netloop(imgprocess);
}

