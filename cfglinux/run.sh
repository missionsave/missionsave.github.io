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


_compile_fltk_wasm_notworking() {
    local fltk_dir="${1:-fltk-src}"
    
    if [ -f "$HOME/msv/emsdk/emsdk_env.sh" ]; then
        source "$HOME/msv/emsdk/emsdk_env.sh" > /dev/null 2>&1
    elif [ -n "$EMSDK" ]; then
        source "$EMSDK/emsdk_env.sh" > /dev/null 2>&1
    fi

    if [ ! -d "$fltk_dir" ]; then
        echo "A clonar o repositório oficial do FLTK (branch 1.4)..."
        git clone --branch branch-1.4 --depth 1 https://github.com/fltk/fltk.git "$fltk_dir"
    else
        echo "A limpar diretório de build anterior..."
        rm -rf "$fltk_dir/build-wasm"
    fi

    echo "A configurar o projeto FLTK sem suporte a imagens/jpeg/png..."
    emcmake cmake -B "$fltk_dir/build-wasm" -S "$fltk_dir" \
        -DCMAKE_BUILD_TYPE=Release \
        -DOPTION_BUILD_SHARED_LIBS=OFF \
        -DOPTION_BUILD_EXAMPLES=OFF \
        -DOPTION_BUILD_TESTS=OFF \
        -DOPTION_USE_X11=OFF \
        -DOPTION_USE_GL=OFF \
        -DOPTION_USE_THREADS=OFF \
        -DOPTION_USE_PLUGINS=OFF \
        -DOPTION_USE_libjpeg=OFF \
        -DOPTION_USE_libpng=OFF \
        -DOPTION_USE_zlib=OFF \
        -DCMAKE_CXX_FLAGS="-D__EMSCRIPTEN__"

    echo "A compilar o FLTK para WASM..."
    cmake --build "$fltk_dir/build-wasm" -j$(nproc)

    if [ $? -eq 0 ]; then
        echo "=========================================="
        echo "FLTK compilado com sucesso para WASM!"
        echo "Os ficheiros .a estão em: $fltk_dir/build-wasm/lib/"
        echo "=========================================="
    else
        echo "A compilação falhou."
        return 1
    fi
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
