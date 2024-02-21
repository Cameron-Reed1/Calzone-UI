include lv_dirs.mk

DEPS_DIR    := deps
LVGL_PATH   ?= $(DEPS_DIR)/lvgl
ARGS_DIR    := $(DEPS_DIR)/argparse
SRC_DIRS    := src src/fbd src/lvgl_port src/ui src/fonts $(LVGL_DIRS) $(ARGS_DIR)/Src
INC_DIRS    := inc inc/fbd inc/lvgl_port inc/ui $(DEPS_DIR) $(LVGL_PATH) $(ARGS_DIR)/Inc


BIN_NAME    := calzone_ui
BUILD_DIR   := build
OFILE_DIR   := $(BUILD_DIR)/objects
INSTALL_DIR ?= /usr/local/bin


INCLUDES    := $(addprefix -I, $(INC_DIRS))
C_SOURCES   := $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
CXX_SOURCES := $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.cpp))



OFILES      := $(addprefix $(OFILE_DIR)/, $(notdir $(C_SOURCES:.c=.o) $(CXX_SOURCES:.cpp=.o)))


OPT         := -O2
CPPFLAGS    := $(INCLUDES) -MMD
CFLAGS      := $(OPT) -Wall -Wextra -Wpedantic
CXXFLAGS    := $(OPT) -std=c++17 -Wall -Wextra -Wpedantic


LDFLAGS     := 
LDLIBS      := -pthread


DEPENDS     := $(OFILES:.o=.d)


DIRS        := $(BUILD_DIR) $(OFILE_DIR) $(PCH_DIR)


ifeq ($(strip $(CXX_SOURCES)),)
LD          := $(CC)
else
LD          := $(CXX)
endif


.PHONY: all run clean

all: $(BUILD_DIR)/$(BIN_NAME)

run: all
	@printf "[ EXEC ] $@\n"
	-@$(BUILD_DIR)/$(BIN_NAME)

install: all | $(INSTALL_DIR)
	@printf "[  CP  ] $(BUILD_DIR)/$(BIN_NAME) -> $(INSTALL_DIR)/$(BIN_NAME)\r"
	@cp $(BUILD_DIR)/$(BIN_NAME) $(INSTALL_DIR)/$(BIN_NAME)
	@printf "[  \e[32mCP\e[0m  ]\n"

clean:
	$(RM) -r $(DIRS)


-include $(DEPENDS)



$(BUILD_DIR)/$(BIN_NAME): $(OFILES) | $(BUILD_DIR)
	@printf "[  LD  ] $@\r"
	@$(LD) $^ $(LDFLAGS) $(LDLIBS) -o $@
	@printf "[  \e[32mLD\e[0m  ]\n"

$(OFILE_DIR)/%.o: %.c | $(OFILE_DIR)
	@printf "[  CC  ] $@\r"
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
	@printf "[  \e[32mCC\e[0m  ]\n"

$(OFILE_DIR)/%.o: %.cpp | $(OFILE_DIR)
	@printf "[  CXX ] $@\r"
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
	@printf "[  \e[32mCXX\e[0m ]\n"


$(DIRS) $(INSTALL_DIR):
	@mkdir -p $@


vpath %.c $(SRC_DIRS)
vpath %.cpp $(SRC_DIRS)
