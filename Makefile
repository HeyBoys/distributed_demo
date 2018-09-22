CC = cc
CP = cp
RM = rm -f
MV = mv -f
TOP_DIR = ../amp-wz
INC_AMP = ${TOP_DIR}/include/
INC_THIS = ./
CFLAGS = -I${INC_AMP} -I${INC_THIS}

LDPATH_AMP = ${TOP_DIR}/lib/
LIBNAME_AMP = amp
LDFLAGS_AMP = -L${LDPATH_AMP} -l${LIBNAME_AMP}
LDFLAGS = -lm -lpthread -lrt -pthread


EXE_FILES=server client\
			test_client_multhread_read_data \
			test_server_multhread_read_data\
			test_client_read \
			test_server_read
			
all: ${EXE_FILES} 
SERVER_OBJ=server.o analysis.o
CLIENT_OBJ=client.o fuse_demo.o
TEST_CLIENT_MULTHREAD_READ_DATA_OBJ=test_client_multhread_read_data.o
TEST_SERVER_MULTHREAD_READ_DATA_OBJ=test_server_multhread_read_data.o
TEST_SERVER_READ_OBJ=test_server_read.o
TEST_CLIENT_READ_OBJ=test_client_read.o

OBJS = server.o client.o analysis.o fuse_demo.o test_client_multhread_read_data.o test_server_multhread_read_data.o test_server_read.o test_client_read.o

	
.c.o:
	${CC} ${CFLAGS} ${EXTRA_CFLAGS} -g  -c $*.c `pkg-config fuse --cflags --libs`
##SERVER_OBJ:analysis.h
##	${CC} ${CFLAGS} ${EXTRA_CFLAGS} -g  -c server.c 
##CLIENT_OBJ:
##	${CC} ${CFLAGS} ${EXTRA_CFLAGS} -g  -c client.c 

server: server.o analysis.o
	${CC}  -O2 -g -o $@ ${SERVER_OBJ} ${LDFLAGS} ${LDFLAGS_AMP} 

client: client.o fuse_demo.o
	${CC}  -O2 -g -o $@ ${CLIENT_OBJ} ${LDFLAGS} ${LDFLAGS_AMP} `pkg-config fuse --cflags --libs`

test_client_multhread_read_data: test_client_multhread_read_data.o
	${CC} -O2 -g -o $@ ${TEST_CLIENT_MULTHREAD_READ_DATA_OBJ} ${LDFLAGS} ${LDFLAGS_AMP}
test_server_multhread_read_data: test_server_multhread_read_data.o
	${CC} -O2 -g -o $@ ${TEST_SERVER_MULTHREAD_READ_DATA_OBJ} ${LDFLAGS} ${LDFLAGS_AMP}
test_server_read:test_server_read.o
	${CC} -O2 -g -o $@ ${TEST_SERVER_READ_OBJ} ${LDFLAGS} ${LDFLAGS_AMP}
test_client_read:test_client_read.o
	${CC} -O2 -g -o $@ ${TEST_CLIENT_READ_OBJ} ${LDFLAGS} ${LDFLAGS_AMP}
clean:
	${RM} *.o core ~* *.cpp ${EXE_FILES}
