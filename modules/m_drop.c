/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  m_drop.c: Lets a lazylink indicate it no longer knows a channel.
 *
 *  Copyright (C) 2002 by the past and present ircd coders, and others.
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
 * $Id: m_drop.c,v 1.4 2002/01/13 07:15:17 a1kmm Exp $
 */

#include "tools.h"
#include "channel.h"
#include "client.h"
#include "common.h"
#include "hash.h"
#include "irc_string.h"
#include "ircd.h"
#include "list.h"
#include "numeric.h"
#include "s_serv.h"             /* captab */
#include "s_user.h"
#include "send.h"
#include "handlers.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"

#include <string.h>
#include <stdlib.h>

static void ms_drop(struct Client *, struct Client *, int, char **);

struct Message drop_msgtab[] = {
#ifdef ENABLE_TS5
  {"DROP", 0, 0, 2, 0, MFLG_SLOW | MFLG_UNREG, 0, &p_ts5, &ms_drop},
#endif
  {NULL, 0, 0, 1, 0, 0, 0, NULL, NULL}
};
#ifndef STATIC_MODULES
void
_modinit(void)
{
  mod_add_cmd(drop_msgtab);
}

void
_moddeinit(void)
{
  mod_del_cmd(drop_msgtab);
}

char *_version = "$Revision: 1.4 $";
#endif
/*
** ms_drop
**      parv[0] = sender prefix
**      parv[1] = channel
**      parv[2] = channel password (key)
**
**      "drop" a channel from consideration on a lazy link
*/
static void
ms_drop(struct Client *client_p,
        struct Client *source_p, int parc, char *parv[])
{
  char *name;
  struct Channel *chptr;

  if (parc < 2 || *parv[1] == '\0')
    return;

  name = parv[1];

#ifdef DEBUGLL
  sendto_realops(FLAGS_ALL, "DROP called by %s for %s", client_p->name, name);
#endif

  if ((chptr = hash_find_channel(name)) == NULL)
    return;

  if (client_p->localClient->serverMask)        /* JIC */
    chptr->lazyLinkChannelExists &= ~client_p->localClient->serverMask;
  return;
}
