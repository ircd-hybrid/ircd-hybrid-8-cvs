/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  m_resv.c: Reserves(jupes) a nickname or channel.
 *
 * $Id: m_resv.c,v 1.4 2002/01/13 07:15:19 a1kmm Exp $
 */

#include "handlers.h"
#include "client.h"
#include "channel.h"
#include "ircd.h"
#include "numeric.h"
#include "s_serv.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"
#include "s_conf.h"
#include "resv.h"
#include "hash.h"

static void mo_resv(struct Client *, struct Client *, int, char **);
static void mo_unresv(struct Client *, struct Client *, int, char **);

struct Message resv_msgtab[] = {
  {"RESV", 0, 0, 3, 0, MFLG_SLOW | MFLG_UNREG, 0, &p_unregistered,
   &m_unregistered},
  {"UNRESV", 0, 0, 2, 0, MFLG_SLOW | MFLG_UNREG, 0, &p_unregistered,
   &m_unregistered},
  {"RESV", 0, 0, 3, 0, MFLG_SLOW, 0, &p_user, &m_not_oper},
  {"UNRESV", 0, 0, 2, 0, MFLG_SLOW, 0, &p_user, &m_not_oper},
  {"RESV", 0, 0, 3, 0, MFLG_SLOW, 0, &p_operuser, &mo_resv},
  {"UNRESV", 0, 0, 3, 0, MFLG_SLOW, 0, &p_operuser, &mo_unresv},
  {NULL, 0, 0, 0, 0, 0, 0, NULL, NULL}
};

#ifndef STATIC_MODULES
void
_modinit(void)
{
  mod_add_cmd(resv_msgtab);
}

void
_moddeinit(void)
{
  mod_del_cmd(resv_msgtab);
}

char *_version = "$Revision: 1.4 $";
#endif

/*
 * mo_resv()
 *      parv[0] = sender prefix
 *      parv[1] = channel/nick to forbid
 */

static void
mo_resv(struct Client *client_p, struct Client *source_p,
        int parc, char *parv[])
{
  if (BadPtr(parv[1]))
    return;

  if (IsChannelName(parv[1]))
  {
    struct ResvChannel *resv_p;

    resv_p = create_channel_resv(parv[1], parv[2], 0);

    if (!(resv_p))
    {
      sendto_one(source_p,
                 ":%s NOTICE %s :A RESV has already been placed on channel: %s",
                 me.name, source_p->name, parv[1]);
      return;
    }

    sendto_one(source_p,
               ":%s NOTICE %s :A local RESV has been placed on channel: %s [%s]",
               me.name, source_p->name, resv_p->name, resv_p->reason);

    sendto_realops_flags(FLAGS_ALL, L_ALL,
                         "%s has placed a local RESV on channel: %s [%s]",
                         get_oper_name(source_p),
                         resv_p->name, resv_p->reason);
  }
  else if (clean_resv_nick(parv[1]))
  {
    struct ResvNick *resv_p;

    if ((strchr(parv[1], '*') || strchr(parv[1], '?')) && !IsAdmin(source_p))
    {
      sendto_one(source_p,
                 ":%s NOTICE %s :You must be an admin to perform a wildcard RESV",
                 me.name, source_p->name);
      return;
    }

    resv_p = create_nick_resv(parv[1], parv[2], 0);

    if (!(resv_p))
    {
      sendto_one(source_p,
                 ":%s NOTICE %s :A RESV has already been placed on nick: %s",
                 me.name, source_p->name, parv[1]);
      return;
    }

    sendto_one(source_p,
               ":%s NOTICE %s :A local RESV has been placed on nick: %s [%s]",
               me.name, source_p->name, resv_p->name, resv_p->reason);

    sendto_realops_flags(FLAGS_ALL, L_ALL,
                         "%s has placed a local RESV on nick: %s [%s]",
                         get_oper_name(source_p),
                         resv_p->name, resv_p->reason);
  }
  else
    sendto_one(source_p,
               ":%s NOTICE %s :You have specified an invalid resv: [%s]",
               me.name, source_p->name, parv[1]);
}

/*
 * mo_unresv()
 *     parv[0] = sender prefix
 *     parv[1] = channel/nick to unforbid
 */

static void
mo_unresv(struct Client *client_p, struct Client *source_p,
          int parc, char *parv[])
{
  if (IsChannelName(parv[1]))
  {
    struct ResvChannel *resv_p;

    if (!ResvChannelList ||
        !(resv_p = (struct ResvChannel *)hash_find_resv(parv[1])))
    {
      sendto_one(source_p,
                 ":%s NOTICE %s :A RESV does not exist for channel: %s",
                 me.name, source_p->name, parv[1]);
      return;
    }

    else if (resv_p->conf)
    {
      sendto_one(source_p,
                 ":%s NOTICE %s :The RESV for channel: %s is in the config file and must be removed by hand.",
                 me.name, source_p->name, parv[1]);
      return;
    }
    /* otherwise, delete it */
    else
    {
      delete_channel_resv(resv_p);

      sendto_one(source_p,
                 ":%s NOTICE %s :The local RESV has been removed on channel: %s",
                 me.name, source_p->name, parv[1]);

      sendto_realops_flags(FLAGS_ALL, L_ALL,
                           "%s has removed the local RESV for channel: %s",
                           get_oper_name(source_p), parv[1]);

    }
  }
  else if (clean_resv_nick(parv[1]))
  {
    struct ResvNick *resv_p;

    if (!ResvNickList || !(resv_p = return_nick_resv(parv[1])))
    {
      sendto_one(source_p,
                 ":%s NOTICE %s :A RESV does not exist for nick: %s",
                 me.name, source_p->name, parv[1]);
      return;
    }

    else if (resv_p->conf)
    {
      sendto_one(source_p,
                 ":%s NOTICE %s :The RESV for nick: %s is in the config file and must be removed by hand.",
                 me.name, source_p->name, parv[1]);
      return;
    }

    else
    {
      delete_nick_resv(resv_p);

      sendto_one(source_p,
                 ":%s NOTICE %s :The local RESV has been removed on nick: %s",
                 me.name, source_p->name, parv[1]);

      sendto_realops_flags(FLAGS_ALL, L_ALL,
                           "%s has removed the local RESV for nick: %s",
                           get_oper_name(source_p), parv[1]);
    }
  }
}
