default: filter

LIB_DIR     = ../lib
BIN_DIR     = ../bin

LDLIBS   += -lpcap -ljson

COMPONENT_DIR = 
ROOT_DIR      = ..
SOURCE_DIR    = $(ROOT_DIR)/src/$(COMPONENT_DIR)
BUILD_DIR     = $(ROOT_DIR)/build/$(COMPONENT_DIR)

SOURCES        := $(wildcard $(SOURCE_DIR)/filterprograms/*.c) $(wildcard $(SOURCE_DIR)/pcapsession/*.c) $(wildcard $(SOURCE_DIR)/gtp/*.c) $(wildcard $(SOURCE_DIR)/utilities/*.c)
COMMON_SOURCES := $(wildcard $(SOURCE_DIR)/pcapsession/*.c) $(wildcard $(SOURCE_DIR)/gtp/*.c) $(wildcard $(SOURCE_DIR)/utilities/*.c)
OBJECTS        := $(subst $(ROOT_DIR)/src, $(ROOT_DIR)/build, $(SOURCES:.c=.o))
COMMON_OBJECTS := $(subst $(ROOT_DIR)/src, $(ROOT_DIR)/build, $(COMMON_SOURCES:.c=.o))

# Variables for paths
INCLUDES += -I$(ROOT_DIR)/include

# Default config
CFLAGS = -g -O2 -fPIC -Wall -std=c99 -D_GNU_SOURCE
CXXFLAGS = -pthread

# Optimize
# CFLAGS = -g -O2
# Debug
# CFLAGS = -O0 -ggdb3
# CFLAGS += -Wall
# Profile
# CFLAGS = -pg
# Valgrind
# CFLAGS = -g -O1
# With Google Perftools' malloc library
# LDLIBS += -ltcmalloc

$(BUILD_DIR)/%.o : %.c
	quantify gcc -record-child-process-data=yes -write-export-file=quantifyhttptest -write-summary-file=quantifysummary $(CXXFLAGS) $(CFLAGS) $(INCLUDES) -MD -c $< -o $@
	sed 's/\.\.[\/\.]*\//$$(ROOT_DIR)\//g' $(@D)/$(*F).d > $(@D)/$(*F).d.tmp; \
	mv $(@D)/$(*F).d.tmp $(@D)/$(*F).d; \
	cp $(@D)/$(*F).d $(@D)/$(*F).P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' < $(@D)/$(*F).d >> $(@D)/$(*F).P; \
	rm -f $(@D)/$(*F).d

-include $(OBJECTS:%.o=%.P)

filter: build $(BIN_DIR) $(BIN_DIR)/pcapdistributer $(BIN_DIR)/pcapmerger $(BIN_DIR)/decodegtp

build: $(BUILD_DIR) $(BUILD_DIR)/filterprograms $(BUILD_DIR)/pcapsession $(BUILD_DIR)/gtp  $(BUILD_DIR)/utilities $(OBJECTS)

$(BIN_DIR)/pcapdistributer: $(COMMON_OBJECTS) $(BUILD_DIR)/filterprograms/pcapdistributer.o
	quantify gcc -record-child-process-data=yes -write-export-file=quantifyhttptest -write-summary-file=quantifysummary ${CXXFLAGS} ${CFLAGS} $(LDLIBS) $(COMMON_OBJECTS) $(BUILD_DIR)/filterprograms/pcapdistributer.o -o $@

$(BIN_DIR)/pcapmerger: $(COMMON_OBJECTS) $(BUILD_DIR)/filterprograms/pcapmerger.o
	quantify gcc -record-child-process-data=yes -write-export-file=quantifyhttptest -write-summary-file=quantifysummary ${CXXFLAGS} ${CFLAGS} $(LDLIBS) $(COMMON_OBJECTS) $(BUILD_DIR)/filterprograms/pcapmerger.o -o $@

$(BIN_DIR)/decodegtp: $(COMMON_OBJECTS) $(BUILD_DIR)/filterprograms/decodegtp.o
        quantify gcc -record-child-process-data=yes -write-export-file=quantifyhttptest -write-summary-file=quantifysummary ${CXXFLAGS} ${CFLAGS} $(LDLIBS) $(COMMON_OBJECTS) $(BUILD_DIR)/filterprograms/decodegtp.o -o $@

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
