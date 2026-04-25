
MAKEFLAGS += -j12

CXX := em++
AR := emar

SRCF := src
TEMP := obj
TARGET := dist

SRC := $(wildcard $(SRCF)/*.cpp)
OBJ := $(patsubst $(SRCF)/%.cpp,$(TEMP)/%.o,$(SRC))
DEP := $(patsubst $(SRCF)/%.cpp,$(TEMP)/%.d,$(SRC))

OUT := libsimpleomp

# https://emscripten.org/docs/tools_reference/emcc.html
# https://github.com/emscripten-core/emscripten/blob/main/src/settings.js

CPPFLAGS := -I$(SRCF) -Iinclude -std=c++20 -DNCNN_SIMPLEOMP=1
CXXFLAGS := -Oz -flto -fopenmp -pthread -sDISABLE_EXCEPTION_CATCHING=1

SLIB := $(TARGET)/$(OUT).a

ifeq ($(OS),Windows_NT)
# Error is still possible in parallel running, so we add extra protection
	MK = -@if not exist "$(@D)" mkdir "$(@D)" 2> NUL
	RM = rmdir /s /q
else
	MK = @mkdir -p "$(@D)"
endif

.PHONY: all
all: $(SLIB)

$(SLIB): $(OBJ) makefile
	$(MK)
	@echo Compiling [33m$(SLIB)[0m
	@$(AR) $(USRFLAGS) rcs $@ $(OBJ)
	@echo [33mLibrary compile complete![0m

$(TEMP)/%.o: $(SRCF)/%.cpp
	$(MK)
	@echo Compiling [32m$(patsubst $(SRCF)/%,%,$<)[0m
	@$(COMPILE.cc) -MMD -c $< -o $@

# Ignoring old dependencies that were removed
%.h: ;
%.hpp: ;
%.d: ;

-include $(DEP)

.PHONY: clean
clean:
	@$(RM) "$(TEMP)"

.PHONY: with-examples
with-examples: all
	@echo [33mBuilding examples...[0m
	@$(MAKE) -C example --no-print-directory

.PHONY: with-test
with-test: all
	@$(MAKE) -C test --no-print-directory

.PHONY: serve
serve:
	@pnpx statikk --coi example