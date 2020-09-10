## This is a project made within Seoul National University of Science and Technology
## Embedded Systems Laboratory 2018 - Raimarius Tolentino Delgado

#######################################################################################################
CUR_DIR = ./

# json parser 
CONFIGS_PATH=$(RT_AIDE_ROOT_PATH)/configs
CONFIGS_FULL_PATH=$(CONFIGS_PATH)/config.json
define GetFromJson
$(shell node -p "require('$(CONFIGS_FULL_PATH)').$(1)")
endef

INC_EMBD = $(CUR_DIR)/libs/embedded
INC_DIRS = -I$(CUR_DIR) -I$(INC_EMBD) 

CFLAGS_OPTIONS = -Wall -O3 -mtune=native -flto
CFLAGS   = $(CFLAGS_OPTIONS) $(INC_DIRS)
LDFLAGS	 = -lm

RT_DOMAIN = $(call GetFromJson, rt_linux)

ifeq ($(RT_DOMAIN),xenomai)
XENOMAI_SKIN = $(call GetFromJson, rt_skin) 
XENOMAI_PATH?=/usr/xenomai
ifeq ($(XENOMAI_SKIN),posix)
INC_XENO = $(shell $(XENOMAI_PATH)/bin/xeno-config --skin posix --cflags) 
LIB_XENO = $(shell $(XENOMAI_PATH)/bin/xeno-config --skin posix --ldflags) 
else # alchemy as default
INC_XENO = $(shell $(XENOMAI_PATH)/bin/xeno-config --skin alchemy --cflags) 
LIB_XENO = $(shell $(XENOMAI_PATH)/bin/xeno-config --skin aclhemy --ldflags)
CFLAGS += -D__XENOMAI_NATIVE__ 
endif
CFLAGS += $(INC_XENO) 
LDFLAGS += $(LIB_XENO) 
else
# CFLAGS   += $(CFLAGS_OPTIONS) $(INC_DIRS) $(INC_XENO)
LDFLAGS	 += -lm -lrt -lpthread
endif

SOURCES	+= main.c 			
SOURCES	+= $(INC_EMBD)/src/rt_tasks.c
SOURCES	+= $(INC_EMBD)/src/rt_itc.c
ifneq ($(RT_DOMAIN),xenomai)
SOURCES	+= $(INC_EMBD)/src/rt_posix_task.c
SOURCES	+= $(INC_EMBD)/src/rt_posix_mutex.c
endif

OBJ_DIR = obj
OUT_DIR = bin
EXEC_TARGET	= test_perf
START	= start

ifeq ($(wildcard main.cpp),)
CC = $(CROSS_COMPILE)gcc
else
CC = $(CROSS_COMPILE)g++
endif

LD = $(CROSS_COMPILE)ld
AS = $(CROSS_COMPILE)as

CHMOD	= /bin/chmod
MKDIR	= /bin/mkdir
ECHO	= echo
RM	= /bin/rm
#######################################################################################################
OBJECTS = $(addprefix $(OBJ_DIR)/, $(notdir $(patsubst %.c, %.o, $(patsubst %.cpp, %.o, $(SOURCES)))))
#######################################################################################################
vpath %.c  $(CUR_DIR) $(INC_SERVO) $(INC_EMBD)/src
vpath %.cpp $(CUR_DIR) $(INC_SERVO) $(INC_EMBD)/src
#######################################################################################################

ifeq ($(wildcard $(START).sh),)
all: 	$(OUT_DIR)/$(EXEC_TARGET) $(START)
	@$(ECHO) BUILD DONE.
	@$(CHMOD) +x $(START).sh
else
all: 
	@$(ECHO) BUILD ERROR: Run make clean first!
endif

$(START): 
	@printf "## This is a project made within Seoul National University of Science and Technology \n" > $(START).sh
	@printf "## Embedded Systems Laboratory 2018 - Raimarius Tolentino Delgado \n\n" >> $(START).sh
	@printf "## Start-up for dynamically linked executable file \n\n\n\n" >> $(START).sh
ifeq ($(RT_DOMAIN),xenomai)
	@printf "export LD_LIBRARY_PATH=$(XENOMAI_PATH)/lib \n" >> $(START).sh
endif
	@printf "./$(OUT_DIR)/$(EXEC_TARGET) \$$1 \$$2 \$$3 \$$4 \$$5 \$$6\n" >> $(START).sh

$(OUT_DIR)/$(EXEC_TARGET): $(OBJECTS)
	@$(MKDIR) -p $(OUT_DIR); pwd > /dev/null
	$(CC) $(CFLAGS) -o $(OUT_DIR)/$(EXEC_TARGET) $(OBJECTS) $(LDFLAGS)

$(OBJ_DIR)/%.o : %.cpp
	@$(MKDIR) -p $(OBJ_DIR); pwd > /dev/null
	$(CC) -MD $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o : %.c
	@$(MKDIR) -p $(OBJ_DIR); pwd > /dev/null
	$(CC) -MD $(CFLAGS) -c -o $@ $<

clean:
	$(RM) -rf \
		$(OBJ_DIR)/* \
		$(OBJ_DIR)   \
			$(OUT_DIR)/* \
			$(OUT_DIR)   \
			*.dat	     \
		$(START)*
re:
	@touch ./* $(INC_EMBD)/src/* 
	make clean
	make 

test:
	@echo $(RT_DOMAIN)


.PHONY: all clean 
#######################################################################################################
# Include header file dependencies generated by -MD option:
-include $(OBJ_DIR_CUR)/*.d


