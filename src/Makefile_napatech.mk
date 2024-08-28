default: filter

LIB_DIR     = ../lib
BIN_DIR     = ../bin_napatech

LDLIBS   += -L /opt/napatech3/lib -lpcap -ljson

COMPONENT_DIR = 
ROOT_DIR      = ..
SOURCE_DIR    = $(ROOT_DIR)/src/$(COMPONENT_DIR)
BUILD_DIR     = $(ROOT_DIR)/build/$(COMPONENT_DIR)

SOURCES        := $(wildcard $(SOURCE_DIR)/filterprograms/*.c) $(wildcard $(SOURCE_DIR)/pcapsession/*.c) $(wildcard $(SOURCE_DIR)/gtp/*.c) $(wildcard $(SOURCE_DIR)/utilities/*.c)
COMMON_SOURCES := $(wildcard $(SOURCE_DIR)/pcapsession/*.c) $(wildcard $(SOURCE_DIR)/gtp/*.c) $(wildcard $(SOURCE_DIR)/utilities/*.c)
OBJECTS        := $(subst $(ROOT_DIR)/src, $(ROOT_DIR)/build, $(SOURCES:.c=.o))
COMMON_OBJECTS := $(subst $(ROOT_DIR)/src, $(ROOT_DIR)/build, $(COMMON_SOURCES:.c=.o))

include Makefile_common.mk

filter: build $(BIN_DIR) $(BIN_DIR)/pcapdistributer $(BIN_DIR)/pcapmerger $(BIN_DIR)/decodegtp $(BIN_DIR)/gtpimsieua $(BIN_DIR)/gtpaddr $(BIN_DIR)/usectstamp $(BIN_DIR)/gtpuntunnel $(BIN_DIR)/chunkpcapfile

build: $(BUILD_DIR) $(BUILD_DIR)/filterprograms $(BUILD_DIR)/pcapsession $(BUILD_DIR)/gtp  $(BUILD_DIR)/utilities $(OBJECTS)

$(BIN_DIR)/pcapdistributer: $(COMMON_OBJECTS) $(BUILD_DIR)/filterprograms/pcapdistributer.o
	gcc ${CXXFLAGS} ${CFLAGS} $(LDLIBS) $(COMMON_OBJECTS) $(BUILD_DIR)/filterprograms/pcapdistributer.o -o $@

$(BIN_DIR)/pcapmerger: $(COMMON_OBJECTS) $(BUILD_DIR)/filterprograms/pcapmerger.o
	gcc ${CXXFLAGS} ${CFLAGS} $(LDLIBS) $(COMMON_OBJECTS) $(BUILD_DIR)/filterprograms/pcapmerger.o -o $@

$(BIN_DIR)/gtpimsieua: $(COMMON_OBJECTS) $(BUILD_DIR)/filterprograms/gtpimsieua.o
	gcc ${CXXFLAGS} ${CFLAGS} $(LDLIBS) $(COMMON_OBJECTS) $(BUILD_DIR)/filterprograms/gtpimsieua.o -o $@

$(BIN_DIR)/decodegtp: $(COMMON_OBJECTS) $(BUILD_DIR)/filterprograms/decodegtp.o
	gcc ${CXXFLAGS} ${CFLAGS} $(LDLIBS) $(COMMON_OBJECTS) $(BUILD_DIR)/filterprograms/decodegtp.o -o $@

$(BIN_DIR)/gtpaddr: $(COMMON_OBJECTS) $(BUILD_DIR)/filterprograms/gtpaddr.o
	gcc ${CXXFLAGS} ${CFLAGS} $(LDLIBS) $(COMMON_OBJECTS) $(BUILD_DIR)/filterprograms/gtpaddr.o -o $@

$(BIN_DIR)/usectstamp: $(COMMON_OBJECTS) $(BUILD_DIR)/filterprograms/usectstamp.o
	gcc ${CXXFLAGS} ${CFLAGS} $(LDLIBS) $(COMMON_OBJECTS) $(BUILD_DIR)/filterprograms/usectstamp.o -o $@

$(BIN_DIR)/gtpuntunnel: $(COMMON_OBJECTS) $(BUILD_DIR)/filterprograms/gtpuntunnel.o
	gcc ${CXXFLAGS} ${CFLAGS} $(LDLIBS) $(COMMON_OBJECTS) $(BUILD_DIR)/filterprograms/gtpuntunnel.o -o $@

$(BIN_DIR)/chunkpcapfile: $(COMMON_OBJECTS) $(BUILD_DIR)/filterprograms/chunkpcapfile.o
	gcc ${CXXFLAGS} ${CFLAGS} $(LDLIBS) $(COMMON_OBJECTS) $(BUILD_DIR)/filterprograms/chunkpcapfile.o -o $@

clean:
	rm -fr $(BUILD_DIR) $(LIB_DIR) $(BIN_DIR)
	
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	
$(BUILD_DIR)/filterprograms:
	mkdir -p $(BUILD_DIR)/filterprograms
	
$(BUILD_DIR)/pcapsession:
	mkdir -p $(BUILD_DIR)/pcapsession
	
$(BUILD_DIR)/gtp:
	mkdir -p $(BUILD_DIR)/gtp

$(BUILD_DIR)/utilities:
	mkdir -p $(BUILD_DIR)/utilities
		
$(BIN_DIR):
	mkdir -p $(BIN_DIR)
