/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  s_protocol.c: Functions related to multiple protocol support.
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
 *  $Id: s_protocol.c,v 1.1 2002/01/13 07:15:39 a1kmm Exp $
 */

#include "s_protocol.h"
#include "send.h"
#include "client.h"
#include "numeric.h"
#include "channel.h"
#include <stdlib.h>

