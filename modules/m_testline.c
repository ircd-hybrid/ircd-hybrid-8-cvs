/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  m_testline.c: Tests a hostmask to see what will happen to it.
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
 *   $Id: m_testline.c,v 1.5 2002/04/19 10:56:17 a1kmm Exp $
 */

#include "handlers.h"
#include "client.h"
#include "common.h"
#include "irc_string.h"
#include "ircd_defs.h"
#include "ircd.h"
#include "restart.h"
#include "s_conf.h"
#include "send.h"
#include "msg.h"
#include "hostmask.h"
#include "numeric.h"
#include "parse.h"
#include "modules.h"

#include <string.h>

static void mo_testline(struct Client *, struct Client *, int, char **);

struct Message testline_msgtab[] = {
  {"TESTLINE", 0, 0, 0, 0, MFLG_SLOW, 0, &p_unregistered, &m_unregistered},
  {"TESTLINE", 0, 0, 0, 0, MFLG_SLOW, 0, &p_user, &m_not_oper},
  {"TESTLINE", 0, 0, 0, 0, MFLG_SLOW, 0, &p_operuser, &mo_testline},
  {NULL, 0, 0, 0, 0, 0, 0, NULL, NULL}
};

#ifndef STATIC_MODULES
void
_modinit(void)
{
  mod_add_cmd(testline_msgtab);
}

void
_moddeinit(void)
{
  mod_del_cmd(testline_msgtab);
}

char *_version = "$Revision: 1.5 $";
#endif
/*
 * mo_testline
 *
 * inputs       - pointer to physical connection request is coming from
 *              - pointer to source connection request is comming from
 *              - parc arg count
 *              - parv actual arguments   
 *   
 * output       - always 0
 * side effects - command to test I/K lines on server
 *   
 * i.e. /quote testline user@host,ip
 *
 */

static void
mo_testline(struct Client *client_p, struct Client *source_p,
            int parc, char *parv[])
{
  struct ConfItem *aconf;
  struct irc_inaddr ip;
  int host_mask;
  char *host, *pass, *user, *name, *classname, *given_host, *given_name, *p;
  int port, t;

  if (parc > 1)
  {
    given_name = parv[1];
    if (!(p = (char *)strchr(given_name, '@')))
    {
      if ((t = parse_netmask(given_name, &ip, &host_mask)) != HM_HOST)
      {
        aconf = find_dline(&ip,
#ifdef IPV6
                           (t == HM_IPV6) ? AF_INET6 : AF_INET
#else
                           AF_INET
#endif
          );
        if (aconf)
        {
          get_printable_conf(aconf, &name, &host, &pass, &user, &port,
                             &classname);
          if (aconf->status & CONF_EXEMPTDLINE)
          {
            sendto_one(source_p,
                       ":%s NOTICE %s :Exempt D-line host [%s] pass [%s]",
                       me.name, parv[0], host, pass);
          }
          else
          {
            sendto_one(source_p,
                       ":%s NOTICE %s :D-line host [%s] pass [%s]", me.name,
                       parv[0], host, pass);
          }
        }
        else
          sendto_one(source_p, ":%s NOTICE %s :No D-line found",
                     me.name, parv[0]);
      }
      else
      {
        sendto_one(source_p, ":%s NOTICE %s :usage: user@host|ip",
                   me.name, parv[0]);
      }
      return;
    }
    *p = '\0';
    p++;
    given_host = p;
    if ((t = parse_netmask(given_host, &ip, &host_mask)) != HM_HOST)
      aconf = find_address_conf(given_host, given_name, &ip,
#ifdef IPV6
                                (t == HM_IPV6) ? AF_INET6 : AF_INET
#else
                                AF_INET
#endif
        );
    else
      aconf = find_address_conf(given_host, given_name, NULL, 0);

    if (aconf)
    {
      get_printable_conf(aconf, &name, &host, &pass, &user, &port,
                         &classname);

      if (aconf->status & CONF_KILL)
      {
        sendto_one(source_p,
                   ":%s NOTICE %s :%c-line name [%s] host [%s] pass [%s]",
                   me.name, parv[0],
                   (aconf->flags & BAN_FLAGS_TEMPORARY) ? 'k' : 'K',
                   user, host, pass);
      }
      else if (aconf->status & CONF_CLIENT)
      {
        sendto_one(source_p,
                   ":%s NOTICE %s :I-line mask [%s] prefix [%s] name [%s] host [%s] port [%d] class [%s]",
                   me.name, parv[0],
                   name,
                   show_iline_prefix(source_p, aconf, user),
                   user, host, port, classname);

      }
    }
    else
      sendto_one(source_p, ":%s NOTICE %s :No aconf found", me.name, parv[0]);
  }
  else
    sendto_one(source_p, ":%s NOTICE %s :usage: user@host|ip",
               me.name, parv[0]);
}
