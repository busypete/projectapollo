DESTDIR = /

prefix = usr
etcdir = etc
bindir = ${prefix}/bin
incdir = ${prefix}/include
libdir = ${prefix}/lib

all:

install: 
	mkdir -p ${DESTDIR}${etcdir}
	mkdir -p ${DESTDIR}${bindir}
	mkdir -p ${DESTDIR}${incdir}
	mkdir -p ${DESTDIR}${libdir}
	cp -ar etc/* ${DESTDIR}${etcdir}
	cp -ar bin/* ${DESTDIR}${bindir}
	cp -ar include/* ${DESTDIR}${incdir}
	cp -ar lib/* ${DESTDIR}${libdir}
