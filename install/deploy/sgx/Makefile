DESTDIR = /
prefix = /usr
etcdir = /etc
bindir = ${prefix}/bin
incdir = ${prefix}/include
libdir = ${prefix}/lib

all:

install: 
	mkdir -p ${DESTDIR}${etcdir}
	mkdir -p ${DESTDIR}${bindir}
	mkdir -p ${DESTDIR}${incdir}
	mkdir -p ${DESTDIR}${libdir}
	mv  etc/* ${DESTDIR}${etcdir}
	mv  bin/* ${DESTDIR}${bindir}
	mv  include/* ${DESTDIR}${incdir}
	mv  lib/* ${DESTDIR}${libdir}
