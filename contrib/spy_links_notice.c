/* contrib/spy_links_notice.c
 * Copyright (c) 2000 Edward Brocklesby, Hybrid Development Team
 *
 * $Id: spy_links_notice.c,v 1.1 2002/01/04 09:12:05 a1kmm Exp $
 */

#include "modules.h"
#include "hook.h"
#include "client.h"
#include "ircd.h"
#include "send.h"

int
show_links(struct hook_links_data *);

void
_modinit(void)
{
  hook_add_hook("doing_links", (hookfn *)show_links);
}

void
_moddeinit(void)
{
  hook_del_hook("doing_links", (hookfn *)show_links);
}

char *_version = "$Revision: 1.1 $";

int
show_links(struct hook_links_data *data)
{
  if(MyConnect(data->source_p))
    sendto_realops_flags(FLAGS_SPY, L_ALL,
                         "LINKS '%s' requested by %s (%s@%s) [%s]",
                         data->mask, data->source_p->name, data->source_p->username,
                         data->source_p->host, data->source_p->user->server);

  return 0;
}
