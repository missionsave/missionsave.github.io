  em++ \
  /home/super/msv/appcad/missioncad.cpp \
  /home/super/msv/common/general.cpp \
  /home/super/msv/common/fl_scintilla.cpp \
  /home/super/msv/common/scintilla_370/scintilla_fltk_amalgamated.cpp \
  /home/super/msv/common/fl_browser_msv.cpp \
  -o app.mjs \
  -O2 \
  -I/home/super/msv/emsdk/downloads/occt_wasm/include/opencascade \
  -L/home/super/msv/emsdk/downloads/occt_wasm/lib \
  -lTKDESTEP -lTKDEIGES -lTKDESTL -lTKDEOBJ -lTKDEVRML -lTKDEPLY -lTKDECascade -lTKDE -lTKXSBase \
  -lTKXCAF -lTKVCAF -lTKCAF -lTKLCAF -lTKCDF \
  -lTKXmlXCAF -lTKBinXCAF -lTKXml -lTKBin -lTKXmlL -lTKBinL -lTKXmlTObj -lTKBinTObj -lTKTObj -lTKStd -lTKStdL \
  -lTKRWMesh -lTKMeshVS -lTKXMesh -lTKV3d -lTKService -lTKOpenGles -lTKHLR \
  -lTKHelix -lTKOffset -lTKFillet -lTKFeat -lTKBO -lTKBool -lTKShHealing \
  -lTKTopAlgo -lTKGeomAlgo -lTKMesh -lTKPrim -lTKBRep -lTKExpress \
  -lTKGeomBase -lTKG3d -lTKG2d -lTKMath -lTKernel \
  /home/super/msv/emsdk/downloads/gmsh_wasm/lib/libgmsh.a \
  -I/home/super/msv/emsdk/downloads/gmsh_wasm/include \
  -L/home/super/msv/common/scintilla_370 -l:libscinte.a \
  -I/home/super/msv/emsdk/downloads/fltk_wasm \
  -I/home/super/msv/emsdk/downloads/fltk_wasm/build_wasm \
  /home/super/msv/emsdk/downloads/fltk_wasm/build_wasm/lib/libfltk.a \
  -I/home/super/msv/common \
  -I/home/super/msv/common/sol2/include \
  -I/home/super/msv/appcad \
  -I/home/super/msv/common/scintilla_370 \
  -L/home/super/msv/emsdk/downloads/lua-5.5.1-wasm/lib -llua \
  -I/home/super/msv/emsdk/downloads/lua-5.5.1-wasm/include \
  -I/home/super/msv/common/scintilla_370/include/ \
  -I/home/super/msv/common/scintilla_370/lexlib \
  -I/home/super/msv/common/scintilla_370/src \
  -DSCI_LEXER -DSCI_NAMESPACE \
  -D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS \
  --bind \
  -sSTACK_SIZE=5MB \
  -sINITIAL_MEMORY=512MB \
  -sALLOW_MEMORY_GROWTH=0 \
  -sMAXIMUM_MEMORY=2GB \
  -sGROWABLE_ARRAYBUFFERS=2 \
  -pthread \
  -std=c++20 \
  -fwasm-exceptions \
  -sSUPPORT_LONGJMP=wasm \
  --preload-file /home/super/msv/appcad/lua@lua \
  -sUSE_WEBGL2=1 \
  -sFULL_ES3=1 \
  -sEXPORTED_RUNTIME_METHODS=ccall,FS \
  --preload-file /home/super/msv/appcad/DejaVuSans-Bold.ttf@/fonts/DejaVuSans-Bold.ttf \
  -sJSPI=1 \
  -sMODULARIZE=1 \
  --pre-js pre.js \
  -matomics -mbulk-memory \
  -sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency \
  -sPTHREAD_POOL_SIZE_STRICT=2