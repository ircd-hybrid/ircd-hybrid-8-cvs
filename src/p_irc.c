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
 *   $Id: p_irc.c,v 1.4 2002/04/26 04:00:30 a1kmm Exp $
 */

#include "client.h"
#include "numeric.h"
#include "channel.h"
#include "channel_mode.h"
#include "s_protocol.h"
#include "send.h"
#include "debug.h"

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
  const char *name = (chptr->root_chptr ? chptr->root_chptr : chptr)->chname;
  if (chptr->topic[0] != '\0')
  {
    sendto_one(client_p, form_str(RPL_TOPIC), me.name,
               client_p->name, name, chptr->topic);

    if (!(chptr->mode.mode & MODE_HIDEOPS) ||
        (bursttype & CHFL_CHANOP) || (bursttype & CHFL_HALFOP))
    {
      sendto_one(client_p, form_str(RPL_TOPICWHOTIME),
                 me.name, client_p->name, name,
                 chptr->topic_info, chptr->topic_time);
    }
    else /* Hide from nonops */
    {
      sendto_one(client_p, form_str(RPL_TOPICWHOTIME),
                 me.name, client_p->name, name,
                 me.name, chptr->topic_time);
    }
  }
  
  channel_member_names(client_p, chptr, (char*)name, 1);
}

/* Some special protocols are declared here... */
struct Protocol p_unregistered =
{
  m_unregistered: &irc_unregistered,
  m_not_oper: &irc_not_oper,
  m_error: &irc_error,
  toofew_params: &irc_toofew_params
};

struct Protocol p_user =
{
  m_registered: &irc_registered,
  m_not_oper: &irc_not_oper,
  illegal_command: &irc_illegal_command,
  toofew_params: &irc_toofew_params,
  burst_channel: &usr_burst_channel
};

struct Protocol p_operuser =
{
  parent: &p_user,
  m_registered: &irc_registered,
  illegal_command: &irc_illegal_command,
  toofew_params: &irc_toofew_params,
  burst_channel: &usr_burst_channel
};
