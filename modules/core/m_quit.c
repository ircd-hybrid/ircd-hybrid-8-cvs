/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  m_quit.c: Makes a user quit from IRC.
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
 *   $Id: m_quit.c,v 1.5 2002/02/12 09:46:50 a1kmm Exp $
 */

#include "handlers.h"
#include "client.h"
#include "ircd.h"
#include "numeric.h"
#include "s_serv.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "s_conf.h"

static void m_quit(struct Client *, struct Client *, int, char **);
static void ms_quit(struct Client *, struct Client *, int, char **);

struct Message quit_msgtab[] = {
  {"QUIT", 0, 0, 0, 0, MFLG_SLOW | MFLG_UNREG, 0, &p_unregistered, &m_quit},
  {"QUIT", 0, 0, 0, 0, MFLG_SLOW, 0, &p_user, &m_quit},
#ifdef ENABLE_TS5
  {"QUIT", 0, 0, 0, 0, MFLG_SLOW, 0, &p_ts5, ms_quit},
#endif
  {NULL, 0, 0, 1, 0, 0, 0, NULL, NULL}
};

#ifndef STATIC_MODULES
void
_modinit(void)
{
  mod_add_cmd(quit_msgtab);
}

void
_moddeinit(void)
{
  mod_del_cmd(quit_msgtab);
}

char *_version = "$Revision: 1.5 $";
#endif
/*
** m_quit
**      parv[0] = sender prefix
**      parv[1] = comment
*/
static void
m_quit(struct Client *client_p,
       struct Client *source_p, int parc, char *parv[])
{
  char *comment = (parc > 1 && parv[1]) ? parv[1] : client_p->name;
  char reason[TOPICLEN + 1];

  source_p->flags |= FLAGS_NORMALEX;
  if (strlen(comment) > (size_t) TOPICLEN)
    comment[TOPICLEN] = '\0';

  if (ConfigFileEntry.client_exit && comment[0])
  {
#ifndef VMS
    snprintf(reason, TOPICLEN, "Client Exit: %s", comment);
#else
    sprintf(reason, "Client Exit: %s", comment);
#endif
    comment = reason;
  }

  if (!IsOper(source_p) &&
      (source_p->firsttime + ConfigFileEntry.anti_spam_exit_message_time)
      > CurrentTime)
  {
    comment = "Client Quit";
  }

  exit_client(client_p, source_p, source_p, comment);
}

/*
** ms_quit
**      parv[0] = sender prefix
**      parv[1] = comment
*/
static void
ms_quit(struct Client *client_p,
        struct Client *source_p, int parc, char *parv[])
{
  char *comment = (parc > 1 && parv[1]) ? parv[1] : client_p->name;

  source_p->flags |= FLAGS_NORMALEX;
  if (strlen(comment) > (size_t) TOPICLEN)
    comment[TOPICLEN] = '\0';

  exit_client(client_p, source_p, source_p, comment);
}
