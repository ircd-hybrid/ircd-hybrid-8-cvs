/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  s_protocol.h: A protocol handler interface header.
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
 *  $Id: s_protocol.h,v 1.2 2002/04/26 04:00:24 a1kmm Exp $
 */

#ifndef INCLUDED_S_PROTOCOL_H
#define INCLUDED_S_PROTOCOL_H
#include "msg.h"
#include "config.h"

struct Channel;

struct Protocol
{
  /* This protocol is inherits what protocol? */
  struct Protocol *parent;
  /* Some handlers for sending default responses... */
  MessageHandler m_unregistered;
  MessageHandler m_registered;
  MessageHandler m_not_oper;
  MessageHandler m_error;
  void (*illegal_command)(struct Client *client_p, const char *cmd);
  void (*toofew_params)(struct Client *client_p, const char *cmd);
  void (*burst_channel)(struct Client *client_p, struct Channel *chptr,
                        int bursttype, int burstflags);
  void (*burst_client)(struct Client *client_p, struct Client *target_p,
                       int bursttype, int burstflags);
};

extern struct Protocol p_unregistered, p_user, p_operuser;
#ifdef ENABLE_TS5
extern struct Protocol p_ts5;
#endif

#endif /* INCLUDED_S_PROTOCOL_H */
