# Build the host-side compiler drivers; application ABI flags live in driver.cpp.
.DEFAULT_GOAL := all

ifeq ($(OS),Windows_NT)
PLATFORM ?= windows
else
PLATFORM ?= linux
endif

CXX ?= g++
CXXFLAGS ?= -O2 -Wall -Wextra -Wpedantic
PYTHON ?= python3
OUTPUT_DIR ?= release/$(PLATFORM)
BUILD_DIR := $(OUTPUT_DIR)/.build

ifeq ($(PLATFORM),windows)
EXE := .exe
HOST_CPPFLAGS := -DXXCC_HOST_WINDOWS
LDLIBS += -lshell32
else ifeq ($(PLATFORM),xj380)
EXE := .epf
else ifeq ($(PLATFORM),linux)
HOST_CPPFLAGS := -DXXCC_HOST_LINUX
else
$(error Unknown PLATFORM '$(PLATFORM)'; use linux, windows, or xj380)
endif

NAMES := xxcc xxcc-- xxcc-elf xxcc-elf-- xxccte xxccte-- xxccte-elf xxccte-elf--
TARGETS := $(addprefix $(OUTPUT_DIR)/,$(addsuffix $(EXE),$(NAMES)))
COMMON := $(BUILD_DIR)/driver.o $(BUILD_DIR)/platform.o

.PHONY: all linux win xj380 test clean
all: $(TARGETS)

# These aliases select the layout. CXX must target the selected host platform.
linux:
	$(MAKE) PLATFORM=linux all
win:
	$(MAKE) PLATFORM=windows all
xj380:
	$(MAKE) PLATFORM=xj380 all

$(OUTPUT_DIR) $(BUILD_DIR):
ifeq ($(OS),Windows_NT)
ifeq (,$(findstring sh,$(notdir $(SHELL))))
	if not exist "$(subst /,\,$@)" mkdir "$(subst /,\,$@)"
else
	mkdir -p "$@"
endif
else
	mkdir -p "$@"
endif

$(BUILD_DIR)/driver.o: src/driver.cpp src/include/driver.h src/include/platform.h | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(HOST_CPPFLAGS) $(CXXFLAGS) -std=c++17 -MMD -MP -c "$<" -o "$@"

$(BUILD_DIR)/platform.o: src/platform.cpp src/include/platform.h | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(HOST_CPPFLAGS) $(CXXFLAGS) -std=c++17 -MMD -MP -c "$<" -o "$@"

define variant
$(BUILD_DIR)/$(1).o: src/main.cpp src/include/driver.h src/include/platform.h | $(BUILD_DIR)
	$$(CXX) $$(CPPFLAGS) $$(HOST_CPPFLAGS) $$(CXXFLAGS) -std=c++17 -MMD -MP \
		$$(if $$(findstring xxccte,$(1)),-DXXCC_TUI,-DXXCC_GUI) \
		$$(if $$(findstring -elf,$(1)),-DXXCC_ELF,-DXXCC_EPF) \
		$$(if $$(findstring --,$(1)),-DXXCC_C,-DXXCC_CPP) -c "$$<" -o "$$@"

$(OUTPUT_DIR)/$(1)$(EXE): $(BUILD_DIR)/$(1).o $(COMMON) | $(OUTPUT_DIR)
	$$(CXX) $$(LDFLAGS) $$^ $$(LDLIBS) -o "$$@"
endef
$(foreach name,$(NAMES),$(eval $(call variant,$(name))))

test: all
	$(PYTHON) tests/test_driver.py --bin-dir "$(abspath $(OUTPUT_DIR))" --suffix "$(EXE)" -v

clean:
ifeq ($(OS),Windows_NT)
ifeq (,$(findstring sh,$(notdir $(SHELL))))
	-if exist "$(subst /,\,$(BUILD_DIR))" rmdir /s /q "$(subst /,\,$(BUILD_DIR))"
	-del /q $(subst /,\,$(TARGETS))
else
	rm -rf -- "$(BUILD_DIR)" $(TARGETS)
endif
else
	rm -rf -- "$(BUILD_DIR)" $(TARGETS)
endif

-include $(COMMON:.o=.d) $(addprefix $(BUILD_DIR)/,$(addsuffix .d,$(NAMES)))
