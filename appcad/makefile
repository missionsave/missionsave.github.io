# --- Configurações do compilador e flags ---
CXX       := g++
# CXXFLAGS  :=  -Wall -Wextra -Werror
CXXFLAGS  := -std=c++20 -Iinclude -I../common -MMD -MP -DSCI_LEXER -DSCI_NAMESPACE  -D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS -D_WINDOWS -D_USRDLL

ifneq ($(dbg),)
	dbgd:=g
	# CXXFLAGS+= -g -O0 -Wall -Wextra -fsanitize=thread -fno-omit-frame-pointer
	# LDFLAGS+= -fsanitize=thread
	CXXFLAGS+= -g -O0 -Wall -Wextra -fsanitize=address,undefined -fno-omit-frame-pointer
	LDFLAGS+= -fsanitize=address,undefined
else
	CXXFLAGS+= -Os -w -Wfatal-errors -DNDEBUG -flto=auto  -O3 -march=native -mtune=native
	LDFLAGS+= -s -flto=auto
endif

# --- Diretórios e binário final ---
BUILD_DIR := .objw
LIBNAME   := libapp.a$(dbgd)
TARGET    := simplecad.exe


LDFLAGS+=  -Wl,--as-needed -Wl,--gc-sections


define common
CXXFLAGS+= -I../common
LDFLAGS+= -L. -l:$(LIBNAME)	
endef

ifeq ($(OS),Windows_NT)
    OS_NAME := Windows
else
    OS_NAME := $(shell uname -s)
	ifeq ($(shell uname -m),aarch64)
		OS_NAME := Arm
	endif
endif
ifeq ($(OS_NAME), Windows)
# $(eval $(common))
# $(info common: $(LDFLAGS))
	CXXFLAGS+= -I../common
	LDFLAGS+= -L. -l:$(LIBNAME)		

# 	LDFLAGS+= -L../common/scintilla_370 -l:libscint.a

	CXXFLAGS+= -I../common/scintilla_370/include -I../common/scintilla_370/fltk -I../common/scintilla_370/lexlib -I../common/scintilla_370/lexers -I../common/scintilla_370/src
	# LDFLAGS+=  -L../common/fltk-1.5.x-20250606-5293beea6f7b/bldw/lib 
	# CXXFLAGS+=  -I../common/fltk-1.5.x-20250606-5293beea6f7b/bldw -I../common/fltk-1.5.x-20250606-5293beea6f7b 
# 	LDFLAGS+= -l:libfltk_gl.a -l:libfltk.a    -l:libglew32.a -lopengl32 -lgdi32 -lglu32 -lWs2_32 -lwinspool  
	# LDFLAGS+=$(shell pkg-config  openscenegraph-osg openscenegraph-osgViewer --libs) -losgSim -losgManipulator
	# LDFLAGS+=$(shell pkg-config opencv4 --libs ) 
	
# 	LDFLAGS+= -ltbb12 -l:libtbbbind_2_5.dll.a  -l:libtbbmalloc.dll.a -l:libtbbmalloc_proxy.dll.a
	CXXFLAGS+= -I"C:\msys64\mingw64\include\opencascade"
	# dynamic
	LDFLAGS+= -lTKGeomBase -lTKMath -lTKTopAlgo -lTKBRep -lTKOpenGl -lTKernel -lTKService -lTKGeomBase -lTKMath -lTKTopAlgo -lTKBRep -lTKOpenGl -lTKService -lTKV3d -lTKBRep -lTKPrim -lTKTopAlgo -lTKGeomBase -lTKGeomAlgo  -lTKG3d -lTKV3d -lTKOpenGl -lTKService -lTKernel -lTKHLR -lTKBO -lTKBRep -lTKXSBase -lTKShHealing -lTKMesh
	#static
	# LDFLAGS+=  -Wl,-Bstatic
	# LDFLAGS+= -lTKViewerTest -lTKXDEDRAW -lTKTopTest -lTKExpress -lTKFeat -lTKBinXCAF -lTKBinTObj -lTKBinL -lTKBin -lTKTObjDRAW -lTKTObj -lTKVCAF -lTKLCAF -lTKDCAF -lTKCAF -lTKCDF -lTKXmlXCAF -lTKXmlTObj -lTKXmlL -lTKXml -lTKXSDRAWPLY -lTKXSDRAWOBJ -lTKXSDRAWGLTF -lTKXSDRAWVRML -lTKXSDRAWSTEP -lTKXSDRAWIGES -lTKXSDRAWDE -lTKXSDRAW -lTKQADraw -lTKDraw -lTKDEVRML -lTKDEGLTF -lTKDEPLY -lTKDECascade -lTKDEOBJ -lTKDEIGES -lTKDESTEP -lTKDE -lTKXMesh -lTKXCAF -lTKOpenGl -lTKV3d -lTKXSBase -lTKService -lTKOpenGlTest  -lTKRWMesh -lTKMeshVS -lTKMesh -lTKHLR -lTKOffset -lTKBO -lTKBool -lTKShHealing -lTKPrim -lTKTopAlgo -lTKBRep -lTKFillet -lTKGeomAlgo -lTKGeomBase -lTKG3d -lTKG2d -lTKMath -lTKStd -lTKernel
	# # LDFLAGS+=  -Wl,-Bdynamic

	# LDFLAGS += -Wl,-Bstatic

	# # Core
	# LDFLAGS += -lTKernel -lTKMath -lTKG2d -lTKG3d -lTKGeomBase -lTKGeomAlgo -lTKBRep \
	# 		-lTKTopAlgo -lTKPrim -lTKBool -lTKBO -lTKFillet -lTKOffset -lTKShHealing \
	# 		-lTKHLR -lTKMesh -lTKMeshVS -lTKRWMesh

	# # Services and Visualization
	# LDFLAGS += -lTKStd -lTKService -lTKXSBase -lTKV3d -lTKOpenGl -lTKOpenGlTest

	# # Data Exchange (DE*)
	# LDFLAGS += -lTKDE -lTKDECascade -lTKDEIGES -lTKDESTEP -lTKDEOBJ -lTKDEPLY -lTKDEGLTF -lTKDEVRML

	# # DRAW support
	# LDFLAGS += -lTKDraw -lTKQADraw -lTKXSDRAW -lTKXSDRAWDE -lTKXSDRAWIGES -lTKXSDRAWSTEP \
	# 		-lTKXSDRAWVRML -lTKXSDRAWGLTF -lTKXSDRAWOBJ -lTKXSDRAWPLY

	# # XML/Bin persistence
	# LDFLAGS += -lTKXml -lTKXmlL -lTKXmlTObj -lTKXmlXCAF -lTKCDF \
	# 		-lTKBin -lTKBinL -lTKBinTObj -lTKBinXCAF

	# # Application framework and extended attributes
	# LDFLAGS += -lTKCAF -lTKDCAF -lTKLCAF -lTKVCAF -lTKXCAF

	# # TObj framework
	# LDFLAGS += -lTKTObj -lTKTObjDRAW

	# # Features / Express / Testing
	# LDFLAGS += -lTKFeat -lTKExpress -lTKTopTest -lTKXDEDRAW -lTKViewerTest

	LDFLAGS += -Wl,-Bdynamic

	LDFLAGS+=  -lopengl32 -lgdi32 -lglu32 -lWs2_32 -lwinspool  
# 	LDFLAGS+= -ltbb12 -l:libtbbbind_2_5.dll.a  -l:libtbbmalloc.dll.a -l:libtbbmalloc_proxy.dll.a
# 	LDFLAGS+= -lfltk_gl -lfltk 
	LDFLAGS+= -l:libfltk_gl.a -l:libfltk.a


# 	LDFLAGS+= -l:libFreeImage.a
	# LDFLAGS+= -lFreeImage
# 	LDFLAGS+= -l:liblua.a

	# LDFLAGS+=  -l:libsqlite3.a
# 	LDFLAGS+=  -l:libiconv.a
	LDFLAGS+=  -l:libstdc++.a
	LDFLAGS+= -static-libgcc
	# LDFLAGS+= -lGL -lGLU
	LDFLAGS+= -lfontconfig
	LDFLAGS +=   -lole32 -luuid -lcomctl32 -l:libcomdlg32.a -lole32 -lgdiplus
	LDFLAGS+= -l:libwinpthread.a
endif
ifeq ($(OS_NAME), Linux)
	BUILD_DIR := .objl
	LIBNAME   := libappl.a$(dbgd)
	TARGET    := simplecad
	LDFLAGS+= -L. -l:$(LIBNAME)
	CXXFLAGS+= -I../common 
	
	
	
	# CXXFLAGS+= `fltk-config --cxxflags` -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_THREAD_SAFE -D_REENTRANT
	# LDFLAGS+= `fltk-config --ldflags` 
	# LDFLAGS+= -L../common/scintilla_370 -l:libscintl.a	
	# CXXFLAGS+= -I../common/scintilla_370/include -I../common/scintilla_370/fltk -I../common/scintilla_370/lexlib -I../common/scintilla_370/lexers -I../common/scintilla_370/src
	# LDFLAGS+=$(shell pkg-config  openscenegraph-osg openscenegraph-osgViewer --libs) -losgSim -losgManipulator
	# LDFLAGS+=$(shell pkg-config opencv4 --libs ) 
	

	# dynamic
	CXXFLAGS+= -I/usr/include/opencascade	
	LDFLAGS+= -lTKGeomBase -lTKMath -lTKTopAlgo -lTKBRep -lTKOpenGl -lTKernel -lTKService -lTKGeomBase -lTKMath -lTKTopAlgo -lTKBRep -lTKOpenGl -lTKService -lTKV3d -lTKBRep -lTKPrim -lTKTopAlgo -lTKGeomBase -lTKGeomAlgo  -lTKG3d -lTKV3d -lTKOpenGl -lTKService -lTKernel -lTKHLR -lTKBO -lTKBRep -lTKXSBase -lTKShHealing -lTKBRep -lTKMesh

	# static
	# LDFLAGS+= -L../common/opencascade_lin/lib	
	# CXXFLAGS += -I/home/super/missionsave/common/opencascade_lin/include
	# LDFLAGS+= -lTKViewerTest -lTKXDEDRAW -lTKTopTest -lTKExpress -lTKFeat -lTKBinXCAF -lTKBinTObj -lTKBinL -lTKBin -lTKTObjDRAW -lTKTObj -lTKVCAF -lTKLCAF -lTKDCAF -lTKCAF -lTKCDF -lTKXmlXCAF -lTKXmlTObj -lTKXmlL -lTKXml -lTKXSDRAWPLY -lTKXSDRAWOBJ -lTKXSDRAWGLTF -lTKXSDRAWVRML -lTKXSDRAWSTEP -lTKXSDRAWIGES -lTKXSDRAWDE -lTKXSDRAW -lTKQADraw -lTKDraw -lTKDEVRML -lTKDEGLTF -lTKDEPLY -lTKDECascade -lTKDEOBJ -lTKDEIGES -lTKDESTEP -lTKDE -lTKXMesh -lTKXCAF -lTKOpenGl -lTKV3d -lTKXSBase -lTKService -lTKOpenGlTest  -lTKRWMesh -lTKMeshVS -lTKMesh -lTKHLR -lTKOffset -lTKBO -lTKBool -lTKShHealing -lTKPrim -lTKTopAlgo -lTKBRep -lTKFillet -lTKGeomAlgo -lTKGeomBase -lTKG3d -lTKG2d -lTKMath -lTKStd -lTKernel


	# LDFLAGS+= -llua5.4
	# LDFLAGS+=  -lsqlite3 
	LDFLAGS+= -lfltk_gl -lfltk 
	# LDFLAGS+= -l:libfltk_gl.a -l:libfltk.a 
	# LDFLAGS+= -l:libfltk_gl.so -l:libfltk.so 
	# LDFLAGS += -lwayland-client -lwayland-cursor -lwayland-egl


	# LDFLAGS+= -lepoxy
	# LDFLAGS+= -ltcl -ltk

# 	LDFLAGS+= -ltbb

	#static fltk:
	LDFLAGS+= -lX11 -lXext -lGL -lGLU -lXft -lXrender -lXcursor -lXinerama -lXfixes -lfontconfig -lfreetype -lz -lm -ldl -lpthread -lstdc++
	# LDFLAGS+=  -lX11
	# LDFLAGS+= -lGL -lGLU
	# LDFLAGS+= -lfontconfig
	# LDFLAGS+= -lz -lpthread -ldl -lm -lrt -lstdc++
	# LDFLAGS+= -lfreetype
	# LDFLAGS+=  -l:libstdc++.a

endif
ifeq ($(OS_NAME), Arm)
	BUILD_DIR := .obja
	LIBNAME   := libappa.a$(dbgd)
	TARGET    := simplecad
	LDFLAGS+= -L. -l:$(LIBNAME)

	LDFLAGS+= -lTKGeomBase -lTKMath -lTKTopAlgo -lTKBRep -lTKOpenGl -lTKernel -lTKService -lTKGeomBase -lTKMath -lTKTopAlgo -lTKBRep -lTKOpenGl -lTKService -lTKV3d -lTKBRep -lTKPrim -lTKTopAlgo -lTKGeomBase -lTKGeomAlgo  -lTKG3d -lTKV3d -lTKOpenGl -lTKService -lTKernel -lTKHLR -lTKBO -lTKBRep -lTKXSBase -lTKShHealing


	CXXFLAGS+= -O2 -march=armv8-a -mtune=cortex-a53
	CXXFLAGS+= -I../common 
# 	LDFLAGS+= -L../common/scintilla_370 -l:libscintla.a	
# 	CXXFLAGS+= -I../common/scintilla_370/include -I../common/scintilla_370/fltk -I../common/scintilla_370/lexlib -I../common/scintilla_370/lexers -I../common/scintilla_370/src

# 	LDFLAGS+=$(shell pkg-config  openscenegraph-osg openscenegraph-osgViewer --libs) -losgSim -losgManipulator
	# LDFLAGS+=$(shell pkg-config opencv4 --libs ) 	
# 	LDFLAGS+= -llua5.4
	# LDFLAGS+=  -lsqlite3 
	LDFLAGS+= -lfltk_gl -lfltk 
	# LDFLAGS+= -l:libfltk_gl.a -l:libfltk.a 
	LDFLAGS+=  -lX11
	LDFLAGS+= -lGL -lGLU
	LDFLAGS+= -lz -lpthread -ldl -lm -lrt -lstdc++
	
	CXXFLAGS+= -I/usr/include/opencascade 
endif

# lib files
SRC_LIB :=  \
    ../common/general.cpp

# exec
SRC_APP := \
    simplecad.cpp

ifneq ($(app),)
    TARGET := $(app)
endif
$(info app name: $(TARGET))
# Check if src is not empty
ifneq ($(src),)
    SRC_APP := $(src)
    $(info Source files: $(SRC_APP))
endif


# --- Macro para gerar nome do objeto achatado na build/ ---
define OBJ_NAME
$(BUILD_DIR)/$(subst /,_,$(basename $(1))).o$(dbgd)
endef

# --- Objs of lib ---
OBJS_LIB := $(foreach src,$(SRC_LIB),$(call OBJ_NAME,$(src)))

# --- Objs of app ---
OBJS_APP := $(foreach src,$(SRC_APP),$(call OBJ_NAME,$(src)))

# --- Dependencies ---
DEPS_LIB := $(OBJS_LIB:.o$(dbgd)=.d)
DEPS_APP := $(OBJS_APP:.o$(dbgd)=.d)


all: $(TARGET)

# --- Biblioteca estática ---
$(LIBNAME): $(OBJS_LIB)
	ar rcs $@ $^

# --- Executável linkando objetos app + lib ---
$(TARGET): $(OBJS_APP) $(LIBNAME)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS_APP)  $(LDFLAGS) 
	@if [ "$(nr)" != "1" ]; then \
		sleep 0.2; \
		./$(TARGET); \
	fi

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
