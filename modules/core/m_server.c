/***********************************************************************
 *   IRC - Internet Relay Chat, modules/m_server.c
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
 *   $Id: m_server.c,v 1.1 2002/01/04 10:57:32 a1kmm Exp $
 */
#include "tools.h"
#include "handlers.h"  /* m_server prototype */
#include "client.h"      /* client struct */
#include "common.h"      /* TRUE bleah */
#include "event.h"
#include "hash.h"        /* add_to_client_hash_table */
#include "irc_string.h"  /* strncpy_irc */
#include "ircd.h"        /* me */
#include "list.h"        /* make_server */
#include "numeric.h"     /* ERR_xxx */
#include "s_conf.h"      /* struct ConfItem */
#include "s_log.h"       /* log level defines */
#include "s_serv.h"      /* server_estab, check_server, my_name_for_link */
#include "s_stats.h"     /* ServerStats */
#include "scache.h"      /* find_or_add */
#include "send.h"        /* sendto_one */
#include "motd.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"

#include <string.h>
#include <stdlib.h>

static void mr_server(struct Client*, struct Client*, int, char **);
static void ms_server(struct Client*, struct Client*, int, char **);

static int set_server_gecos(struct Client *, char *);

struct Message server_msgtab = {
  "SERVER", 0, 0, 3, 0, MFLG_SLOW | MFLG_UNREG, 0,
  {mr_server, m_registered, ms_server, m_registered}
};

#ifndef STATIC_MODULES
void 
_modinit(void)
{
  mod_add_cmd(&server_msgtab);
}

void
_moddeinit(void)
{
  mod_del_cmd(&server_msgtab);
}
char *_version = "$Revision: 1.1 $";
#endif

int bogus_host(char *host);
struct Client *server_exists(char *);

/*
 * mr_server - SERVER message handler
 *      parv[0] = sender prefix
 *      parv[1] = servername
 *      parv[2] = serverinfo/hopcount
 *      parv[3] = serverinfo
 */
static void mr_server(struct Client *client_p, struct Client *source_p,
                      int parc, char *parv[])
{
  char             info[REALLEN + 1];
  char             *name;
  struct Client    *target_p;
  int hop;

  if (parc < 4)
    {
      sendto_one(client_p,"ERROR :No servername");
      exit_client(client_p, client_p, client_p, "Wrong number of args");
      return;
    }

  name = parv[1];
  hop = atoi(parv[2]);
  strncpy_irc(info, parv[3], REALLEN);
  info[REALLEN] = '\0';

  /* 
   * Reject a direct nonTS server connection if we're TS_ONLY -orabidoo
   */
  if (!DoesTS(client_p))
    {
      sendto_realops_flags(FLAGS_ALL, L_ADMIN,"Link %s dropped, non-TS server",
			   get_client_name(client_p, HIDE_IP));
      sendto_realops_flags(FLAGS_ALL, L_OPER,"Link %s dropped, non-TS server",
			   get_client_name(client_p, MASK_IP));
      exit_client(client_p, client_p, client_p, "Non-TS server");
      return;
    }

  if (bogus_host(name))
  {
    exit_client(client_p, client_p, client_p, "Bogus server name");
    return;
  }

  /* Now we just have to call check_server and everything should be
   * check for us... -A1kmm. */
  switch (check_server(name, client_p, CHECK_SERVER_NOCRYPTLINK))
  {
    case -1:
      if (ConfigFileEntry.warn_no_nline)
      {
        sendto_realops_flags(FLAGS_ALL, L_ADMIN,
           "Unauthorized server connection attempt from %s: No entry for "
           "servername %s", get_client_name(client_p, HIDE_IP), name);

        sendto_realops_flags(FLAGS_ALL, L_OPER,
           "Unauthorized server connection attempt from %s: No entry for "
           "servername %s", get_client_name(client_p, MASK_IP), name);
      }
      
      exit_client(client_p, client_p, client_p, "Invalid servername.");
      return;
      break;
      
    case -2:
      sendto_realops_flags(FLAGS_ALL, L_ADMIN,
           "Unauthorized server connection attempt from %s: Bad password "
           "for server %s", get_client_name(client_p, HIDE_IP), name);

      sendto_realops_flags(FLAGS_ALL, L_OPER,
           "Unauthorized server connection attempt from %s: Bad password "
           "for server %s", get_client_name(client_p, MASK_IP), name);

      exit_client(client_p, client_p, client_p, "Invalid password.");
      return;
      break;
      
    case -3:
      sendto_realops_flags(FLAGS_ALL, L_ADMIN,
           "Unauthorized server connection attempt from %s: Invalid host "
           "for server %s", get_client_name(client_p, HIDE_IP), name);

      sendto_realops_flags(FLAGS_ALL, L_OPER,
           "Unauthorized server connection attempt from %s: Invalid host "
           "for server %s", get_client_name(client_p, MASK_IP), name);

      exit_client(client_p, client_p, client_p, "Invalid host.");
      return;
      break;
  }
    
  if ((target_p = server_exists(name)))
    {
      /*
       * This link is trying feed me a server that I already have
       * access through another path -- multiple paths not accepted
       * currently, kill this link immediately!!
       *
       * Rather than KILL the link which introduced it, KILL the
       * youngest of the two links. -avalon
       *
       * Definitely don't do that here. This is from an unregistered
       * connect - A1kmm.
       */
      sendto_realops_flags(FLAGS_ALL, L_ADMIN,
         "Attempt to re-introduce server %s from %s", name,
         get_client_name(client_p, HIDE_IP));

      sendto_realops_flags(FLAGS_ALL, L_OPER,
         "Attempt to re-introduce server %s from %s", name,
         get_client_name(client_p, MASK_IP));

      sendto_one(client_p, "ERROR :Server already exists.");
      exit_client(client_p, client_p, client_p, "Server Exists");
      return;
    }

  if(ServerInfo.hub && IsCapable(client_p, CAP_LL))
    {
      if(IsCapable(client_p, CAP_HUB))
        {
          ClearCap(client_p,CAP_LL);
          sendto_realops_flags(FLAGS_ALL, L_ALL,
               "*** LazyLinks to a hub from a hub, thats a no-no.");
        }
      else
        {
          client_p->localClient->serverMask = nextFreeMask();

          if(!client_p->localClient->serverMask)
            {
              sendto_realops_flags(FLAGS_ALL, L_ALL,
                                   "serverMask is full!");
              /* try and negotiate a non LL connect */
              ClearCap(client_p,CAP_LL);
            }
        }
    }
  else if (IsCapable(client_p, CAP_LL))
    {
      if(!IsCapable(client_p, CAP_HUB))
        {
          ClearCap(client_p,CAP_LL);
          sendto_realops_flags(FLAGS_ALL, L_ALL,
               "*** LazyLinks to a leaf from a leaf, thats a no-no.");
        }
    }

  /*
   * if we are connecting (Handshake), we already have the name from the
   * C:line in client_p->name
   */

  strncpy_irc(client_p->name, name, HOSTLEN);
  set_server_gecos(client_p, info);
  client_p->hopcount = hop;
  server_estab(client_p);
}

/*
 * ms_server - SERVER message handler
 *      parv[0] = sender prefix
 *      parv[1] = servername
 *      parv[2] = serverinfo/hopcount
 *      parv[3] = serverinfo
 */
static void ms_server(struct Client *client_p, struct Client *source_p,
                      int parc, char *parv[])
{
  char             info[REALLEN + 1];
                   /* same size as in s_misc.c */
  char*            name;
  struct Client*   target_p;
  struct Client*   bclient_p;
  struct ConfItem* aconf;
  int              hop;
  int              hlined = 0;
  int              llined = 0;
  dlink_node	   *ptr;

  /* Just to be sure -A1kmm. */
  if (!IsServer(source_p))
   return;

  if (parc < 4)
    {
      sendto_one(client_p,"ERROR :No servername");
      return;
    }

  name = parv[1];
  hop = atoi(parv[2]);
  strncpy_irc(info, parv[3], REALLEN);
  info[REALLEN] = '\0';

  if ((target_p = server_exists(name)))
    {
      /*
       * This link is trying feed me a server that I already have
       * access through another path -- multiple paths not accepted
       * currently, kill this link immediately!!
       *
       * Rather than KILL the link which introduced it, KILL the
       * youngest of the two links. -avalon
       *
       * I think that we should exit the link itself, not the introducer,
       * and we should always exit the most recently received(i.e. the
       * one we are receiving this SERVER for. -A1kmm
       *
       * You *cant* do this, if you link somewhere, it bursts you a server
       * that already exists, then sends you a client burst, you squit the
       * server, but you keep getting the burst of clients on a server that
       * doesnt exist, although ircd can handle it, its not a realistic
       * solution.. --fl_ 
       */
      /* It is behind a host-masked server. Completely ignore the
       * server message(don't propagate or we will delink from whoever
       * we propagate to). -A1kmm */
      if (irccmp(target_p->name, name) && target_p->from==client_p)
        return;
      
      if(client_p->firsttime > target_p->from->firsttime)
      {
        sendto_one(client_p, "ERROR :Server %s already exists", name);
	
        sendto_realops_flags(FLAGS_ALL, L_ADMIN,
	                  "Link %s cancelled, server %s already exists",
			  get_client_name(client_p, SHOW_IP), name);
        sendto_realops_flags(FLAGS_ALL, L_OPER,
	                  "Link %s cancelled, server %s already exists",
			  client_p->name, name);
      
        exit_client(client_p, client_p, &me, "Server Exists");
	return;
      }
      else
      {
        sendto_one(target_p->from, "ERROR :Server %s already exists", name);
        sendto_realops_flags(FLAGS_ALL, L_ADMIN,
	            "Link %s cancelled, server %s reintroduced by %s",
		    get_client_name(target_p->from, SHOW_IP),
		    name, client_p->name);
        sendto_realops_flags(FLAGS_ALL, L_ADMIN,
	            "Link %s cancelled, server %s reintroduced by %s",
		    target_p->from->name, name, client_p->name);

        exit_client(target_p->from, target_p->from, &me, "Server Exists");
	return;
      }
  }
  
  /* 
   * User nicks never have '.' in them and server names
   * must always have '.' in them.
   */
  if (strchr(name,'.') == NULL)
    {
      /*
       * Server trying to use the same name as a person. Would
       * cause a fair bit of confusion. Enough to make it hellish
       * for a while and servers to send stuff to the wrong place.
       */
      sendto_one(client_p,"ERROR :Nickname %s already exists!", name);
      sendto_realops_flags(FLAGS_ALL, L_ADMIN,
			   "Link %s cancelled: Server/nick collision on %s",
		/* inpath */ get_client_name(client_p, HIDE_IP), name);
      sendto_realops_flags(FLAGS_ALL, L_OPER,
          "Link %s cancelled: Server/nick collision on %s",
	  get_client_name(client_p, MASK_IP), name);
      exit_client(client_p, client_p, client_p, "Nick as Server");
      return;
    }

  /*
   * Server is informing about a new server behind
   * this link. Create REMOTE server structure,
   * add it to list and propagate word to my other
   * server links...
   */
  if (parc == 1 || info[0] == '\0')
    {
      sendto_one(client_p, "ERROR :No server info specified for %s", name);
      return;
    }

  /*
   * See if the newly found server is behind a guaranteed
   * leaf. If so, close the link.
   *
   */

  for (aconf = ConfigItemList; aconf; aconf=aconf->next)
    {
     if ((aconf->status & (CONF_LEAF|CONF_HUB)) == 0)
       continue;

     if (match(aconf->name, client_p->name))
       {
        if (aconf->status == CONF_HUB)
	  {
	    if(match(aconf->host, name))
	      hlined++;
	  }
        else if (aconf->status == CONF_LEAF)
	  {
	    if(match(aconf->host, name))
	      llined++;
	  }
       }
    }

  /* Ok, this way this works is
   *
   * A server can have a CONF_HUB allowing it to introduce servers
   * behind it.
   *
   * connect {
   *            name = "irc.bighub.net";
   *            hub_mask="*";
   *            ...
   * 
   * That would allow "irc.bighub.net" to introduce anything it wanted..
   *
   * However
   *
   * connect {
   *            name = "irc.somehub.fi";
   *		hub_mask="*";
   *		leaf_mask="*.edu";
   *...
   * Would allow this server in finland to hub anything but
   * .edu's
   */

  /* Ok, check client_p can hub the new server, and make sure it's not a LL */
  if (!hlined || (IsCapable(client_p, CAP_LL) && !IsCapable(client_p, CAP_HUB)))
    {
      /* OOOPs nope can't HUB */
      sendto_realops_flags(FLAGS_ALL, L_ADMIN, "Non-Hub link %s introduced %s.",
                get_client_name(client_p, HIDE_IP), name);

      sendto_realops_flags(FLAGS_ALL, L_OPER,
          "Non-Hub link %s introduced %s.",
	  get_client_name(client_p, MASK_IP), name);
      /* If it is new, we are probably misconfigured, so split the
       * non-hub server introducing this. Otherwise, split the new
       * server. -A1kmm. */
      if ((CurrentTime - source_p->firsttime) < 20)
        {
          exit_client(NULL, source_p, &me, "No matching hub_mask.");
          return;
        }
      else
        {
          sendto_one(source_p, ":%s SQUIT %s :Sorry, no matching hub_mask.",
                     me.name, name);
          return;
        }
    }

  /* Check for the new server being leafed behind this HUB */
  if (llined)
    {
      /* OOOPs nope can't HUB this leaf */
      sendto_realops_flags(FLAGS_ALL, L_ADMIN,
            "link %s introduced leafed %s.",
	    get_client_name(client_p, HIDE_IP), name);
      sendto_realops_flags(FLAGS_ALL, L_OPER, 
            "link %s introduced leafed %s.",
            get_client_name(client_p, MASK_IP), name);
      /* If it is new, we are probably misconfigured, so split the
       * non-hub server introducing this. Otherwise, split the new
       * server. -A1kmm.
       */
      if ((CurrentTime - source_p->firsttime) < 20)
        {
          exit_client(NULL, source_p, &me, "Leafed Server.");
          return;
        }
      else
        {
          sendto_one(source_p, ":%s SQUIT %s :Sorry, Leafed server.",
                     me.name, name);
          return;
        }
    }

  target_p = make_client(client_p);
  make_server(target_p);
  target_p->hopcount = hop;
  strncpy_irc(target_p->name, name, HOSTLEN);
  set_server_gecos(target_p, info);

  target_p->serv->up = find_or_add(parv[0]);
  target_p->servptr = source_p;

  SetServer(target_p);

  Count.server++;

  add_client_to_list(target_p);
  add_server_to_list(target_p);
  add_to_client_hash_table(target_p->name, target_p);
  add_client_to_llist(&(target_p->servptr->serv->servers), target_p);


  /*
   * Old sendto_serv_but_one() call removed because we now
   * need to send different names to different servers
   * (domain name matching)
   */
  for (ptr = serv_list.head; ptr; ptr = ptr->next)
    {
      bclient_p = ptr->data;

      if (bclient_p == client_p)
	continue;
      if (!(aconf = bclient_p->serv->sconf))
	{
	  sendto_realops_flags(FLAGS_ALL, L_ADMIN, 
	        "Lost N-line for %s on %s. Closing",
		get_client_name(client_p, HIDE_IP), name);
	  sendto_realops_flags(FLAGS_ALL, L_OPER, 
	        "Lost N-line for %s on %s. Closing",
		get_client_name(client_p, MASK_IP), name);
	  exit_client(client_p, client_p, client_p, "Lost N line");
          return;
	}
      if (match(my_name_for_link(me.name, aconf), target_p->name))
	continue;

      sendto_one(bclient_p, ":%s SERVER %s %d :%s%s",
		 parv[0], target_p->name, hop + 1,
		 target_p->hidden_server ? "(H) " : "",
		 target_p->info);
    }
      
  sendto_realops_flags(FLAGS_EXTERNAL, L_ALL,
                       "Server %s being introduced by %s",
		       target_p->name, source_p->name);

}

/* set_server_gecos()
 *
 * input	- pointer to client
 * output	- none
 * side effects - servers gecos field is set
 */
int set_server_gecos(struct Client *client_p, char *info)
{
  /* check the info for [IP] */
  if(info[0])
  {
    char *p;
    char *s;
    char *t;
    
    s = info;
    
    /* we should only check the first word for an ip */
    if((p = strchr(s, ' ')))
      *p = '\0';
      
    /* check for a ] which would symbolise an [IP] */
    if((t = strchr(s, ']')))
    {
      /* set s to after the first space */
      if(p)
        s = ++p;
      else
        s = NULL;
    }
    /* no ], put the space back */
    else if(p)
      *p = ' ';

    /* p may have been set to a trailing space, so check s exists and that
     * it isnt \0 */
    if(s && (*s != '\0'))
    {
      /* a space? if not (H) could be the last part of info.. */
      if((p = strchr(s, ' ')))
        *p = '\0';
      
      /* check for (H) which is a hidden server */
      if(!strcmp(s, "(H)"))
      {
        client_p->hidden_server = 1;

        /* if there was no space.. theres nothing to set info to */
        if(p)
	  s = ++p;
	else
	  s = NULL;
      }
      else if(p)
        *p = ' ';
      
      /* if there was a trailing space, s could point to \0, so check */
      if(s && (*s != '\0'))
        strncpy_irc(client_p->info, s, REALLEN);
      else
        strncpy_irc(client_p->info, "(Unknown Location)", REALLEN);
    }
    else
      strncpy_irc(client_p->info, "(Unknown Location)", REALLEN);
  }
  else
    strncpy_irc(client_p->info, "(Unknown Location)", REALLEN);

  return 1;
}

/*
 * bogus_host
 *
 * inputs	- hostname
 * output	- 1 if a bogus hostname input, 0 if its valid
 * side effects	- none
 */
int bogus_host(char *host)
{
  int bogus_server = 0;
  char *s;
  int dots = 0;

  for( s = host; *s; s++ )
    {
      if (!IsServChar(*s))
	{
	  bogus_server = 1;
	  break;
	}
      if ('.' == *s)
	++dots;
    }

  if (!dots || bogus_server )
    return 1;

  return 0;
}

/*
 * server_exists()
 * 
 * inputs	- servername
 * output	- 1 if server exists, 0 if doesnt exist
 */
struct Client *server_exists(char *servername)
{
  struct Client *target_p;
  dlink_node *ptr;

  for(ptr = global_serv_list.head; ptr; ptr = ptr->next)
  {
    target_p = ptr->data;

    if(match(target_p->name, servername) || 
         match(servername, target_p->name))
      return target_p;
  }

  return NULL;
}