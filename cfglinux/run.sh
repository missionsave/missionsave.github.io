#!/usr/bin/env bash

# --- Colors ---
RED="\e[31m"
GREEN="\e[32m"
YELLOW="\e[33m"
RESET="\e[0m"

# --- Example functions ---
mytask() {
    echo -e "${GREEN}Running mytask with args:${RESET} $*"
}

othertask() {
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
    cmake -DNCNN_VULKAN=ON ..
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
