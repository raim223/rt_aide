## This is a project made within Seoul National University of Science and Technology
## Embedded Systems Laboratory 2018 - Raimarius Tolentino Delgado

#######################################################################################################
CUR_DIR = .

INC_EMBD = $(CUR_DIR)/libs/embedded
INC_DIRS = -I$(CUR_DIR) -I$(INC_EMBD) 

CFLAGS_OPTIONS = -Wall -O3 -mtune=native -flto
CFLAGS   = $(CFLAGS_OPTIONS) $(INC_DIRS)
LDFLAGS	 = $(LIB_EMBD) -lm

XENOMAI_PATH?=/usr/xenomai
INC_XENO_NATIVE = $(shell $(XENOMAI_PATH)/bin/xeno-config --skin alchemy --cflags) 
LIB_XENO_NATIVE = $(shell $(XENOMAI_PATH)/bin/xeno-config --skin alchemy --ldflags --no-auto-init)
INC_XENO_POSIX = $(shell $(XENOMAI_PATH)/bin/xeno-config --skin posix --cflags) 
LIB_XENO_POSIX = $(shell $(XENOMAI_PATH)/bin/xeno-config --skin posix --ldflags --no-auto-init)


LDFLAGS	 += -lm -lrt -lpthread

SOURCES	+= $(INC_EMBD)/src/rt_tasks.c
SOURCES	+= $(INC_EMBD)/src/rt_posix_task.c

OBJ_DIR = obj
OUT_DIR = lib_out

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
AS = $(CROSS_COMPILE)as

CHMOD	= /bin/chmod
MKDIR	= /bin/mkdir
ECHO	= echo
RM	= /bin/rm
#######################################################################################################
OBJECTS = $(addprefix $(OBJ_DIR)/, $(notdir $(patsubst %.c, %.o, $(patsubst %.cpp, %.o, $(SOURCES)))))
#######################################################################################################
vpath %.c  $(CUR_DIR)/ $(INC_EMBD)/src
#######################################################################################################

all: library_posix


library_posix: $(OUT_DIR)/librtaide_xeno.so

# rtpreempt: $(INC_EMBD)/librtaide.so

# rtpreempt: $(INC_EMBD)/librtaide.so

$(OUT_DIR)/librtaide_xeno_posix.so: objects_posix
	@$(MKDIR) -p $(OUT_DIR); pwd > /dev/null
	$(CC) -shared $(CFLAGS) $(INC_XENO_NATIVE) -o $@ -fPIC $(OBJECTS) $(LDFLAGS)
objects_posix: prepare_posix $(OBJECTS)
prepare_posix: reset
	$(eval CFLAGS=$(filter-out $(INC_XENO_NATIVE) -D__XENOMAI_NATIVE__,$(CFLAGS)))
	$(eval LDFLAGS=$(filter-out $(LIB_XENO_NATIVE) -D__XENOMAI_NATIVE__,$(CFLAGS)))
	$(eval CFLAGS+=$(INC_XENO_POSIX))
	$(eval CFLAGS+=$(INC_XENO_POSIX))

prepare_for_native: reset


$(OBJ_DIR)/%.o : %.c
	@$(MKDIR) -p $(OBJ_DIR); pwd > /dev/null
	$(CC) -MD $(CFLAGS) -c -o $@ $<

test_cflags:	
	$(eval CFLAGS+=$(INC_XENO_NATIVE))
	@echo $(CFLAGS) 
	$(eval CFLAGS=$(filter-out $(INC_XENO_NATIVE),$(CFLAGS)))
	@echo $(CFLAGS)

reset:
	$(RM) -rf \
		$(OBJ_DIR)/* \
		$(OBJ_DIR)   	

clean:
	$(RM) -rf \
		$(OBJ_DIR)/* \
		$(OBJ_DIR)   \
		$(OUT_DIR)/librtaide_xeno.so

re:
	@touch ./* $(INC_EMBD)/src/* 
	make clean
	make 

.PHONY: all clean 
#######################################################################################################
# Include header file dependencies generated by -MD option:
-include $(OBJ_DIR_CUR)/*.d


