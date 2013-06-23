# $Gateweaver: Makefile,v 1.8 2011/10/13 16:42:41 cmaxwell Exp $
PROG=chpass.cgi
SRCS=chpass.c cgi.c output.c conf.c
NOMAN=1

LOCALBASE?=	/usr/local
BINDIR?=	${LOCALBASE}/share/chpass
BINMODE?=	555
LDSTATIC=	${STATIC}

CFLAGS+=-Wall 
#-Werror
CFLAGS+=-Wshadow -Wpointer-arith -Wcast-align -Wcast-qual
CFLAGS+=-Waggregate-return
CFLAGS+=-Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations
CFLAGS+=-Wsign-compare -Wbounded -Wformat

LDADD+=-lkrb5 -lcrypto

beforeinstall:
	${INSTALL} -d ${DESTDIR}/${LOCALBASE}/share/chpass

afterinstall:
	${INSTALL} ${INSTALL_COPY} ${.CURDIR}/chpass.conf \
		${DESTDIR}/${LOCALBASE}/share/chpass/
	${INSTALL} ${INSTALL_COPY} ${.CURDIR}/success.html \
		${DESTDIR}/${LOCALBASE}/share/chpass/
	${INSTALL} ${INSTALL_COPY} ${.CURDIR}/front.html \
		${DESTDIR}/${LOCALBASE}/share/chpass/

.include <bsd.prog.mk>
