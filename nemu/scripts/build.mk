YELLOW := $(shell printf "\033[33m")
NONE := $(shell printf "\033[0m")

$(info $(YELLOW)file: NEMU/build is used$(NONE))

.DEFAULT_GOAL = app

# Add necessary options if the target is a shared library
# 判断是不是要编译成为动态链接库
ifeq ($(SHARE),1)
SO = -so
CFLAGS  += -fPIC -fvisibility=hidden
LDFLAGS += -shared -fPIC
endif

WORK_DIR  = $(shell pwd)
$(info $(YELLOW)build.mk                                    WORK_DIR is $(WORK_DIR)$(NONE))
BUILD_DIR = $(WORK_DIR)/build
$(info $(YELLOW)build.mk                                    BUILD_DIR is $(BUILD_DIR)$(NONE))

INC_PATH := $(WORK_DIR)/include $(INC_PATH)
$(info $(YELLOW)build.mk                                    BUILD_DIR is $(INC_PATH)$(NONE))

OBJ_DIR  = $(BUILD_DIR)/obj-$(NAME)$(SO)
$(info $(YELLOW)build.mk                                    OBJ_DIR is $(OBJ_DIR)$(NONE))

BINARY   = $(BUILD_DIR)/$(NAME)$(SO)
$(info $(YELLOW)build.mk                                    BIINARY is $(BINARY)$(NONE))

# Compilation flags
ifeq ($(CC),clang)
CXX := clang++
else
CXX := g++
endif
LD := $(CXX)
INCLUDES = $(addprefix -I, $(INC_PATH))
CFLAGS  := -O2 -MMD -Wall -Werror $(INCLUDES) $(CFLAGS)
LDFLAGS := -O2 $(LDFLAGS)

OBJS = $(SRCS:%.c=$(OBJ_DIR)/%.o) $(CXXSRC:%.cc=$(OBJ_DIR)/%.o)

# Compilation patterns
$(OBJ_DIR)/%.o: %.c
	@echo + CC $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c -o $@ $<
	$(call call_fixdep, $(@:.o=.d), $@)

$(OBJ_DIR)/%.o: %.cc
	@echo + CXX $<
	@mkdir -p $(dir $@)
	@$(CXX) $(CFLAGS) $(CXXFLAGS) -c -o $@ $<
	$(call call_fixdep, $(@:.o=.d), $@)

# Depencies
-include $(OBJS:.o=.d)

# Some convenient rules

.PHONY: app clean

app: $(BINARY)

$(BINARY):: $(OBJS) $(ARCHIVES)
	@echo + LD $@
	@$(LD) -o $@ $(OBJS) $(LDFLAGS) $(ARCHIVES) $(LIBS)
	$($(YELLOW)info the command Binary(build.mk) is used$(NONE))

clean:
	-rm -rf $(BUILD_DIR)
