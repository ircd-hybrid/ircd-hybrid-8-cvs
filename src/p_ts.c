/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  p_ts.c: The TSora and descendant protocol definition.
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
 *   $Id: p_ts.c,v 1.2 2002/04/19 10:56:20 a1kmm Exp $
 */

#include "s_protocol.h"
#include "send.h"
#include "debug.h"
#include "client.h"
#include <stdlib.h>

#ifdef ENABLE_TS5

void tsm_error(struct Client*, struct Client*, int, char **);

void
tsm_error(struct Client *client_p, struct Client *source_p,
         int parc, char *parv[])
{
  char *para;

  para = (parc > 1 && *parv[1] != '\0') ? parv[1] : "<>";

  deprintf("error", "Received ERROR message from %s: %s",
           source_p->name, para);

  if (client_p == source_p)
    sendto_realops_flags(FLAGS_ALL, L_ALL, "ERROR :from %s -- %s",
                         get_client_name(client_p, MASK_IP), para);
  else
    sendto_realops_flags(FLAGS_ALL, L_ALL, "ERROR :from %s via %s -- %s",
                         source_p->name, get_client_name(client_p, MASK_IP),
                         para);
}

struct Protocol p_ts5 =
{
  m_error: tsm_error,
  /* burst_channel: ts_burst_channel */
};

#endif
