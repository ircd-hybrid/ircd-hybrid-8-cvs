/*
 *  ircd_loader.c: Loads the ircd bootup modules.
 *
 *  This is part of ircd-hybrid, an advanced Internet Relay Chat daemon.
 *  Copyright (C) 2002 by the past and present ircd coders.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 *  USA.
 *
 * $Id: ircd_loader.c,v 1.2 2002/01/04 11:06:15 a1kmm Exp $
 */
#include "setup.h"
#include <stdio.h>
#include <unistd.h>
#include "tools.h"
#include "channel.h"
#include "memory.h"
#include "s_conf.h"
#include "s_stats.h"

#ifndef STATIC_MODULES
#include <dlfcn.h>
#endif

int ircd_ready = 0;

/* We have to stick all the globals here which should be preserved across
 * reloads...
 */
struct config_channel_entry ConfigChannel;
struct Channel *GlobalChannelList = NULL;
BlockHeap *channel_heap;
BlockHeap *ban_heap;
struct Client *uplink = NULL;
int MaxConnectionCount = 1;
int MaxClientCount = 1;
int refresh_user_links = 0;

/*
 * stats stuff
 */
static struct ServerStatistics ircst;
struct ServerStatistics *ServerStats = &ircst;

int
main(int argc, char **argv)
{
  while (-1)
  {
#ifndef STATIC_MODULES
    void *mod;
    int (*ircd_main) (int, char **);
    mod = dlopen(IRCD_PREFIX "/modules/main/ircd.so", RTLD_NOW | RTLD_GLOBAL);
    if (mod == NULL)
    {
      fprintf(stderr,
              "Could not load " IRCD_PREFIX
              "/modules/main/ircd.so: %s\n", dlerror());
      exit(-1);
    }
    ircd_main = dlsym(mod, "ircd_main");
    if (ircd_main == NULL)
    {
      fprintf(stderr, "The symbol ircd_main() is not available.\n");
      exit(-1);
    }
#endif
    ircd_main(argc, argv);
#ifndef STATIC_MODULES
    dlclose(mod);
#endif
  }
}
