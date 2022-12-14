# MMS/MMK Makefile for OpenVMS
# Copyright (c) 2001 Edward Brocklesby

CC=	CC
CFLAGS=	/INCLUDE_DIRECTORY=[-.INCLUDE]/STANDARD=ISOC94
LDFLAGS=

DEFAULT : IRCD.EXE

OBJECTS=	ADNS.OBJ,CHANNEL.OBJ,CLASS.OBJ,CLIENT.OBJ,DEBUG.OBJ,EVENT.OBJ,FDLIST.OBJ,FILEIO.OBJ,GETOPT.OBJ,-
		HASH.OBJ,HOOK.OBJ,HOSTMASK.OBJ,IRC_STRING.OBJ,IRCD_SIGNAL.OBJ,LINEBUF.OBJ,LIST.OBJ,IRCD.OBJ,-
		LISTENER.OBJ,M_ERROR.OBJ,MATCH.OBJ,MD5.OBJ,MEMORY.OBJ,MODULES.OBJ,MOTD.OBJ,NUMERIC.OBJ,PACKET.OBJ,-
		PARSE.OBJ,RESTART.OBJ,RSA.OBJ,S_AUTH.OBJ,S_BSD.OBJ,S_BSD_SELECT.OBJ,S_CONF.OBJ,S_DEBUG.OBJ,-
		S_GLINE.OBJ,S_LOG.OBJ,S_MISC.OBJ,S_SERV.OBJ,S_STATS.OBJ,S_USER.OBJ,SCACHE.OBJ,SEND.OBJ,SPRINTF_IRC.OBJ,-
		VCHANNEL.OBJ,WHOWAS.OBJ,TOOLS.OBJ,RESV.OBJ,KDPARSE.OBJ,-
		CHANNEL_MODE.OBJ,CRYPT.OBJ,BALLOC.OBJ,SNPRINTF.OBJ

VERSION.C : VERSION.COM
	@- @VERSION

VERSION.OBJ : VERSION.C

IRCD_PARSER_TAB.OBJ : IRCD_PARSER_TAB.C

IRCD_PARSER_TAB.C : IRCD_PARSER.Y
	BISON -d IRCD_PARSER.Y

LEXYY.OBJ : LEXYY.C

LEXYY.C : IRCD_LEXER.L
	FLEX IRCD_LEXER.L

IRCD.EXE : $(OBJECTS) IRCD_PARSER_TAB.OBJ LEXYY.OBJ VERSION.OBJ
	$(LINK)$(LDFLAGS)/EXECUTABLE=IRCD $(OBJECTS), IRCD_PARSER_TAB.OBJ, LEXYY.OBJ, VERSION.OBJ, [-.adns]adns.olb/LIB, [-.modules]modules.olb/LIB

CLEAN : 
	DELETE *.OBJ;*
	DELETE *.OLB;*
	DELETE *.EXE;*
