/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  hook.c: Provides a generic event hooking interface.
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
 * $Id: hook.c,v 1.4 2002/02/09 08:56:52 a1kmm Exp $
 */

/* hooks are used by modules to hook into events called by other parts of
   the code.  modules can also register their own events, and call them
   as appropriate. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tools.h"
#include "list.h"
#include "hook.h"
#include "memory.h"

dlink_list hooks;

void
init_hooks(void)
{
  memset(&hooks, 0, sizeof(hooks));
#ifndef NDEBUG
  hook_add_event("iosend");
  hook_add_event("iorecv");
  hook_add_event("iorecvctrl");
#endif
  hook_add_event("burst_channel");
}

static hook *
new_hook(char *name)
{
  hook *h;

  h = MyMalloc(sizeof(hook));
  memset(h, 0, sizeof(*h));
  DupString(h->name, name);
  return h;
}

int
hook_add_event(char *name)
{
  dlink_node *node;
  hook *newhook;

  node = make_dlink_node();
  newhook = new_hook(name);

  dlinkAdd(newhook, node, &hooks);
  return 0;
}

int
hook_del_event(char *name)
{
  dlink_node *node;
  hook *h;

  for (node = hooks.head; node; node = node->next)
  {
    h = node->data;

    if (!strcmp(h->name, name))
    {
      dlinkDelete(node, &hooks);
      MyFree(h);
      return 0;
    }
  }
  return 0;
}

static hook *
find_hook(char *name)
{
  dlink_node *node;
  hook *h;

  for (node = hooks.head; node; node = node->next)
  {
    h = node->data;

    if (!strcmp(h->name, name))
      return h;
  }
  return NULL;
}

int
hook_del_hook(char *event, hookfn * fn)
{
  hook *h;
  dlink_node *node, *nnode;
  h = find_hook(event);
  if (!h)
    return -1;

  for (node = h->hooks.head; node; node = nnode)
  {
    nnode = node->next;
    if (fn == node->data)
    {
      dlinkDelete(node, &h->hooks);
      free_dlink_node(node);
    }
  }
  return 0;
}

int
hook_add_hook(char *event, hookfn * fn)
{
  hook *h;
  dlink_node *node;

  h = find_hook(event);
  if (!h)
    return -1;

  node = make_dlink_node();

  dlinkAdd(fn, node, &h->hooks);
  return 0;
}

int
hook_call_event(char *event, void *data)
{
  hook *h;
  dlink_node *node;
  hookfn fn;

  h = find_hook(event);
  if (!h)
    return -1;

  for (node = h->hooks.head; node; node = node->next)
  {
    fn = (hookfn) node->data;

    if (fn(data) != 0)
      return 0;
  }
  return 0;
}
