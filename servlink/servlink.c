/************************************************************************
 *   IRC - Internet Relay Chat, servlink/servlink.c
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   $Id: servlink.c,v 1.1 2002/01/04 09:13:38 a1kmm Exp $
 */

#include "setup.h"                                                   

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_LIBCRYPTO
#include <openssl/evp.h>
#include <openssl/err.h>
#endif
#ifdef HAVE_LIBZ
#include <zlib.h>
#endif

#include "servlink.h"
#include "io.h"
#include "control.h"

static void usage(void);

struct slink_state       in_state;
struct slink_state       out_state;

struct fd_table          fds[5] =
        {
          { 0,  read_ctrl, NULL },
          { 0,       NULL, NULL },
          { 0,       NULL, NULL },
          { 0,       NULL, NULL },
          { 0,       NULL, NULL }
        };

/* usage();
 *
 * Display usage message
 */
static void usage(void)
{
  fprintf(stderr, "ircd-hybrid server link v1.0\n");
  fprintf(stderr, "2001-06-04\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "This program is called by the ircd-hybrid ircd.\n");
  fprintf(stderr, "It cannot be used on its own.\n");
  exit(1);
}

int main(int argc, char *argv[])
{
  int max_fd = 0;
  int i;
#ifdef SERVLINK_DEBUG
  int GDBAttached = 0;

  while (!GDBAttached)
    sleep(1);
#endif

#ifdef HAVE_LIBCRYPTO
  /* load error strings */
  ERR_load_crypto_strings();
#endif

  /* Make sure we are running under hybrid.. */
  if (argc != 6 || strcmp(argv[0], "-slink"))
    usage(); /* exits */

  for (i = 0; i < 5; i++ )
  {
    fds[i].fd = atoi(argv[i+1]);
    if (fds[i].fd < 0)
      exit(1);
    if (fds[i].fd > max_fd)
      max_fd = fds[i].fd;
  }
  
  /* set file descriptors to nonblocking mode */
  for (i = 0; i < 5; i++)
  {
    fcntl(fds[i].fd, F_SETFL, O_NONBLOCK);
  }

  /* enter io loop */
  io_loop(max_fd + 1);

  /* NOTREACHED */
  exit(0);
} /* main() */