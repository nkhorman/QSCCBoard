PROG=		prog

SRCSC=		
SRCSCPP=	main.cpp CBoard.cpp
SRCS=		${SRCSC} ${SRCSCPP}

OBJSC=		${SRCSC:.c=.o}
OBJSCPP=	${SRCSCPP:.cpp=.o}
OBJS=		${OBJSC} ${OBJSCPP}

INCLUDESC=	${SRCSC:.c=.h}
INCLUDESCPP=	${SRCSCPP:.cpp=.h}
INCLUDES=	${INCLUDESC} ${INCLUDESCPP}

INCLUDEDIR=	-I.
#OPENSSLPKGDIR=	$(shell brew info openssl | grep PKG_CONFIG_PATH | sed -e's/[^:]\{1,\}: //')
#INCLUDEDIR+=	$(shell PKG_CONFIG_PATH="${OPENSSLPKGDIR}" pkg-config openssl --cflags)
#LIBS+=		$(shell PKG_CONFIG_PATH="${OPENSSLPKGDIR}" pkg-config openssl --libs)

LIBDIRS=	


#CFLAGS+=	-O -Wall
CFLAGS+=	-MP
CFLAGS+=	-g3 -ggdb
#CFLAGS+=	-D_DEBUG

#CXXFLAGS+=	-O -Wall
#CXXFLAGS+=	-Wno-unused -Wno-c++11-compat-deprecated-writable-strings -MP
CXXFLAGS+=	-Wno-unused -MP
CXXFLAGS+=	-std=c++11
CXXFLAGS+=	-g3 -ggdb
#CXXFLAGS+=	-D_DEBUG

LFLAGS=		

all: ${PROG}

test:	${PROG}
	./${PROG}

dbg:	${PROG}
	lldb ./${PROG}

clean:
	@echo Cleaning...
	@rm -f ${PROG} *.core *.d ${OBJS}

.PHONY: all test clean

${PROG}: Makefile ${SRCS} ${OBJS} ${INCLUDES}
	@echo Linking $@ ...
	@${CXX} ${LFLAGS} -o $@ ${OBJS} ${LIBDIRS} ${LIBS}

-include ${OBJS:.o=.d}

.c.o: Makefile ${INCLUDESC}
	@echo "CC $< --> $@"
	@${CC} ${CFLAGS} ${INCLUDEDIR} -c $< -o $@

.cpp.o: Makefile ${INCLUDESCPP}
	@echo "CXX $< --> $@"
	@${CXX} ${CXXFLAGS} ${INCLUDEDIR} -c $< -o $@
