/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  ircd_signal.c: Contains the ircd signal handlers.
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
 *   $Id: ircd_signal.c,v 1.3 2002/01/06 07:18:49 a1kmm Exp $
 */

#include <signal.h>
#include <stdlib.h>

#include "ircd_signal.h"
#include "ircd.h"               /* dorehash */
#include "restart.h"            /* server_reboot */
#include "s_log.h"
#include "memory.h"
#include "s_bsd.h"
/*
 * dummy_handler - don't know if this is really needed but if alarm is still
 * being used we probably will
 */
static void
dummy_handler(int sig)
{
  /* Empty */
}

/*
 * sigterm_handler - exit the server
 */
static void
sigterm_handler(int sig)
{
  /* XXX we had a flush_connections() here - we should close all the
   * connections and flush data. read server_reboot() for my explanation.
   *     -- adrian
   */
  ilog(L_CRIT, "Server killed By SIGTERM");
  exit(-1);
}

/* 
 * sighup_handler - reread the server configuration
 */
static void
sighup_handler(int sig)
{
  dorehash = 1;
}

/*
 * sigint_handler - restart the server
 */
static void
sigint_handler(int sig)
{
  static int restarting = 0;

  if (server_state.foreground)
  {
    ilog(L_WARN, "Server exiting on SIGINT");
    exit(0);
  }
  else
  {
    ilog(L_WARN, "Server Restarting on SIGINT");
    if (restarting == 0)
    {
      restarting = 1;
      server_reboot();
    }
  }
}

/*
 * setup_signals - initialize signal handlers for server
 */
void
setup_signals()
{
  struct sigaction act;

  act.sa_flags = 0;
  act.sa_handler = SIG_IGN;
  sigemptyset(&act.sa_mask);
  sigaddset(&act.sa_mask, SIGPIPE);
  sigaddset(&act.sa_mask, SIGALRM);

# ifdef SIGWINCH
  sigaddset(&act.sa_mask, SIGWINCH);
  sigaction(SIGWINCH, &act, 0);
# endif
  sigaction(SIGPIPE, &act, 0);

  act.sa_handler = dummy_handler;
  sigaction(SIGALRM, &act, 0);

  act.sa_handler = sighup_handler;
  sigemptyset(&act.sa_mask);
  sigaddset(&act.sa_mask, SIGHUP);
  sigaction(SIGHUP, &act, 0);

  act.sa_handler = sigint_handler;
  sigaddset(&act.sa_mask, SIGINT);
  sigaction(SIGINT, &act, 0);

  act.sa_handler = sigterm_handler;
  sigaddset(&act.sa_mask, SIGTERM);
  sigaction(SIGTERM, &act, 0);
#ifdef USE_SIGIO
  act.sa_handler = do_sigio;
  sigaddset(&act.sa_mask, SIGIO);
  sigaction(SIGIO, &act, 0);
#endif

}
