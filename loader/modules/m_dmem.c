/************************************************************************
 *   IRC - Internet Relay Chat, modules/m_rehash.c
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
 *   $Id: m_dmem.c,v 1.1 2002/01/04 09:15:10 a1kmm Exp $
 */
#include "handlers.h"
#include "client.h"
#include "common.h"
#include "irc_string.h"
#include "ircd.h"
#include "ircd_defs.h"
#include "list.h"
#include "s_gline.h"
#include "numeric.h"
#include "res.h"
#include "s_conf.h"
#include "s_log.h"
#include "send.h"
#include "msg.h"
#include "parse.h"
#include "modules.h"

static void mo_dmem(struct Client*, struct Client*, int, char**);

struct Message dmem_msgtab = {
  "DMEM", 0, 0, 0, 0, MFLG_SLOW, 0,
  {m_unregistered, m_not_oper, m_ignore, mo_dmem}
};
#ifndef STATIC_MODULES
void
_modinit(void)
{
  mod_add_cmd(&dmem_msgtab);
}

void
_moddeinit(void)
{
  mod_del_cmd(&dmem_msgtab);
}

char *_version = "$Revision: 1.1 $";
#endif
#ifdef MEMDEBUG
void ReportAllocated(struct Client*);
#endif

/*
 * mo_dmem - DMEM message handler
 *
 */
static void
mo_dmem(struct Client *client_p, struct Client *source_p, int parc, char *parv[])
{
#ifdef MEMDEBUG
  if (parc < 2)
    ReportAllocated(source_p);
  else
    log_memory();
#else
  sendto_one(source_p, ":%s NOTICE %s :Compiled without memory debugging",
    me.name, source_p->name);
#endif
}
