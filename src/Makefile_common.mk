# Variables for paths
INCLUDES += -I$(ROOT_DIR)/include

# Default config
CFLAGS = -g -O2 -fPIC -Wall -std=c99 -D_GNU_SOURCE
CPPFLAGS = -g -O2 -fPIC -Wall -D_GNU_SOURCE
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
	gcc $(CXXFLAGS) $(CFLAGS) $(INCLUDES) -MD -c $< -o $@
	sed 's/\.\.[\/\.]*\//$$(ROOT_DIR)\//g' $(@D)/$(*F).d > $(@D)/$(*F).d.tmp; \
	mv $(@D)/$(*F).d.tmp $(@D)/$(*F).d; \
	cp $(@D)/$(*F).d $(@D)/$(*F).P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' < $(@D)/$(*F).d >> $(@D)/$(*F).P; \
	rm -f $(@D)/$(*F).d

$(BUILD_DIR)/%.o : %.cpp
	g++ $(CXXFLAGS) $(CPPFLAGS) $(INCLUDES) -MD -c $< -o $@
	sed 's/\.\.[\/\.]*\//$$(ROOT_DIR)\//g' $(@D)/$(*F).d > $(@D)/$(*F).d.tmp; \
	mv $(@D)/$(*F).d.tmp $(@D)/$(*F).d; \
	cp $(@D)/$(*F).d $(@D)/$(*F).P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' < $(@D)/$(*F).d >> $(@D)/$(*F).P; \
	rm -f $(@D)/$(*F).d

-include $(OBJECTS:%.o=%.P)
