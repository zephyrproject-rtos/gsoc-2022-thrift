# Copyright 2022 Meta
# SPDX-License-Identifier: Apache-2.0

.PHONY: all clean

CXXFLAGS :=
#CXXFLAGS += -Wall -Wextra -Werror
CXXFLAGS += -std=c++17

GEN_DIR = gen-cpp
GENSRC = $(GEN_DIR)/Hello.cpp $(GEN_DIR)/Hello.h $(GEN_DIR)/hello_types.h
GENHDR = $(filter %.h, $(GENSRC))
GENOBJ = $(filter-out %.h, $(GENSRC:.cpp=.o))

THRIFT_FLAGS :=
THRIFT_FLAGS += $(shell pkg-config --cflags thrift)
THRIFT_FLAGS += -I$(GEN_DIR)
THRIFT_LIBS = $(shell pkg-config --libs thrift)

all: hello_server

hello_server.stamp: ../../../../thrift/hello.thrift
	thrift --gen cpp:no_skeleton $<

$(GENSRC): hello_server.stamp

%.o: %.cpp $(GENHDR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(THRIFT_FLAGS) -o $@ -c $<

hello_server: src/main.cpp  $(GENOBJ) $(GENHDR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(THRIFT_FLAGS) -o $@ $< $(GENOBJ) $(THRIFT_LIBS)

clean:
	rm -Rf hello_server $(GEN_DIR)
