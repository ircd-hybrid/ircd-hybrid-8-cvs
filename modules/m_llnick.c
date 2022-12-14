/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  m_llnick.c: Used to introduce a client on a lazy-link.
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
 * $Id: m_llnick.c,v 1.4 2002/01/13 07:15:18 a1kmm Exp $
 */

#include "tools.h"
#include "client.h"
#include "hash.h"
#include "common.h"
#include "hash.h"
#include "irc_string.h"
#include "ircd.h"
#include "list.h"
#include "numeric.h"
#include "s_serv.h"
#include "send.h"
#include "handlers.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

static void ms_llnick(struct Client *, struct Client *, int, char **);

struct Message llnick_msgtab[] = {
#ifdef ENABLE_TS5
  {"LLNICK", 0, 0, 3, 0, MFLG_SLOW | MFLG_UNREG, 0, &p_ts5, &ms_llnick},
#endif
  {NULL, 0, 0, 0, 0, 0, 0, NULL, NULL}
};
#ifndef STATIC_MODULES

void
_modinit(void)
{
  mod_add_cmd(llnick_msgtab);
}

void
_moddeinit(void)
{
  mod_del_cmd(llnick_msgtab);
}

char *_version = "$Revision: 1.4 $";
#endif
/*
 * m_llnick
 *      parv[0] = sender prefix
 *      parv[1] = status 
 *      parv[2] = nick
 *      parv[3] = old nick
 *
 */
static void
ms_llnick(struct Client *client_p,
          struct Client *source_p, int parc, char *parv[])
{
  char *nick;
  char *nick_old = NULL;
  struct Client *target_p = NULL;
  int exists = 0;
  int new = 0;
  dlink_node *ptr;

  if (!IsCapable(client_p, CAP_LL) || !IsCapable(client_p, CAP_HUB))
  {
    sendto_realops_flags(FLAGS_ALL, L_ALL,
                         "*** LLNICK requested from non LL server %s",
                         client_p->name);
    return;
  }

  if (parc < 4)
    return;

  if (*parv[1] == 'Y')
    exists = 1;

  nick = parv[2];
  nick_old = parv[3];

  if (*nick_old == '!')
    new = 1;

  if (new)
  {
    nick_old++;                 /* skip '!' */
    /* New user -- find them */
    for (ptr = unknown_list.head; ptr; ptr = ptr->next)
    {
      if (!strcmp(nick_old, ((struct Client *)ptr->data)->llname))
      {
        target_p = ptr->data;
        *target_p->llname = '\0';       /* unset their temp-nick */
        break;
      }
    }
    if (!target_p)              /* Can't find them -- maybe they got a different nick */
      return;
  }
  else
  {
    /* Existing user changing nickname */
    target_p = find_client(nick_old);

    if (!target_p)              /* Can't find them -- maybe they got a different nick */
      return;
  }

  /* Don't try this on a remote client... */
  if (!MyConnect(target_p))
    return;

  if (find_client(nick) || exists)
  {
    /* The nick they want is in use. complain */
    sendto_one(target_p, form_str(ERR_NICKNAMEINUSE), me.name,
               new ? "*" : nick_old, nick);
    return;
  }

  if (new)
    set_initial_nick(target_p, target_p, nick);
  else
    change_local_nick(target_p, target_p, nick);
}
