/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  spy_stats_p_notice.c: Sends a notice when someone uses STATS p.
 *
 * $Id: spy_stats_p_notice.c,v 1.2 2002/01/06 07:18:07 a1kmm Exp $
 */

#include "modules.h"
#include "hook.h"
#include "client.h"
#include "ircd.h"
#include "send.h"

int
show_stats_p(struct hook_stats_data *);

void
_modinit(void)
{
  hook_add_hook("doing_stats_p", (hookfn *)show_stats_p);
}

void
_moddeinit(void)
{
  hook_del_hook("doing_stats_p", (hookfn *)show_stats_p);
}

char *_version = "$Revision: 1.2 $";

int show_stats_p(struct hook_stats_data *data)
{
  sendto_realops_flags(FLAGS_SPY, L_ALL,
                       "STATS p requested by %s (%s@%s) [%s]",
  	               data->source_p->name, data->source_p->username,
		       data->source_p->host, data->source_p->user->server);

  return 0;
}
