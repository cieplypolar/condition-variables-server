# Makefile for CVS
PROG=	cvs
SRCS=	main.c structures.c signals.c requests.c utils.c

DPADD+=	${LIBSYS}
LDADD+=	-lsys

MAN=

BINDIR?= /usr/sbin
FILES=cvs.conf
FILESNAME=cvs
FILESDIR= /etc/system.conf.d

.include <minix.service.mk>
