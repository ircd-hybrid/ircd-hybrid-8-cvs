/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  p_irc.c: The IRC protocol definition.
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
 *   $Id: p_irc.c,v 1.1 2002/01/13 07:15:39 a1kmm Exp $
 */

#include "client.h"
#include "numeric.h"
#include "channel.h"
#include "s_protocol.h"
#include "send.h"

static void irc_unregistered(struct Client*, struct Client*, int, char**);
static void irc_registered(struct Client*, struct Client*, int, char**);
static void irc_error(struct Client*, struct Client*, int, char**);
static void irc_not_oper(struct Client*, struct Client*, int, char**);
static void irc_illegal_command(struct Client*, const char*);
static void irc_toofew_params(struct Client*, const char*);
static void usr_burst_channel(struct Client*, struct Channel*, int, int);
static void usr_burst_channel(struct Client*, struct Channel*, int, int);

static void
irc_unregistered(struct Client *client_p, struct Client *source_p,
                int parc, char *parv[])
{
  /* bit of a hack.
   * I don't =really= want to waste a bit in a flag
   * number_of_nick_changes is only really valid after the client
   * is fully registered..
   */
  if (client_p->localClient->number_of_nick_changes == 0)
  {
    sendto_one(client_p, ":%s %d * %s :Register first.",
               me.name, ERR_NOTREGISTERED, parv[0]);
    client_p->localClient->number_of_nick_changes++;
  }
}

static void
irc_registered(struct Client *client_p, struct Client *source_p,
              int parc, char *parv[])
{
  sendto_one(client_p, form_str(ERR_ALREADYREGISTRED), me.name, parv[0]);
}

static void
irc_error(struct Client *client_p, struct Client *source_p,
        int parc, char *parv[])
{
  char *para;

  para = (parc > 1 && *parv[1] != '\0') ? parv[1] : "<>";

  deprintf("error", "Received ERROR message from %s: %s",
           source_p->name, para);

  if (client_p == source_p)
  {
    sendto_realops_flags(FLAGS_ALL, L_ADMIN,
                         "ERROR :from %s -- %s",
                         get_client_name(client_p, HIDE_IP), para);
    sendto_realops_flags(FLAGS_ALL, L_OPER,
                         "ERROR :from %s -- %s",
                         get_client_name(client_p, MASK_IP), para);
  }
  else
  {
    sendto_realops_flags(FLAGS_ALL, L_OPER,
                         "ERROR :from %s via %s -- %s",
                         source_p->name, get_client_name(client_p, MASK_IP),
                         para);
    sendto_realops_flags(FLAGS_ALL, L_ADMIN, "ERROR :from %s via %s -- %s",
                         source_p->name, get_client_name(client_p, HIDE_IP),
                         para);
  }
  exit_client(client_p, source_p, source_p, "ERROR");
}


static void
irc_not_oper(struct Client *client_p, struct Client *source_p,
           int parc, char *parv[])
{
  sendto_one(source_p, form_str(ERR_NOPRIVILEGES), me.name, parv[0]);
}

static void
irc_illegal_command(struct Client *client_p, const char *cmd)
{
  sendto_one(client_p, form_str(ERR_UNKNOWNCOMMAND), me.name,
             client_p->name[0] ? "*" : client_p->name, cmd);
}

static void
irc_toofew_params(struct Client *client_p, const char *cmd)
{
  sendto_one(client_p, form_str(ERR_NEEDMOREPARAMS), me.name,
             client_p->name[0] ? "*" : client_p->name, cmd);
}

static void
usr_burst_channel(struct Client *client_p, struct Channel *chptr,
                  int bursttype, int burstflags)
{
}

/* Some special protocols are declared here... */
struct Protocol p_unregistered =
{
  NULL,                /* Parent. */
  &irc_unregistered,   /* Outgoing: Unregistered. */
  NULL,                /* Outgoing: Registered. */
  &irc_not_oper,       /* Outgoing: Not an IRC operator. */
  &irc_error,          /* Incoming/Outgoing: error. */
  NULL,                /* Outgoing: No such command. */
  &irc_toofew_params,  /* Outgoing: Too few parameters. */
  NULL,                /* Outgoing: Burst channel. */
};

struct Protocol p_user =
{
  NULL,                /* Parent. */
  NULL,                /* Outgoing: Unregistered. */
  &irc_registered,     /* Outgoing: Registered. */
  &irc_not_oper,       /* Outgoing: Not an IRC operator. */
  NULL,                /* Incoming/Outgoing: error. */
  &irc_illegal_command,/* Outgoing: No such command. */
  &irc_toofew_params,  /* Outgoing: Too few parameters. */
  &usr_burst_channel,  /* Outgoing: Burst channel. */
};

struct Protocol p_operuser =
{
  &p_user,             /* Parent. */
  NULL,                /* Outgoing: Unregistered. */
  &irc_registered,     /* Outgoing: Registered. */
  NULL,                /* Outgoing: Not an IRC operator. */
  NULL,                /* Incoming/Outgoing: error. */
  &irc_illegal_command,/* Outgoing: No such command. */
  &irc_toofew_params,  /* Outgoing: Too few parameters. */
  &usr_burst_channel,  /* Outgoing: Burst channel. */
};
