/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  s_user.h: The user functions header.
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
 *   $Id: s_user.h,v 1.3 2002/04/26 04:00:24 a1kmm Exp $
 */

#ifndef INCLUDED_s_user_h
#define INCLUDED_s_user_h
#ifndef INCLUDED_config_h
#include "config.h"
#endif
#ifndef INCLUDED_sys_types_h
#include <sys/types.h>      /* time_t */
#define INCLUDED_sys_types_h
#endif

struct Client;
extern time_t LastUsedWallops;


extern void user_mode(struct Client* , struct Client* , int, char**);
extern void send_umode (struct Client* , struct Client* ,
                        int, int, char* );
extern void send_umode_out(struct Client*, struct Client*, int);
extern void show_lusers(struct Client *source_p);
extern void register_local_user(struct Client*, struct Client*,
                               char*, char*);
extern void register_remote_user(struct Client*, struct Client*, char*, char*);
extern void do_local_user(char*, struct Client*, struct Client*,
                          char*, char*, char*, char*);

extern void do_remote_user(char*, struct Client*, struct Client*,
                           char*, char*, char*, char*, char*);

extern int user_modes_from_c_to_bitmask[];
extern void show_isupport(struct Client*);
extern void ts_burst_client(struct Client *client_p, struct Client *target_p,
                            int bursttype, int burstflags);
#endif
