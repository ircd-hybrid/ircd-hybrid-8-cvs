/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  m_users.c: Gives some basic user statistics.
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 *  USA
 *
 *   $Id: m_users.c,v 1.4 2002/01/13 07:15:20 a1kmm Exp $
 */

#include "handlers.h"
#include "client.h"
#include "ircd.h"
#include "numeric.h"
#include "s_serv.h"
#include "s_conf.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"

static void m_users(struct Client *, struct Client *, int, char **);
static void mo_users(struct Client *, struct Client *, int, char **);

struct Message users_msgtab[] = {
  {"USERS", 0, 0, 0, 0, MFLG_SLOW, 0, &p_unregistered, &m_unregistered},
  {"USERS", 0, 0, 0, 0, MFLG_SLOW, 0, &p_user, &m_users},
  {"USERS", 0, 0, 0, 0, MFLG_SLOW, 0, &p_operuser, &mo_users},
#ifdef ENABLE_TS5
  {"USERS", 0, 0, 0, 0, MFLG_SLOW, 0, &p_ts5, &mo_users},
#endif
  {NULL, 0, 0, 0, 0, 0, 0, NULL, NULL}
};

#ifndef STATIC_MODULES
void
_modinit(void)
{
  mod_add_cmd(users_msgtab);
}

void
_moddeinit(void)
{
  mod_del_cmd(users_msgtab);
}

char *_version = "$Revision: 1.4 $";
#endif

/*
 * m_users
 *      parv[0] = sender prefix
 *      parv[1] = servername
 */
static void
m_users(struct Client *client_p, struct Client *source_p,
        int parc, char *parv[])
{
  if (!ConfigServerHide.disable_remote)
  {
    if (hunt_server(client_p, source_p, ":%s USERS :%s", 1, parc, parv) !=
        HUNTED_ISME)
      return;
  }

  sendto_one(source_p, form_str(RPL_LOCALUSERS), me.name, parv[0],
             ConfigServerHide.hide_servers ? Count.total : Count.local,
             ConfigServerHide.hide_servers ? Count.max_tot : Count.max_loc);

  sendto_one(source_p, form_str(RPL_GLOBALUSERS), me.name, parv[0],
             Count.total, Count.max_tot);
}

/*
 * mo_users
 *      parv[0] = sender prefix
 *      parv[1] = servername
 */
static void
mo_users(struct Client *client_p, struct Client *source_p,
         int parc, char *parv[])
{
  if (hunt_server(client_p, source_p, ":%s USERS :%s", 1, parc, parv) ==
      HUNTED_ISME)
  {
    if (!IsOper(source_p) && ConfigServerHide.hide_servers)
      sendto_one(source_p, form_str(RPL_LOCALUSERS), me.name, parv[0],
                 Count.total, Count.max_tot);
    else
      sendto_one(source_p, form_str(RPL_LOCALUSERS), me.name, parv[0],
                 Count.local, Count.max_loc);

    sendto_one(source_p, form_str(RPL_GLOBALUSERS), me.name, parv[0],
               Count.total, Count.max_tot);
  }
}
