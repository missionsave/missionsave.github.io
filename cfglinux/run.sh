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

_build_fltk_wasm() {
    local repo_url="https://github.com/MoAlyousef/fltk_wasm32_emscripten.git"
    local dir_name="fltk_wasm32_emscripten"

    # 1. Verify that the Emscripten SDK environment is active
    if ! command -v emcmake &> /dev/null; then
        echo "Error: 'emcmake' not found. Please activate your Emscripten SDK first (e.g., source ./emsdk_env.sh)."
        return 1
    fi

    # 2. Clone or update the repository on the 'emscripten' branch
    if [ ! -d "$dir_name" ]; then
        echo "Cloning $repo_url (emscripten branch)..."
        git clone -b emscripten "$repo_url" || return 1
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


_build_occt_wasm(){
# ==============================================================================
# Prerequisite Reminder:
# Make sure you have activated your Emscripten environment before running:
# source /path/to/emsdk/emsdk_env.sh
# ==============================================================================

INSTALL_DIR="$(pwd)/occt_wasm_install"
BUILD_DIR="build_wasm"

echo "=== [1/5] Checking Emscripten environment ==="
if [ -z "$EMSDK" ]; then
    echo "Error: EMSDK environment variable is not set. Please source your emsdk_env.sh first."
    exit 1
fi
echo "Found EMSDK at: $EMSDK"

echo "=== [2/5] Cloning OpenCASCADE Source Repository ==="
if [ ! -d "occt" ]; then
    # Clone stable branch (e.g., V7_8_0 or adjust tag/branch as needed)
    git clone --depth 1 --branch V7_8_0 https://github.com/Open-Cascade-SAS/OCCT.git occt
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
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
    -D3RDPARTY_FREETYPE_INCLUDE_DIR="$EMSDK/upstream/emscripten/cache/sysroot/include/freetype2"

echo "=== [4/5] Compiling OCCT WebAssembly Static Libraries ==="
# Detect available CPU cores for parallel build
CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
echo "Building using $CORES parallel jobs..."
cmake --build . -j"$CORES"

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
    exit 1
fi

# Optional: show what is being executed
echo -e "${GREEN}Executing function:${RESET} $func $*"

# Call the function
"$func" "$@"

# Apaga AUTOMATICAMENTE todas as funções que começam por _
# (Procura no ambiente as funções com esse nome e faz unset nelas)
unset -f $(compgen -A function _) 2>/dev/null
