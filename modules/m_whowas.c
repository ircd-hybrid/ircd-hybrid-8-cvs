/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  m_whowas.c: Shows who a user was.
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
 *   $Id: m_whowas.c,v 1.4 2002/01/13 07:15:20 a1kmm Exp $
 */

#include "whowas.h"
#include "handlers.h"
#include "client.h"
#include "common.h"
#include "hash.h"
#include "irc_string.h"
#include "ircd.h"
#include "ircd_defs.h"
#include "numeric.h"
#include "s_serv.h"
#include "s_user.h"
#include "send.h"
#include "s_conf.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

static void m_whowas(struct Client *, struct Client *, int, char **);
static void mo_whowas(struct Client *, struct Client *, int, char **);

struct Message whowas_msgtab[] = {
  {"WHOWAS", 0, 0, 0, 0, MFLG_SLOW, 0, &p_unregistered, &m_unregistered},
  {"WHOWAS", 0, 0, 0, 0, MFLG_SLOW, 0, &p_user, &m_whowas},
  {"WHOWAS", 0, 0, 0, 0, MFLG_SLOW, 0, &p_operuser, &mo_whowas},
  {NULL, 0, 0, 0, 0, 0, 0, NULL, NULL}
};

#ifndef STATIC_MODULES
void
_modinit(void)
{
  mod_add_cmd(whowas_msgtab);
}

void
_moddeinit(void)
{
  mod_del_cmd(whowas_msgtab);
}

char *_version = "$Revision: 1.4 $";
#endif
static int whowas_do(struct Client *client_p, struct Client *source_p,
                     int parc, char *parv[]);


/*
** m_whowas
**      parv[0] = sender prefix
**      parv[1] = nickname queried
*/
static void
m_whowas(struct Client *client_p,
         struct Client *source_p, int parc, char *parv[])
{
  static time_t last_used = 0L;

  if (parc < 2)
  {
    sendto_one(source_p, form_str(ERR_NONICKNAMEGIVEN), me.name, parv[0]);
    return;
  }

  if ((last_used + ConfigFileEntry.whois_wait) > CurrentTime)
  {
    sendto_one(source_p, form_str(RPL_LOAD2HI), me.name, source_p->name);
    return;
  }
  else
  {
    last_used = CurrentTime;
  }

  whowas_do(client_p, source_p, parc, parv);
}

static void
mo_whowas(struct Client *client_p,
          struct Client *source_p, int parc, char *parv[])
{
  if (parc < 2)
  {
    sendto_one(source_p, form_str(ERR_NONICKNAMEGIVEN), me.name, parv[0]);
    return;
  }

  whowas_do(client_p, source_p, parc, parv);
}

static int
whowas_do(struct Client *client_p, struct Client *source_p,
          int parc, char *parv[])
{
  struct Whowas *temp;
  int cur = 0;
  int max = -1, found = 0;
  char *p, *nick;

  if (parc < 2)
  {
    sendto_one(source_p, form_str(ERR_NONICKNAMEGIVEN), me.name, parv[0]);
    return 0;
  }
  if (parc > 2)
    max = atoi(parv[2]);
  if (parc > 3)
    if (hunt_server
        (client_p, source_p, ":%s WHOWAS %s %s :%s", 3, parc, parv))
      return 0;


  if ((p = strchr(parv[1], ',')))
    *p = '\0';

  nick = parv[1];

  temp = WHOWASHASH[hash_whowas_name(nick)];
  found = 0;
  for (; temp; temp = temp->next)
  {
    if (!irccmp(nick, temp->name))
    {
      sendto_one(source_p, form_str(RPL_WHOWASUSER),
                 me.name, parv[0], temp->name,
                 temp->username, temp->hostname, temp->realname);

      if (ConfigServerHide.hide_servers && !IsOper(source_p))
        sendto_one(source_p, form_str(RPL_WHOISSERVER),
                   me.name, parv[0], temp->name,
                   ServerInfo.network_name, myctime(temp->logoff));
      else
        sendto_one(source_p, form_str(RPL_WHOISSERVER),
                   me.name, parv[0], temp->name,
                   temp->servername, myctime(temp->logoff));
      cur++;
      found++;
    }
    if (max > 0 && cur >= max)
      break;
  }
  if (!found)
    sendto_one(source_p, form_str(ERR_WASNOSUCHNICK), me.name, parv[0], nick);

  sendto_one(source_p, form_str(RPL_ENDOFWHOWAS), me.name, parv[0], parv[1]);
  return 0;
}
