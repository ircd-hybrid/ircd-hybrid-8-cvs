/************************************************************************
 *   IRC - Internet Relay Chat, modules/m_user.c
 *   Copyright (C) 1990 Jarkko Oikarinen and
 *                      University of Oulu, Computing Center
 *
 *   See file AUTHORS in IRC package for additional names of
 *   the programmers. 
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
 *   $Id: m_user.c,v 1.1 2002/01/04 09:15:33 a1kmm Exp $
 */
#include "handlers.h"
#include "client.h"
#include "irc_string.h"
#include "ircd.h"
#include "numeric.h"
#include "s_user.h"
#include "send.h"
#include "s_conf.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"

#include <string.h>

#define UFLAGS  (FLAGS_INVISIBLE|FLAGS_WALLOP|FLAGS_SERVNOTICE)

static void mr_user(struct Client*, struct Client*, int, char**);

struct Message user_msgtab = {
  "USER", 0, 0, 5, 0, MFLG_SLOW, 0L,
  {mr_user, m_registered, m_ignore, m_registered}
};

#ifndef STATIC_MODULES
void
_modinit(void)
{
  mod_add_cmd(&user_msgtab);
}

void
_moddeinit(void)
{
  mod_del_cmd(&user_msgtab);
}

char *_version = "$Revision: 1.1 $";
#endif
/*
** mr_user
**      parv[0] = sender prefix
**      parv[1] = username (login name, account)
**      parv[2] = client host name (used only from other servers)
**      parv[3] = server host name (used only from other servers)
**      parv[4] = users real name info
*/
static void mr_user(struct Client* client_p, struct Client* source_p,
		   int parc, char *parv[])
{
  char* p;
 
  if ((p = strchr(parv[1],'@')))
    *p = '\0'; 

  if (*parv[4] == '\0')
    {
      sendto_one(source_p, form_str(ERR_NEEDMOREPARAMS),
                 me.name, BadPtr(parv[0]) ? "*" : parv[0], "USER");
      return;
    }

  do_local_user(parv[0], client_p, source_p,
                parv[1],	/* username */
                parv[2],	/* host */
                parv[3],	/* server */
                parv[4]	/* users real name */ );
}


