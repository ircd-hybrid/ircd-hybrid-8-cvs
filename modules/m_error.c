/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  m_error.c: Handles error messages from the other end.
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
 *   $Id: m_error.c,v 1.1 2002/01/13 07:15:17 a1kmm Exp $
 */

#include "handlers.h"
#include "client.h"
#include "common.h"             /* FALSE */
#include "ircd.h"
#include "numeric.h"
#include "send.h"
#include "s_debug.h"
#include "msg.h"
#include "memory.h"
#include "debug.h"
#include "s_protocol.h"
#include "modules.h"

void m_error(struct Client*, struct Client*, int, char **);

void
m_error(struct Client *client_p, struct Client *source_p,
         int parc, char *parv[])
{
  client_p->protocol->m_error(client_p, source_p, parc, parv);
}

struct Message error_msgtab[] =
{
   {"ERROR", 0, 0, 1, 0, MFLG_SLOW|MFLG_UNREG, 0, &p_unregistered,
    &m_error},
   {"ERROR", 0, 0, 1, 0, MFLG_SLOW|MFLG_UNREG, 0, &p_ts5,
    &m_error},
   {NULL, 0, 0, 0, 0, 0, 0, NULL, NULL}
};

void
_modinit(void)
{
  mod_add_cmd(error_msgtab);
}
