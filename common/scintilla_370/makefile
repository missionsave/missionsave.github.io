# --- Configurações do compilador e flags ---
CXX       := g++
# CXXFLAGS  :=  -Wall -Wextra -Werror
CXXFLAGS  := -std=c++20 -Os -w -Wfatal-errors -Iinclude -I.. -MMD -MP -DSCI_LEXER -DSCI_NAMESPACE  -D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS -D_WINDOWS -D_USRDLL

# --- Diretórios e binário final ---
BUILD_DIR := .objw
LIBNAME   := libscint.a
TARGET    := fltest.exe

LDFLAGS+=  -Wl,--as-needed -Wl,--gc-sections
LDFLAGS+= -s

ifeq ($(OS),Windows_NT)
    OS_NAME := Windows
else
    OS_NAME := $(shell uname -s)
	ifeq ($(shell uname -m),aarch64)
		OS_NAME := Arm
	endif
endif
ifeq ($(OS_NAME), Windows)
	CXXFLAGS+= -DWIN32 
	LDFLAGS+= -L. -l:$(LIBNAME)
	# LDFLAGS+=  -L../fltk-1.5.x-20250606-5293beea6f7b/bldw/lib
	# CXXFLAGS+=  -I../fltk-1.5.x-20250606-5293beea6f7b/bldw -I../fltk-1.5.x-20250606-5293beea6f7b 
	LDFLAGS+= -l:libfltk_gl.a -l:libfltk.a    -l:libglew32.a -lopengl32 -lgdi32 -lglu32 -lWs2_32  -lwinspool 
	# LDFLAGS+=  -L/desk/missionsave/fl_scint_lib/libiconv1.14win/lib -l:libiconv.a
	LDFLAGS+=  -l:libiconv.a
	LDFLAGS+=  -l:libstdc++.a
	LDFLAGS+= -static-libgcc
	LDFLAGS+= -l:libwinpthread.a
	LDFLAGS +=   -lole32 -luuid -lcomctl32 -l:libcomdlg32.a -lole32 -lgdi32
endif
ifeq ($(OS_NAME), Linux)
	BUILD_DIR := .objl
	LIBNAME   := libscintl.a
	TARGET    := fltest
	LDFLAGS+= -L. -l:$(LIBNAME)
	LDFLAGS+= -lfltk_gl -lfltk 
	# LDFLAGS+= -l:libfltk_gl.a -l:libfltk.a 
	LDFLAGS+=  -lX11
	# LDFLAGS+= -lpthread
	# LDFLAGS+= -lfontconfig
	# LDFLAGS+= -lfreetype
	# LDFLAGS+=  -l:libstdc++.a

endif
ifeq ($(OS_NAME), Arm)
	BUILD_DIR := .objla
	LIBNAME   := libscintla.a
	TARGET    := fltesta
	CXXFLAGS+= -O2 -march=armv8-a -mtune=cortex-a53
	LDFLAGS+= -L. -l:$(LIBNAME)
	LDFLAGS+= -lfltk_gl -lfltk 
	# LDFLAGS+= -l:libfltk_gl.a -l:libfltk.a 
	LDFLAGS+=  -lX11
	# LDFLAGS+= -lpthread
	# LDFLAGS+= -lfontconfig
	# LDFLAGS+= -lfreetype
	# LDFLAGS+=  -l:libstdc++.a

endif



# --- Fontes da biblioteca ---
SRC_LIB := 
SRC_LIB+= $(wildcard fltk/*.cxx)
SRC_LIB+= $(wildcard lexlib/*.cxx)
SRC_LIB+= $(wildcard lexers/*.cxx)
SRC_LIB+= $(wildcard src/*.cxx)
CXXFLAGS+= -Isrc
CXXFLAGS+= -Ilexlib
CXXFLAGS+= -Ilexers
CXXFLAGS+= -Ifltk

# --- Fontes do executável ---
SRC_APP := \
    fltest.cpp 

# --- Macro para gerar nome do objeto achatado na build/ ---
define OBJ_NAME
$(BUILD_DIR)/$(subst /,_,$(basename $(1))).o
endef

# --- Objetos da lib ---
OBJS_LIB := $(foreach src,$(SRC_LIB),$(call OBJ_NAME,$(src)))

# --- Objetos do app ---
OBJS_APP := $(foreach src,$(SRC_APP),$(call OBJ_NAME,$(src)))

# --- Dependências ---
DEPS_LIB := $(OBJS_LIB:.o=.d)
DEPS_APP := $(OBJS_APP:.o=.d)

# --- Alvo padrão ---
all: $(TARGET)

# --- Biblioteca estática ---
$(LIBNAME): $(OBJS_LIB)
	ar rcs $@ $^

# --- Executável linkando objetos app + lib ---
$(TARGET): $(OBJS_APP) $(LIBNAME)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS_APP)  $(LDFLAGS)  

# --- Regras para compilar fontes da lib ---
define MAKE_RULE_LIB
$(call OBJ_NAME,$(1)): $(1)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $$< -o $$@
endef

$(foreach src,$(SRC_LIB),$(eval $(call MAKE_RULE_LIB,$(src))))

# --- Regras para compilar fontes do app ---
define MAKE_RULE_APP
$(call OBJ_NAME,$(1)): $(1)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $$< -o $$@
endef

$(foreach src,$(SRC_APP),$(eval $(call MAKE_RULE_APP,$(src))))

# --- Limpeza ---
clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(LIBNAME)

# --- Incluir dependências ---
-include $(DEPS_LIB)
-include $(DEPS_APP)

.PHONY: all clean
