#!/usr/bin/env bash

# --- Colors ---
RED="\e[31m"
GREEN="\e[32m"
YELLOW="\e[33m"
RESET="\e[0m"

# --- Example functions ---
my_task() {
    echo -e "${GREEN}Running mytask with args:${RESET} $*"
}

_othertask() {
    echo -e "${GREEN}Running othertask with args:${RESET} $*"
}





dirsize(){ 
  echo "📁 Subdirectory Sizes:"; 
  find . -mindepth 1 -maxdepth 1 -type d -print0 | du -sh --files0-from=-; 
  echo "------"; 
  echo "📄 Current Directory Files Total:"; 
  find . -mindepth 1 -maxdepth 1 -type f -print0 | du -ch --files0-from=- | grep total$ | sed 's/total$/current dir/'; 
  echo "------"; 
  echo "📦 Total Size (Files + Subdirs):"; 
  du -sh . | sed 's/\t.*/\ttotal/'; 
}


_compile_cad_scint(){
	em++ testscint.cpp /home/super/msv/common/general.cpp  -o app.mjs   -O2    -I/home/super/msv/emsdk/downloads/fltk_wasm   -I/home/super/msv/emsdk/downloads/fltk_wasm/build_wasm   /home/super/msv/emsdk/downloads/fltk_wasm/build_wasm/lib/libfltk.a -I/home/super/msv/common  --bind -sJSPI=1 -sMODULARIZE=1 --pre-js pre.js


	# em++ testscint.cpp /home/super/msv/common/general.cpp  -o app.mjs   -O2    -I/home/super/msv/emsdk/downloads/fltk_wasm   -I/home/super/msv/emsdk/downloads/fltk_wasm/build_wasm   /home/super/msv/emsdk/downloads/fltk_wasm/build_wasm/lib/libfltk.a -I/home/super/msv/common  --bind -sJSPI=1 -sJSPI_EXPORTS=[...] -sEXPORTED_FUNCTIONS=_main -sEXPORTED_RUNTIME_METHODS=ccall -sMODULARIZE=1 --pre-js pre.js
	
	
# 	em++ testscint.cpp /home/super/msv/common/general.cpp /home/super/msv/common/fl_scintilla.cpp /home/super/msv/common/scintilla_370/scintilla_fltk_amalgamated.cpp /home/super/msv/common/fl_browser_msv.cpp -o app.mjs   -O2 -L/home/super/msv/common/scintilla_370 -l:libscinte.a  -I/home/super/msv/emsdk/downloads/fltk_wasm   -I/home/super/msv/emsdk/downloads/fltk_wasm/build_wasm   /home/super/msv/emsdk/downloads/fltk_wasm/build_wasm/lib/libfltk.a -I/home/super/msv/common -I/home/super/msv/common/sol2/include  -I/home/super/msv/common/scintilla_370 -L/home/super/msv/emsdk/downloads/lua-5.5.1-wasm/lib -llua -I/home/super/msv/emsdk/downloads/lua-5.5.1-wasm/include -I/home/super/msv/common/scintilla_370/include/ -I/home/super/msv/common/scintilla_370/lexlib -I/home/super/msv/common/scintilla_370/src -DSCI_LEXER -DSCI_NAMESPACE  -D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS --bind -sASYNCIFY=2 \
#   -sJSPI_EXPORTS=['fl_run_jspi','main','_main',...] \
#   -sEXPORTED_FUNCTIONS=_main,_fl_run_jspi \
#   -sEXPORTED_RUNTIME_METHODS=ccall,cwrap \
#   -sMODULARIZE=1 -sEXPORT_ES6=1 -sEXPORT_NAME=createApp \
#   -matomics -mbulk-memory -fwasm-exceptions -sSUPPORT_LONGJMP=wasm -pthread -s PTHREAD_POOL_SIZE_STRICT=0 -sPROXY_TO_PTHREAD=0
	
	
	
	
	#    -s MODULARIZE=1   -sSTACK_SIZE=5MB -sINITIAL_MEMORY=64MB -sGROWABLE_ARRAYBUFFERS=2  -sMAXIMUM_MEMORY=512MB -pthread -std=c++20 -fwasm-exceptions -sSUPPORT_LONGJMP=wasm -s EXPORTED_RUNTIME_METHODS=ccall -sASYNCIFY=2  -sJSPI_EXPORTS=[...]
	
	
	# -s JSPI=1 -s EXPORTED_RUNTIME_METHODS=ccall -s JSPI_EXPORTS=_fl_run_jspi,__wasm_call_ctors -s EXPORTED_FUNCTIONS=_fl_run_jspi  -s OFFSCREENCANVAS_SUPPORT=1
	
	# -s ASYNCIFY=2 -s ASYNCIFY_IGNORE_INDIRECT=1 -s JSPI_EXPORTS=_fl_run_jspi,__wasm_call_ctors -s EXPORTED_FUNCTIONS=_fl_run_jspi,_main -s OFFSCREENCANVAS_SUPPORT=1 -s EXPORTED_RUNTIME_METHODS=ccall


	# -sASYNCIFY=1 -sASYNCIFY_ADVISE=1
	
	#  -s JSPI=1 -sJSPI_EXPORTS=['_main_thread','_fl_run_jspi'] -sASYNCIFY_IGNORE_INDIRECT=1 -sPROXY_TO_PTHREAD=0

}
_compile_cad(){
	em++ -w /home/super/msv/appcad/missioncad.cpp /home/super/msv/common/general.cpp /home/super/msv/common/fl_scintilla.cpp /home/super/msv/common/scintilla_370/scintilla_fltk_amalgamated.cpp /home/super/msv/common/fl_browser_msv.cpp -o app.mjs  --bind  -O2 -I/home/super/msv/emsdk/downloads/occt_wasm/include/opencascade -L/home/super/msv/emsdk/downloads/occt_wasm/lib -lTKDESTEP -lTKDEIGES -lTKDESTL -lTKDEOBJ -lTKDEVRML -lTKDEPLY -lTKDECascade -lTKDE -lTKXSBase  -lTKXCAF -lTKVCAF -lTKCAF -lTKLCAF -lTKCDF  -lTKXmlXCAF -lTKBinXCAF -lTKXml -lTKBin -lTKXmlL -lTKBinL -lTKXmlTObj -lTKBinTObj -lTKTObj -lTKStd -lTKStdL  -lTKRWMesh -lTKMeshVS -lTKXMesh -lTKV3d -lTKService -lTKOpenGles -lTKHLR  -lTKHelix -lTKOffset -lTKFillet -lTKFeat -lTKBO -lTKBool -lTKShHealing  -lTKTopAlgo -lTKGeomAlgo -lTKMesh -lTKPrim -lTKBRep -lTKExpress  -lTKGeomBase -lTKG3d -lTKG2d -lTKMath -lTKernel /home/super/msv/emsdk/downloads/gmsh_wasm/lib/libgmsh.a -I/home/super/msv/emsdk/downloads/gmsh_wasm/include  -L/home/super/msv/common/scintilla_370 -l:libscinte.a  -I/home/super/msv/emsdk/downloads/fltk_wasm   -I/home/super/msv/emsdk/downloads/fltk_wasm/build_wasm   /home/super/msv/emsdk/downloads/fltk_wasm/build_wasm/lib/libfltk.a -I/home/super/msv/common -I/home/super/msv/common/sol2/include -I/home/super/msv/appcad  -I/home/super/msv/common/scintilla_370 -L/home/super/msv/emsdk/downloads/lua-5.5.1-wasm/lib -llua -I/home/super/msv/emsdk/downloads/lua-5.5.1-wasm/include -I/home/super/msv/common/scintilla_370/include/ -I/home/super/msv/common/scintilla_370/lexlib -I/home/super/msv/common/scintilla_370/src -DSCI_LEXER -DSCI_NAMESPACE  -D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS   -s MODULARIZE=1  -sSTACK_SIZE=5MB -sINITIAL_MEMORY=64MB -sGROWABLE_ARRAYBUFFERS=2  -sMAXIMUM_MEMORY=1024MB -pthread -std=c++20 -fwasm-exceptions  --preload-file /home/super/msv/appcad/lua@lua -s USE_WEBGL2=1 -s FULL_ES3=1 -s EXPORTED_RUNTIME_METHODS=ccall,FS --preload-file /home/super/msv/appcad/DejaVuSans-Bold.ttf@/fonts/DejaVuSans-Bold.ttf  -s PTHREAD_POOL_SIZE=4 -s PTHREAD_POOL_SIZE_STRICT=0 -sSUPPORT_LONGJMP=wasm -sJSPI --pre-js pre.js
	# -sJSPI_EXPORTS=['main','_main']
  	# -sASSERTIONS=1 -gsource-map
	# -s ASYNCIFY=1
	# -sSUPPORT_LONGJMP=wasm
	
	# ccache em++ -w /home/super/msv/appcad/missioncad.cpp /home/super/msv/common/general.cpp /home/super/msv/common/fl_scintilla.cpp /home/super/msv/common/scintilla_370/scintilla_fltk_amalgamated.cpp /home/super/msv/common/fl_browser_msv.cpp -o app.mjs   -O2 -I/home/super/msv/emsdk/downloads/occt_wasm/include/opencascade -Wl,--start-group /home/super/msv/emsdk/downloads/occt_wasm/lib/*.a -Wl,--end-group /home/super/msv/emsdk/downloads/gmsh_wasm/lib/libgmsh.a -I/home/super/msv/emsdk/downloads/gmsh_wasm/include  -L/home/super/msv/common/scintilla_370 -l:libscinte.a  -I/home/super/msv/emsdk/downloads/fltk_wasm   -I/home/super/msv/emsdk/downloads/fltk_wasm/build_wasm   /home/super/msv/emsdk/downloads/fltk_wasm/build_wasm/lib/libfltk.a -I/home/super/msv/common -I/home/super/msv/common/sol2/include -I/home/super/msv/appcad  -I/home/super/msv/common/scintilla_370 -L/home/super/msv/emsdk/downloads/lua-5.5.1-wasm/lib -llua -I/home/super/msv/emsdk/downloads/lua-5.5.1-wasm/include -I/home/super/msv/common/scintilla_370/include/ -I/home/super/msv/common/scintilla_370/lexlib -I/home/super/msv/common/scintilla_370/src -DSCI_LEXER -DSCI_NAMESPACE  -D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS --bind   -s MODULARIZE=1  -s ASYNCIFY=1 -sSTACK_SIZE=5MB -sINITIAL_MEMORY=64MB -sGROWABLE_ARRAYBUFFERS=2  -sMAXIMUM_MEMORY=512MB -pthread -std=c++20 -fwasm-exceptions -sSUPPORT_LONGJMP=wasm --preload-file /home/super/msv/appcad/lua@lua -s USE_WEBGL2=1 -s FULL_ES3=1 -s EXPORTED_RUNTIME_METHODS=ccall

}

_compile_cad1(){
	em++   /home/super/msv/appcad/missioncad.cpp /home/super/msv/common/general.cpp /home/super/msv/common/fl_scintilla.cpp /home/super/msv/common/scintilla_370/scintilla_fltk_amalgamated.cpp /home/super/msv/common/fl_browser_msv.cpp -o app.mjs   -O2 -I/home/super/msv/emsdk/downloads/occt_wasm/include/opencascade -L/home/super/msv/emsdk/downloads/occt_wasm/lib -lTKDESTEP -lTKDEIGES -lTKDESTL -lTKDEOBJ -lTKDEVRML -lTKDEPLY -lTKDECascade -lTKDE -lTKXSBase  -lTKXCAF -lTKVCAF -lTKCAF -lTKLCAF -lTKCDF  -lTKXmlXCAF -lTKBinXCAF -lTKXml -lTKBin -lTKXmlL -lTKBinL -lTKXmlTObj -lTKBinTObj -lTKTObj -lTKStd -lTKStdL  -lTKRWMesh -lTKMeshVS -lTKXMesh -lTKV3d -lTKService -lTKOpenGles -lTKHLR  -lTKHelix -lTKOffset -lTKFillet -lTKFeat -lTKBO -lTKBool -lTKShHealing  -lTKTopAlgo -lTKGeomAlgo -lTKMesh -lTKPrim -lTKBRep -lTKExpress  -lTKGeomBase -lTKG3d -lTKG2d -lTKMath -lTKernel /home/super/msv/emsdk/downloads/gmsh_wasm/lib/libgmsh.a -I/home/super/msv/emsdk/downloads/gmsh_wasm/include  -L/home/super/msv/common/scintilla_370 -l:libscinte.a  -I/home/super/msv/emsdk/downloads/fltk_wasm   -I/home/super/msv/emsdk/downloads/fltk_wasm/build_wasm   /home/super/msv/emsdk/downloads/fltk_wasm/build_wasm/lib/libfltk.a -I/home/super/msv/common -I/home/super/msv/common/sol2/include -I/home/super/msv/appcad  -I/home/super/msv/common/scintilla_370 -L/home/super/msv/emsdk/downloads/lua-5.5.1-wasm/lib -llua -I/home/super/msv/emsdk/downloads/lua-5.5.1-wasm/include -I/home/super/msv/common/scintilla_370/include/ -I/home/super/msv/common/scintilla_370/lexlib -I/home/super/msv/common/scintilla_370/src -DSCI_LEXER -DSCI_NAMESPACE  -D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS --bind  -sSTACK_SIZE=5MB -sINITIAL_MEMORY=128MB -sALLOW_MEMORY_GROWTH=1 -sMAXIMUM_MEMORY=2GB -sGROWABLE_ARRAYBUFFERS=4  -pthread -std=c++20 -fwasm-exceptions -sSUPPORT_LONGJMP=wasm --preload-file /home/super/msv/appcad/lua@lua -s USE_WEBGL2=1 -s FULL_ES3=1 -s EXPORTED_RUNTIME_METHODS=ccall,FS --preload-file /home/super/msv/appcad/DejaVuSans-Bold.ttf@/fonts/DejaVuSans-Bold.ttf   -sJSPI=1 -sMODULARIZE=1 --pre-js pre.js -matomics -mbulk-memory -sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency -sPTHREAD_POOL_SIZE_STRICT=2 

	#working
	# ccache em++ -w  /home/super/msv/appcad/missioncad.cpp /home/super/msv/common/general.cpp /home/super/msv/common/fl_scintilla.cpp /home/super/msv/common/scintilla_370/scintilla_fltk_amalgamated.cpp /home/super/msv/common/fl_browser_msv.cpp -o app.mjs   -O2 -I/home/super/msv/emsdk/downloads/occt_wasm/include/opencascade -L/home/super/msv/emsdk/downloads/occt_wasm/lib -lTKDESTEP -lTKDEIGES -lTKDESTL -lTKDEOBJ -lTKDEVRML -lTKDEPLY -lTKDECascade -lTKDE -lTKXSBase  -lTKXCAF -lTKVCAF -lTKCAF -lTKLCAF -lTKCDF  -lTKXmlXCAF -lTKBinXCAF -lTKXml -lTKBin -lTKXmlL -lTKBinL -lTKXmlTObj -lTKBinTObj -lTKTObj -lTKStd -lTKStdL  -lTKRWMesh -lTKMeshVS -lTKXMesh -lTKV3d -lTKService -lTKOpenGles -lTKHLR  -lTKHelix -lTKOffset -lTKFillet -lTKFeat -lTKBO -lTKBool -lTKShHealing  -lTKTopAlgo -lTKGeomAlgo -lTKMesh -lTKPrim -lTKBRep -lTKExpress  -lTKGeomBase -lTKG3d -lTKG2d -lTKMath -lTKernel /home/super/msv/emsdk/downloads/gmsh_wasm/lib/libgmsh.a -I/home/super/msv/emsdk/downloads/gmsh_wasm/include  -L/home/super/msv/common/scintilla_370 -l:libscinte.a  -I/home/super/msv/emsdk/downloads/fltk_wasm   -I/home/super/msv/emsdk/downloads/fltk_wasm/build_wasm   /home/super/msv/emsdk/downloads/fltk_wasm/build_wasm/lib/libfltk.a -I/home/super/msv/common -I/home/super/msv/common/sol2/include -I/home/super/msv/appcad  -I/home/super/msv/common/scintilla_370 -L/home/super/msv/emsdk/downloads/lua-5.5.1-wasm/lib -llua -I/home/super/msv/emsdk/downloads/lua-5.5.1-wasm/include -I/home/super/msv/common/scintilla_370/include/ -I/home/super/msv/common/scintilla_370/lexlib -I/home/super/msv/common/scintilla_370/src -DSCI_LEXER -DSCI_NAMESPACE  -D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS --bind  -sSTACK_SIZE=5MB -sINITIAL_MEMORY=64MB -sGROWABLE_ARRAYBUFFERS=2  -sMAXIMUM_MEMORY=1024MB -pthread -std=c++20 -fwasm-exceptions -sSUPPORT_LONGJMP=wasm --preload-file /home/super/msv/appcad/lua@lua -s USE_WEBGL2=1 -s FULL_ES3=1 -s EXPORTED_RUNTIME_METHODS=ccall,FS --preload-file /home/super/msv/appcad/DejaVuSans-Bold.ttf@/fonts/DejaVuSans-Bold.ttf   -sJSPI=1 -sMODULARIZE=1 --pre-js pre.js -matomics -mbulk-memory -sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency -sPTHREAD_POOL_SIZE_STRICT=2 
	# -sASSERTIONS=1 -gsource-map
	
	# ccache em++ -w /home/super/msv/appcad/missioncad.cpp /home/super/msv/common/general.cpp /home/super/msv/common/fl_scintilla.cpp /home/super/msv/common/scintilla_370/scintilla_fltk_amalgamated.cpp /home/super/msv/common/fl_browser_msv.cpp -o app.mjs   -O2 -I/home/super/msv/emsdk/downloads/occt_wasm/include/opencascade -Wl,--start-group /home/super/msv/emsdk/downloads/occt_wasm/lib/*.a -Wl,--end-group /home/super/msv/emsdk/downloads/gmsh_wasm/lib/libgmsh.a -I/home/super/msv/emsdk/downloads/gmsh_wasm/include  -L/home/super/msv/common/scintilla_370 -l:libscinte.a  -I/home/super/msv/emsdk/downloads/fltk_wasm   -I/home/super/msv/emsdk/downloads/fltk_wasm/build_wasm   /home/super/msv/emsdk/downloads/fltk_wasm/build_wasm/lib/libfltk.a -I/home/super/msv/common -I/home/super/msv/common/sol2/include -I/home/super/msv/appcad  -I/home/super/msv/common/scintilla_370 -L/home/super/msv/emsdk/downloads/lua-5.5.1-wasm/lib -llua -I/home/super/msv/emsdk/downloads/lua-5.5.1-wasm/include -I/home/super/msv/common/scintilla_370/include/ -I/home/super/msv/common/scintilla_370/lexlib -I/home/super/msv/common/scintilla_370/src -DSCI_LEXER -DSCI_NAMESPACE  -D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS --bind   -s MODULARIZE=1  -s ASYNCIFY=1 -sSTACK_SIZE=5MB -sINITIAL_MEMORY=64MB -sGROWABLE_ARRAYBUFFERS=2  -sMAXIMUM_MEMORY=512MB -pthread -std=c++20 -fwasm-exceptions -sSUPPORT_LONGJMP=wasm --preload-file /home/super/msv/appcad/lua@lua -s USE_WEBGL2=1 -s FULL_ES3=1 -s EXPORTED_RUNTIME_METHODS=ccall

}

install_em(){
	# Clonar o repositório do Emscripten
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

# Instalar e ativar a versão mais recente
./emsdk install latest
./emsdk activate latest
}

cfg_em(){
# Configurar as variáveis de ambiente no terminal atual
source /home/super/msv/emsdk/emsdk_env.sh
}

_build_fltk_wasm_sne() {
    local dir_name="fltk_wasm_sne"
    local repo_url="https://github.com/MoAlyousef/fltk_wasm32_emscripten.git"

    # 1. Verify that the Emscripten SDK environment is active
    if ! command -v emcmake &> /dev/null; then
        echo "Error: 'emcmake' not found. Please activate your Emscripten SDK first (e.g., source ./emsdk_env.sh)."
        return 1
    fi

    # 2. Clone or update the repository on the 'emscripten' branch
    if [ ! -d "$dir_name" ]; then
        echo "Cloning $repo_url (emscripten branch)..."
        git clone -b emscripten "$repo_url"  "$dir_name" || return 1
    else
        echo "Repository already exists. Switching to emscripten branch and pulling latest..."
        cd "$dir_name" || return 1
        git checkout emscripten
        git pull origin emscripten
        cd ..
    fi

    # 3. Enter directory and clear old build cache
    cd "$dir_name" || return 1
    rm -rf build_wasm

    echo "Configuring FLTK for wasm32 with CMake..."
    emcmake cmake -B build_wasm \
		-DCMAKE_CXX_FLAGS="" \
  		-DCMAKE_C_FLAGS="" \
        -DCMAKE_BUILD_TYPE=Release \
        -DFLTK_BUILD_TEST=OFF \
        -DFLTK_BUILD_EXAMPLES=OFF \
        -DFLTK_BUILD_FLUID=OFF \
        -DFLTK_BUILD_FLTK_OPTIONS=OFF \
        -DFLTK_BUILD_GL=ON || { cd - > /dev/null; return 1; }

    # 4. Build the core FLTK libraries including OpenGL support
    echo "Building FLTK wasm library..."
    local cpu_count=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    cmake --build build_wasm --parallel "$cpu_count" || { cd - > /dev/null; return 1; }

    echo "Successfully built FLTK for wasm32! Artifacts are located in: $(pwd)/build_wasm"
    cd - > /dev/null
}

_build_fltk_wasmdelete() {
    local dir_name="fltk_wasm"
    local repo_url="https://github.com/MoAlyousef/fltk_wasm32_emscripten.git"

    # 1. Verify that the Emscripten SDK environment is active
    if ! command -v emcmake &> /dev/null; then
        echo "Error: 'emcmake' not found. Please activate your Emscripten SDK first (e.g., source ./emsdk_env.sh)."
        return 1
    fi

    # 2. Clone or update the repository on the 'emscripten' branch
    if [ ! -d "$dir_name" ]; then
        echo "Cloning $repo_url (emscripten branch)..."
        git clone -b emscripten "$repo_url"  "$dir_name" || return 1
    else
        echo "Repository already exists. Switching to emscripten branch and pulling latest..."
        cd "$dir_name" || return 1
        git checkout emscripten
        git pull origin emscripten
        cd ..
    fi

    # 3. Enter directory and clear old build cache
    cd "$dir_name" || return 1
    rm -rf build_wasm

    echo "Configuring FLTK for wasm32 with CMake..."
    emcmake cmake -B build_wasm \
		-DCMAKE_CXX_FLAGS="-matomics -mbulk-memory -fwasm-exceptions -sSUPPORT_LONGJMP=wasm -pthread" \
  		-DCMAKE_C_FLAGS="-matomics -mbulk-memory -fwasm-exceptions -sSUPPORT_LONGJMP=wasm -pthread" \
        -DCMAKE_BUILD_TYPE=Release \
        -DFLTK_BUILD_TEST=OFF \
        -DFLTK_BUILD_EXAMPLES=OFF \
        -DFLTK_BUILD_FLUID=OFF \
        -DFLTK_BUILD_FLTK_OPTIONS=OFF \
        -DFLTK_BUILD_GL=ON || { cd - > /dev/null; return 1; }

    # 4. Build the core FLTK libraries including OpenGL support
    echo "Building FLTK wasm library..."
    local cpu_count=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    cmake --build build_wasm --parallel "$cpu_count" || { cd - > /dev/null; return 1; }

    echo "Successfully built FLTK for wasm32! Artifacts are located in: $(pwd)/build_wasm"
    cd - > /dev/null
}

_build_fltk_wasm() {
    local dir_name="fltk_wasm"
    local repo_url="https://github.com/MoAlyousef/fltk_wasm32_emscripten.git"

    # 1. Verificar Emscripten
    if ! command -v emcmake >/dev/null 2>&1; then
        echo "Error: 'emcmake' not found. Activate Emscripten first."
        return 1
    fi

    # 2. Clonar ou atualizar o repositório
    if [ ! -d "$dir_name" ]; then
        echo "Cloning $repo_url..."
        git clone -b emscripten "$repo_url" "$dir_name" || return 1
    else
        echo "Updating FLTK repository..."
        (
            cd "$dir_name" &&
            git checkout emscripten &&
            git pull origin emscripten
        ) || return 1
    fi

    # 3. Entrar no diretório
    cd "$dir_name" || return 1

    local driver="src/drivers/Emscripten/Fl_Emscripten_System_Driver.cxx"

    if [ ! -f "$driver" ]; then
        echo "Error: FLTK Emscripten driver not found: $driver"
        cd - >/dev/null
        return 1
    fi

    # 4. Aplicar o patch JSPI: remover emscripten_sleep(0)
    echo "Patching FLTK Emscripten system driver..."

    python3 - "$driver" <<'PY'
import pathlib
import re
import sys

path = pathlib.Path(sys.argv[1])
src = path.read_text()

pattern = re.compile(
    r'double Fl_Emscripten_System_Driver::wait\(double v\)\s*\{.*?'
    r'\n\}',
    re.S
)

replacement = """double Fl_Emscripten_System_Driver::wait(double v) {
  double ret = Fl_System_Driver::wait(v);
  Fl::flush();
  return ret;
}"""

patched, count = pattern.subn(replacement, src, count=1)

if count != 1:
    if replacement in src:
        print("Patch already applied.")
        sys.exit(0)
    print("Error: could not locate wait() implementation; source unchanged.")
    sys.exit(1)

path.write_text(patched)
print("Patch applied successfully.")
PY

    if [ $? -ne 0 ]; then
        cd - >/dev/null
        return 1
    fi

    # 5. Limpar build anterior
    rm -rf build_wasm

    # 6. Configurar FLTK WASM
    echo "Configuring FLTK for WebAssembly..."

    emcmake cmake -B build_wasm \
        -DCMAKE_CXX_FLAGS="-matomics -mbulk-memory -fwasm-exceptions -pthread" \
        -DCMAKE_C_FLAGS="-matomics -mbulk-memory -fwasm-exceptions -pthread" \
        -DCMAKE_BUILD_TYPE=Release \
        -DFLTK_BUILD_TEST=OFF \
        -DFLTK_BUILD_EXAMPLES=OFF \
        -DFLTK_BUILD_FLUID=OFF \
        -DFLTK_BUILD_FLTK_OPTIONS=OFF \
        -DFLTK_BUILD_GL=ON || {
            cd - >/dev/null
            return 1
        }

    # 7. Compilar
    local cpu_count
    cpu_count=$(nproc 2>/dev/null || echo 4)

    echo "Building patched FLTK WASM..."
    cmake --build build_wasm --parallel "$cpu_count" || {
        cd - >/dev/null
        return 1
    }

    echo "FLTK WASM successfully built."
    echo "Artifacts: $(pwd)/build_wasm"

    cd - >/dev/null
}





_build_lua_wasm() {
    # set -e

    local LUA_VER="5.5.1"
    local INSTALL_DIR="$(pwd)/lua-${LUA_VER}-wasm"
    local BUILD_DIR="$(pwd)/bld_lua_wasm"
	local JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

    echo "=========================================="
    echo "Iniciando Build do Lua ${LUA_VER} para WASM (Multithread / Pthreads)"
    echo "=========================================="

    # 1. Verificações de Ambiente Emscripten
    if [ -z "$EMSDK" ]; then
        echo "ERRO: Variável EMSDK não encontrada. Execute 'source /path/to/emsdk/emsdk_env.sh'."
        return 1
    fi
    echo "Encontrado EMSDK em: $EMSDK"

    # 2. Download e Extração
    echo "[1/4] Baixando e extraindo Lua ${LUA_VER}..."
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    wget -q https://www.lua.org/ftp/lua-${LUA_VER}.tar.gz
    tar xf lua-${LUA_VER}.tar.gz
    cd lua-${LUA_VER}

    # 3. Compilação com Emscripten e Pthreads
    echo "[2/4] Compilando Lua com suporte a Multithreading (Pthreads)..."
    make -j"${JOBS}" generic \
        CC="emcc" \
        AR="emar rcu" \
        RANLIB="emranlib" \
        MYCFLAGS="-O3  -fwasm-exceptions -sSUPPORT_LONGJMP=wasm -pthread -D_REENTRANT" \
        MYLDFLAGS="-fwasm-exceptions -sSUPPORT_LONGJMP=wasm -pthread"

    # 4. Organizando a Instalação
    echo "[3/4] Organizando diretórios de instalação em ${INSTALL_DIR}..."
    mkdir -p "${INSTALL_DIR}/include"
    mkdir -p "${INSTALL_DIR}/lib"
    mkdir -p "${INSTALL_DIR}/bin"

    cp src/lua.h src/luaconf.h src/lualib.h src/lauxlib.h "${INSTALL_DIR}/include/"
    if [ -f "src/lua.hpp" ]; then
        cp src/lua.hpp "${INSTALL_DIR}/include/"
    fi
    cp src/liblua.a "${INSTALL_DIR}/lib/"

	# 5. Compilando o binário standalone lua.js / lua.wasm com pool de threads e growable array buffers
    echo "[4/4] Gerando executável standalone com pool de threads WASM..."
    emcc -O3 src/lua.o src/liblua.a -o "${INSTALL_DIR}/bin/lua.js" \
		-fwasm-exceptions -sSUPPORT_LONGJMP=wasm \
        -pthread \
        -sPTHREAD_POOL_SIZE=4 \
        -sALLOW_MEMORY_GROWTH=1 \
        -sGROWABLE_ARRAYBUFFERS=2 \
        -sEXPORTED_FUNCTIONS='["_main"]' \
        -sEXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]'

    echo "=========================================="
    echo "Lua ${LUA_VER} Multithread WASM instalado com sucesso em: ${INSTALL_DIR}"
    echo "Cabeçalhos: ${INSTALL_DIR}/include"
    echo "Biblioteca: ${INSTALL_DIR}/lib/liblua.a"
    echo "Binário JS: ${INSTALL_DIR}/bin/lua.js"
    echo "=========================================="
}



_build_gmsh_wasm() {
    # set -e

    # --- Configurações de Caminhos para WASM --- 
    local OCC_PREFIX="$(pwd)/occt_wasm"
    local PREFIX="$(pwd)/gmsh_wasm"
    local BUILD_DIR="$(pwd)/bld_gmsh_wasm"
    local JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    
    echo "=========================================="
    echo "Iniciando Build do Gmsh para WebAssembly Multi-Thread (Emscripten)"
    echo "=========================================="

    # 1. Verificações de Ambiente Emscripten
    echo "[1/5] Verificando ambiente Emscripten..."
    if [ -z "$EMSDK" ]; then
        echo "ERRO: Variável EMSDK não encontrada. Certifique-se de executar 'source /path/to/emsdk/emsdk_env.sh'."
        return 1
    fi
    echo "Encontrado EMSDK em: $EMSDK"

    # 2. Verificações da Instalação OCCT WASM
    echo "[2/5] Validando instalação WASM do OCCT em ${OCC_PREFIX}..."
    local occ_cmake_dir="${OCC_PREFIX}/lib/cmake/opencascade"

    if [ ! -d "${OCC_PREFIX}" ]; then
        echo "ERRO: Diretório do OCCT WASM não existe: ${OCC_PREFIX}"
        echo "Execute primeiro o script _build_occt_wasm()."
        return 1
    fi

    if [ ! -f "${occ_cmake_dir}/OpenCASCADEConfig.cmake" ]; then
        echo "ERRO: Ficheiro CMake do OCCT não encontrado em: ${occ_cmake_dir}"
        return 1
    fi

    echo "OCCT WASM validado em ${OCC_PREFIX} ✔"

    # 3. Limpeza e Download do Código Fonte do Gmsh
    echo "[3/5] Preparando ambiente de build..."
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    # Baixando versão estável ou git source do Gmsh
    if [ ! -f "gmsh-git-source.tgz" ]; then
        wget -q https://gmsh.info/src/gmsh-4.15.2-source.tgz
    fi

    if [ -d "gmsh-git-source" ]; then rm -rf gmsh-git-source; fi
    if [ -d "gmsh-4.15.2-source" ]; then rm -rf gmsh-4.15.2-source; fi

    if [ -f "gmsh-git-source.tgz" ]; then
        tar xf gmsh-git-source.tgz
        cd gmsh-git-source
    else
        tar xf gmsh-4.15.2-source.tgz
        cd gmsh-4.15.2-source
    fi

    mkdir build && cd build

    echo "[4/5] Configurando CMake com emcmake para WebAssembly (Biblioteca Estática MT)..."
	emcmake cmake .. \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX="${PREFIX}" \
		-DCMAKE_CXX_STANDARD=17 \
		-DCMAKE_CXX_STANDARD_REQUIRED=ON \
		-DCMAKE_CXX_EXTENSIONS=OFF \
		-DCMAKE_CXX_FLAGS="-pthread -fwasm-exceptions -sSUPPORT_LONGJMP=wasm" \
		-DCMAKE_C_FLAGS="-pthread -fwasm-exceptions -sSUPPORT_LONGJMP=wasm" \
		-DENABLE_OPENMP=OFF \
		-DENABLE_OCC=ON \
		-DENABLE_PARSER=ON \
		-DCMAKE_PREFIX_PATH="${OCC_PREFIX}" \
		-DENABLE_BUILD_LIB=ON \
		-DENABLE_BUILD_SHARED=OFF \
		-DENABLE_BUILD_DYNAMIC=OFF \
		-DENABLE_FLTK=OFF \
		-DENABLE_GRAPHICS=OFF \
		-DENABLE_POST=ON \
		-DENABLE_PLUGINS=OFF


    # 5. Compilação e Instalação (Sem sudo, direcionado ao workspace local)
    echo "[5/5] Compilando Gmsh para WebAssembly com ${JOBS} threads..."
    cmake --build . -j"${JOBS}"
    cmake --install .

    echo "=========================================="
    echo "Gmsh WebAssembly compilado com sucesso em: ${PREFIX}"
    echo "=========================================="
}



_build_occt_wasm(){
# ==============================================================================
# Prerequisite Reminder:
# Make sure you have activated your Emscripten environment before running:
# source /path/to/emsdk/emsdk_env.sh
# ==============================================================================

INSTALL_DIR="$(pwd)/occt_wasm"
BUILD_DIR="build_wasm"

echo "=== [1/5] Checking Emscripten environment ==="
if [ -z "$EMSDK" ]; then
    echo "Error: EMSDK environment variable is not set. Please source your emsdk_env.sh first."
    return 1
fi
echo "Found EMSDK at: $EMSDK"

echo "=== [2/5] Cloning OpenCASCADE Source Repository ==="
if [ ! -d "occt" ]; then
    # Clone stable branch (e.g., V7_8_0 or adjust tag/branch as needed)
    git clone --depth 1 --branch OCCT-801 https://github.com/Open-Cascade-SAS/OCCT.git occt
else
    echo "OCCT source folder already exists. Skipping clone."
fi

cd occt
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "=== [3/5] Configuring OCCT with emcmake ==="
emcmake cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_LIBRARY_TYPE=Static \
    -DBUILD_MODULE_Draw=OFF \
    -DBUILD_USE_PCH=OFF \
    -DUSE_FREETYPE=OFF \
    -DUSE_PTHREADS=ON \
    -DCMAKE_CXX_FLAGS="-pthread -fwasm-exceptions" \
    -DCMAKE_C_FLAGS="-pthread" \
    -DCMAKE_INSTALL_PREFIX="/home/super/msv/emsdk/downloads/occt_wasm" \
    -D3RDPARTY_FREETYPE_INCLUDE_DIR="$EMSDK/upstream/emscripten/cache/sysroot/include/freetype2"


echo "=== [4/5] Compiling OCCT WebAssembly Static Libraries ==="
# Detect available CPU cores for parallel build
CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
echo "Building using $CORES parallel jobs..."
cmake --build . --parallel -j"$CORES"

echo "=== [5/5] Installing Headers and Libraries ==="
cmake --install .

echo "========================================================================"
echo "Build Complete Successfully!"
echo "Headers located at:   $INSTALL_DIR/include/opencascade"
echo "Libraries located at: $INSTALL_DIR/lib"
echo "========================================================================"
}

install_ncnn() {
    GREEN="\e[32m"
    RED="\e[31m"
    YELLOW="\e[33m"
    BLUE="\e[34m"
    RESET="\e[0m"

    echo -e "${BLUE}==> Instalando dependências...${RESET}"
    sudo apt update && sudo apt install -y \
        git cmake build-essential libprotobuf-dev protobuf-compiler \
        libvulkan-dev vulkan-tools glslang-tools

    if [[ $? -ne 0 ]]; then
        echo -e "${RED}Erro ao instalar dependências.${RESET}"
        return 1
    fi

    echo -e "${BLUE}==> Clonando/atualizando repositório NCNN...${RESET}"
    if [[ ! -d ncnn ]]; then
        git clone https://github.com/Tencent/ncnn.git
        if [[ $? -ne 0 ]]; then
            echo -e "${RED}Erro ao clonar o repositório NCNN.${RESET}"
            return 1
        fi
    else
        echo -e "${YELLOW}Repositório já existe, atualizando...${RESET}"
        cd ncnn && git pull && cd ..
    fi

    echo -e "${BLUE}==> Baixando submódulos obrigatórios...${RESET}"
    cd ncnn
    git submodule update --init --recursive
    if [[ $? -ne 0 ]]; then
        echo -e "${RED}Erro ao baixar submódulos.${RESET}"
        return 1
    fi

    echo -e "${BLUE}==> Preparando build...${RESET}"
    rm -rf build
    mkdir build
    cd build

    echo -e "${BLUE}==> Executando CMake...${RESET}"
    cmake -DNCNN_VULKAN=ON -DCMAKE_INSTALL_PREFIX=/usr/local ..
    if [[ $? -ne 0 ]]; then
        echo -e "${RED}Erro no CMake. Verifique dependências ou submódulos.${RESET}"
        return 1
    fi

    echo -e "${BLUE}==> Compilando NCNN...${RESET}"
    make -j"$(nproc)"
    if [[ $? -ne 0 ]]; then
        echo -e "${RED}Erro ao compilar NCNN.${RESET}"
        return 1
    fi

    echo -e "${BLUE}==> Instalando NCNN no sistema...${RESET}"
    sudo make install
    if [[ $? -ne 0 ]]; then
        echo -e "${RED}Falha ao instalar NCNN.${RESET}"
        return 1
    fi

    echo -e "${GREEN}✔ NCNN instalado com sucesso!${RESET}"
}




qemu_xp(){
	# \\10.0.2.4\qemu
	# genisoimage -J -r -joliet-long -o acad2004.iso /mnt/windows/desk/acad2004
	qemu-system-i386 \
	-enable-kvm \
	-cpu qemu32 \
	-smp 1 \
	-m 256 \
	-drive file=windows_xp.qcow2,format=qcow2,cache=writeback \
	-cdrom acad2004.iso \
	-netdev user,id=n1,hostfwd=tcp::13389-:3389,smb=/mnt/windows/desk/ \
	-device rtl8139,netdev=n1 \
	-vga cirrus \
	&

	sleep 2
	sed -i '/\[global\]/a server min protocol = NT1\nlanman auth = yes\nntlm auth = yes' /tmp/qemu-smb.*/smb.conf
	#   -net user,smb=/mnt/windows/desk/ \
}











# --- Dispatcher ---
func="$1"
shift

# Check if function exists
if ! declare -F "$func" >/dev/null; then
    echo -e "${RED}Error:${RESET} function '${YELLOW}$func${RESET}' not found."
    return 1
fi

# Optional: show what is being executed
echo -e "${GREEN}Executing function:${RESET} $func $*"

# Call the function
"$func" "$@"

# Apaga AUTOMATICAMENTE todas as funções que começam por _
# (Procura no ambiente as funções com esse nome e faz unset nelas)
unset -f $(compgen -A function _) 2>/dev/null
