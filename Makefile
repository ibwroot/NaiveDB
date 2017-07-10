SOFA_PBRPC_DIR=./thirdparty
GFLAG_DIR=./thirdparty

OPT ?= -O2

include depends.mk

CXX=g++
INCPATH=-I./src -I$(SOFA_PBRPC_DIR)/include -I$(PROTOBUF_DIR)/include \
		-I$(SNAPPY_DIR)/include -I$(ZLIB_DIR)/include \
		-T$(LEVELDB_DIR)/include -I$(GFLAG_DIR)/include
CXXFLAGS += --std=c++11 $(OPT) -pipe -W -Wall -fPIC -D_GNU_SOURCE -D__STDC_LIMIT_MACROS $(INCPATH)

LIBRARY=$(SOFA_PBRPC_DIR)/lib/libsofa-pbrpc.a $(PROTOBUF_DIR)/lib/libprotobuf.a $(SNAPPY_DIR)/lib/libsnappy.a \
		$(LEVELDB_DIR)/lib/libleveldb.a $(GFLAG_DIR)/lib/libgflags.a
LDFLAGS += -L$(ZLIB_DIR)/lib -lpthread -lz

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LDFLAGS += -lrt
else
    ifeq ($(UNAME_S),Darwin)
        LDFLAGS += -undefined dynamic_lookup
    endif
endif

PROTO_FILE = $(wildcard src/proto/*.proto)
PROTO_SRC = $(patsubst %.proto,%.pb.cc,$(PROTO_FILE))
PROTO_HEADER = $(patsubst %.proto,%.pb.h,$(PROTO_FILE))
PROTO_OBJ = $(patsubst %.proto,%.pb.o,$(PROTO_FILE))

SERVER_SRC = $(wildcard src/server/*.cc)
SERVER_OBJ = $(patsubst %.cc, %.o, $(SERVER_SRC))
SERVER_HEADER = $(wildcard src/server/*.h)

CLIENT_OBJ = $(patsubst %.cc, %.o, $(wildcard src/client/*.cc))

CLIENT_TEST_OBJ = $(patsubst %.cc, %.o, $(wildcard src/client_test/*.cc))

PROTO_OPTIONS=--proto_path=./src/proto --proto_path=$(SOFA_PBRPC_DIR)/include --proto_path=$(PROTOBUF_DIR)/include --cpp_out=./src/proto/

BIN=server client #client_test

all: check_depends $(BIN)

.PHONY: check_depends clean

check_depends:
	@if [ ! -f "$(PROTOBUF_DIR)/include/google/protobuf/message.h" ]; then echo "ERROR: need protobuf header"; exit 1    ; fi
	@if [ ! -f "$(PROTOBUF_DIR)/lib/libprotobuf.a" ]; then echo "ERROR: need protobuf lib"; exit 1; fi
	@if [ ! -f "$(PROTOBUF_DIR)/bin/protoc" ]; then echo "ERROR: need protoc binary"; exit 1; fi
	@if [ ! -f "$(SNAPPY_DIR)/include/snappy.h" ]; then echo "ERROR: need snappy header"; exit 1; fi
	@if [ ! -f "$(SNAPPY_DIR)/lib/libsnappy.a" ]; then echo "ERROR: need snappy lib"; exit 1; fi
	@if [ ! -f "$(SOFA_PBRPC_DIR)/include/sofa/pbrpc/pbrpc.h" ]; then echo "ERROR: need sofa-pbrpc header"; exit 1; fi
	@if [ ! -f "$(SOFA_PBRPC_DIR)/lib/libsofa-pbrpc.a" ]; then echo "ERROR: need sofa-pbrpc lib"; exit 1; fi
 
clean:
	@rm -f $(BIN) src/server/*.o src/proto/*.pb.* src/client/*.o src/client_test/*.o

rebuild: clean all

server: $(PROTO_OBJ) $(SERVER_OBJ)
	$(CXX) $^ -o $@ $(LIBRARY) $(LDFLAGS)

client: $(PROTO_OBJ) $(CLIENT_OBJ)
	$(CXX) $^ -o $@ $(LIBRARY) $(LDFLAGS)

client_test: $(PROTO_OBJ) $(CLIENT_TEST_OBJ)
	$(CXX) $^ -o $@ $(LIBRARY) $(LDFLAGS)

%.pb.o: %.pb.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@
%.pb.h %.pb.cc: %.proto
	$(PROTOBUF_DIR)/bin/protoc $(PROTO_OPTIONS) $<
%.o: %.cc $(PROTO_OBJ)
	$(CXX) $(CXXFLAGS) -c $< -o $@
