#setxkbmap pt
#setxkbmap -model abnt2 -layout br
#loadkeys br-abnt2


# Caminho padr�o
export PATH="$PATH:/$HOME/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin"

export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/home/super/desk/missionsave/robot_driver/openvino_toolkit_ubuntu20_2025.0.0.17942.1f68be9f594_x86_64/runtime/lib/intel64"

export PATH="$PATH:/usr/lib/x86_64-linux-gnu"

#style.*.default=$(font:Courier New,bold)

setdatei(){
	sudo date -s "$(wget -qSO- --max-redirect=0 google.com 2>&1 | grep Date: | cut -d' ' -f5-8)Z"
}

comments(){
# Store the block in a variable (with quotes to preserve formatting)
plugin_content=$(cat << 'EOF'
Raspberry pi stats:
#pi zero 2w
#FLTK version: 1.3.8
#OpenSceneGraph version: 3.6.5
#OCCT version: 7.6.3
#OpenGL Vendor:   Broadcom
#OpenGL Renderer: VC4 V3D 2.1
#OpenGL Version:  2.1 Mesa 24.2.8-1~bpo12+rpt2

#pi 4b
#OpenGL Vendor:   Broadcom
#OpenGL Renderer: V3D 4.2.14.0
#OpenGL Version:  3.1 Mesa 24.2.8-1~bpo12+rpt2


sudo apt install realvnc-vnc-server realvnc-vnc-viewer -y
sudo systemctl enable vncserver-x11-serviced.service
sudo systemctl start vncserver-x11-serviced.service

https://download.nomachine.com/download/9.0/Arm/nomachine_9.0.188_11_arm64.deb
sudo dpkg -i nomachine_*.deb

https://github.com/TurboVNC/turbovnc/releases/download/3.2/turbovnc_3.2_arm64.deb

xhost +si:localuser:$(whoami)  # Allow your user access


tmux -L inner
tmux set-option prefix C-a
#tmux bind-key C-a send-prefix

tmux ls
0: 1 windows (created Sun Jul  6 21:41:47 2025) (attached)
tmux attach -t 0

top -p $(pgrep xcape)


sudo apt-get install libxfixes-dev






_
EOF
)

# Write the variable to a file
echo "$plugin_content"  #> "$plugin_file"
}

xclip1() {
  if [[ "$*" == *"-selection clipboard"* ]]; then
    echo "$(command xclip -o -sel clip)" >> ~/.clip_history
  fi
    xclip "$@"
}



# Override xclip to log clipboard changes
function xclip() {
    local log_file="$HOME/.clipboard_log"

    # Call the real xclip with arguments
    command xclip "$@"

    # Only log if copying to clipboard (-selection clipboard and -i/-in)
    if [[ "$*" == *"-selection clipboard"* ]] && [[ "$*" == *"-in"* || "$*" == *"-i"* ]]; then
        local content
        content=$(command xclip -selection clipboard -o 2>/dev/null)
        
        if [[ -n "$content" ]]; then
            echo "=== $(date '+%Y-%m-%d %H:%M:%S') ===" >> "$log_file"
            echo "$content" >> "$log_file"
            echo "" >> "$log_file"  # Add a blank line for separation
        fi
    fi
}




set_prompt() {
    local user="$(whoami)"
    local host="$(hostname)"
    local user_host="$user@$host"

    # Default PS1 (no red background)
    local default_ps1='\[\e[0m\][\u@\h:\W]\$ '

    # Red background for 'super@raspberrypi'
    #~ if [[ "$user_host" == "super@raspberrypi" ]]; then
    if [[ "$user_host" == "super@debianlite" ]]; then
        PS1='\[\e[43;30m\]vbox:\W\$ \[\e[0m\]'  # Red background
    #else
       # PS1="$default_ps1"
    fi
}

set_prompt

sshtt_old() {
    local IP_SUFFIX="${1:-190}"
    local SSH_COMMAND="ssh -XY -c aes128-gcm@openssh.com -o Compression=yes -o ForwardX11Trusted=yes super@192.168.1.${IP_SUFFIX} -t 'tmux new-session -A -s aarch64 bash'"

    # Check if we are already in a tmux session
    if [ -n "$TMUX" ]; then
        # If in tmux, create a new window and execute the SSH command
        tmux new-window -n "ssh-${IP_SUFFIX}" "$SSH_COMMAND"
    fi
}


ssht() {
    local IP_SUFFIX="${1:-190}"
    xterm -e ssh -XY -c aes128-gcm@openssh.com \
        -o Compression=yes -o ForwardX11Trusted=yes \
        super@192.168.1."$IP_SUFFIX" \
        -t 'tmux new-session -A -s aarch64 bash' &
}
sshtprev() {
    local IP_SUFFIX="${1:-190}"
    xterm -e ssh -XY -c aes128-gcm@openssh.com \
        -o Compression=yes -o ForwardX11Trusted=yes \
        super@192.168.1."$IP_SUFFIX" \
        -t 'tmux attach || tmux new' &
}





mapwifi(){
	 nmap -sL 192.168.1.* | grep \(1
}

memc(){
	awk '{ printf "Orig: %.1f MiB\nCompr: %.1f MiB\nUsado: %.1f MiB\nCompress�o: %.2fx\n", $1/1024/1024, $2/1024/1024, $3/1024/1024, $1/$2 }' /sys/block/zram0/mm_stat
}

alias cpdrv='mkdir -p ~/driver && rsync -u frobot  frobot.sqlite ~/driver/ && rsync -aru stl ~/driver/'

alias mntdesk='sudo mount -t vboxsf -o exec desk ~/desk && cd ~/desk/missionsave/robot_driver'

alias mntmsv='sudo mount -t vboxsf -o exec missionsave ~/missionsave && cd ~/missionsave/approbot'

alias sshraspi='ssh -X super@192.168.1.190'
alias mntraspi='sshfs super@192.168.1.190:/ ~/server'
alias umntraspi='fusermount -u ~/server'

xprarobo(){
xpra start :100   --start=/home/super/missionsave/approbot/frobota   --bind-tcp=0.0.0.0:14500   --html=on   --daemon=no   --xvfb="/usr/bin/Xvfb +extension Composite -screen 0 640x400x16"   --pulseaudio=no   --notifications=no --quality=50 --min-quality=30 
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

ramopt() {
set -e

echo "=== Otimizando Raspberry Pi Zero 2 W ==="

echo "[1/4] Instalando zram-tools e earlyoom..."
sudo apt update
sudo apt install -y zram-tools earlyoom

echo "[2/4] Configurando zram..."
sudo bash -c 'cat > /etc/default/zramswap <<EOF
ALGO=lz4
PERCENT=50
EOF'

sudo systemctl enable --now zramswap.service

echo "[3/4] Ativando earlyoom..."
sudo systemctl enable --now earlyoom

echo "[4/4] (Opcional) Deseja desativar swap no disco? [s/N]"
read -r DISABLE_SWAP
if [[ "$DISABLE_SWAP" == "s" || "$DISABLE_SWAP" == "S" ]]; then
    echo "Desativando swap em disco..."
    sudo swapoff -a
    sudo sed -i '/swap/d' /etc/fstab
fi

sudo mkswap /dev/zram0
sudo swapon /dev/zram0


echo "Finalizado! Use 'htop' para ver zram em a��o."
}

installDesk() {
    local appname="$1"
    local exec="$2"
    local file="$HOME/.local/share/applications/${appname// /_}.desktop"

    mkdir -p "$HOME/.local/share/applications"

    cat > "$file" <<EOF
[Desktop Entry]
Type=Application
Name=$appname
Exec=$exec
Icon=utilities-terminal
Terminal=false
Categories=Utility;
EOF

    chmod +x "$file"

    echo "Created $file"
}

installReload(){
	installDesk "tmux $(uname -m)" "xterm -e tmux new-session -A -s $(uname -m) bash"
}



installdeb() {
  local url="$1"
  local filename=$(basename "$url")

  # Verifica se a URL foi fornecida
  if [[ -z "$url" ]]; then
    echo "❌ Erro: URL não fornecida."
    return 1
  fi

  # Download
  echo "⬇️ Downloading $filename ..."
  if ! wget "$url" -O "$filename"; then
    echo "❌ Falha no download de $filename."
    return 1
  fi

  # Verifica se o arquivo é um .deb
  if [[ "$filename" != *.deb ]]; then
    echo "❌ $filename não é um arquivo .deb válido."
    rm -f "$filename"
    return 1
  fi

  # Instalação
  echo "🔧 Installing $filename ..."
  if ! sudo dpkg -i "$filename"; then
    echo "⚠️ Corrigindo dependências quebradas ..."
    sudo apt-get install -f -y || {
      echo "❌ Falha ao corrigir dependências."
      rm -f "$filename"
      return 1
    }
  fi

  # Limpeza
  echo "🧹 Cleaning up ..."
  rm -f "$filename"

  echo "✅ $filename instalado com sucesso!"
}

# only make -f GNUmakefile 
install_uwebsockets() {
    # Set up working directory
    WORKDIR=buildtemp
    mkdir "$WORKDIR"
    echo "Using temp directory: $WORKDIR"
    cd "$WORKDIR" || exit 1

    # Clone the repo
    echo "Cloning uWebSockets..."
    git clone https://github.com/uNetworking/uWebSockets.git
    cd uWebSockets || exit 1
    
    git submodule update --init --recursive

    # Build using Makefile
    echo "Building with make..."
    make -f GNUmakefile -j"$(nproc)"

    # Install manually
    echo "Installing manually..."
    sudo cp uwebsockets /usr/local/bin/

    # Cleanup
    echo "Cleaning up..."
    #rm -rf "$WORKDIR"

    echo "uWebSockets installed successfully!"
}



installvirtualglarm() {
	wget https://github.com/VirtualGL/virtualgl/releases/download/3.1.3/virtualgl_3.1.3_arm64.deb
	sudo dpkg -i virtualgl_3.1.3_arm64.deb
	sudo apt-get install -f
	unlink ./virtualgl_3.1.3_arm64.deb
}
installvirtualglamd() {
	wget https://github.com/VirtualGL/virtualgl/releases/download/3.1.3/virtualgl_3.1.3_amd64.deb
	sudo dpkg -i virtualgl_3.1.3_amd64.deb
	sudo apt-get install -f
	unlink ./virtualgl_3.1.3_amd64.deb
}


mntraspi2(){
	sshfs super@192.168.1.198:/ ~/server
	ssh -XY -c aes128-gcm@openssh.com -o Compression=yes -o ForwardX11Trusted=yes super@192.168.1.198 -t 'tmux attach || tmux new || tmux set-option prefix C-a'


#ssh -XY -c aes128-gcm@openssh.com -o Compression=no -o ForwardX11Trusted=yes super@192.168.1.198 


	#ssh -X super@192.168.1.198
	#ssh -X -c chacha20-poly1305@openssh.com super@192.168.1.198
	#ssh -X -c arcfour,blowfish-cbc super@192.168.1.198
}

fstream(){
ffmpeg -f x11grab -framerate 10 -video_size 640x480 -i :0.0 \
-c:v h264_omx -b:v 800k -pix_fmt yuv420p \
-f mpegts tcp://192.168.1.77:1234
}

fstreami(){
ffmpeg -f x11grab -framerate 10 -video_size 640x480 -i :0.0 \
-c:v h264_omx -b:v 800k -pix_fmt yuv420p \
-f mpegts -listen 1 tcp://0.0.0.0:1234
}





alias mntpen='sudo mkdir -p ~/pen && mountpoint -q ~/pen || sudo mount -o uid=$(id -u),gid=$(id -g),umask=000 /dev/sdb1 ~/pen'
alias umntpen='sudo umount -l ~/pen'

alias cdr='cd ~/desk/missionsave/robot_driver'

syncf() {
	DIR1="~/desk/missionsave/robot_driver"  # First directory (current directory)
	DIR2="~/pen/desk"  # Second directory
	DIR3="$HOME"  # Third directory


	# Function to sync two directories bidirectionally
	sync_dirs() {
	    local SRC=$1
	    local DEST=$2
	    local FILES=$3

	    for file in "${FILES[@]}"; do
		rsync -u "$SRC/$file" "$DEST/" 2>/dev/null
		rsync -u "$DEST/$file" "$SRC/" 2>/dev/null
	    done
	}
	
	# Array of files to sync
	files=("frobot" "frobot.sqlite" ) 
	sync_dirs "~/desk/missionsave/robot_driver" "$~/pen/desk"" $files
	
	f#iles=(".bashre" "frobot.sqlite" ) 
	#sync_dirs "~/desk/missionsave/robot_driver" "$~/pen/desk"" $files

	# Sync DIR2 <-> DIR3
	#sync_dirs "$DIR2" "$DIR3"

	# Sync DIR3 <-> DIR1
	#sync_dirs "$DIR3" "$DIR1"

}
 
alias sync='mntpen && syncf'

alias dropcache='echo 3 | sudo tee /proc/sys/vm/drop_caches'

alias scitecfgs='scite  ~/.tmux.conf ~/.xinitrc ~/.Xresources ~/.bashrc  ~/.config/openbox/show_windows.sh &'

topm() {
$1 top -o %MEM
}

findf() { find $1 -name "$2"; }

findstr() { grep -r "$2" $1; }

alias keypt='setxkbmap pt'
alias keybr='setxkbmap -model abnt2 -layout br'
alias keybr2='setxkbmap -model abnt2 -layout br && xmodmap -e "keycode 108 = Alt_R Meta_R" && xmodmap -e "add mod1 = Alt_L Alt_R"'

mem() { 
    total=0
    for pid in $(pgrep -x "$1"); do
        # Check ownership of the process
        if [[ -r "/proc/$pid/smaps" && $(stat -c '%u' /proc/$pid) -eq $(id -u) ]]; then
            echo "Readable: /proc/$pid/smaps"
            mem=$(awk '/^Pss:/ {sum+=$2} END {print sum}' "/proc/$pid/smaps" 2>/dev/null)
        else
            echo "Not readable, using sudo: /proc/$pid/smaps"
            mem=$(sudo awk '/^Pss:/ {sum+=$2} END {print sum}' "/proc/$pid/smaps" 2>/dev/null)
        fi
        mem=${mem:-0} # Default to 0 if awk fails
        total=$((total + mem))
    done
    echo "$total KB"
} 



export NNN_FCOLORS='c1e2272e006033f7c6d6abc4'  # Optimized for white bg
export NNN_COLORS='1234'


export EDITOR=vim
export VISUAL=vim
alias ncp="cat ${NNN_SEL:-${XDG_CONFIG_HOME:-$HOME/.config}/nnn/.selection} | tr '\0' '\n'"

nn() {
    #export NNN_CP='rsync -av --progress'
    export NNN_TMPFILE=/tmp/nnn_dir
    command nnn "$@"      # Launch nnn
    if [ -s "$NNN_TMPFILE" ]; then
        # Ler o caminho e remover 'cd ' prefix, aspas extras e espa�os em branco
        dir=$(sed -e "s/^cd //" -e "s/^['\"]//" -e "s/['\"]$//" "$NNN_TMPFILE" | tr -d '\n')

        if [ -d "$dir" ]; then
            cd "$dir" || echo "Failed to change directory"
        else
            echo "Directory \"$dir\" does not exist"
        fi
    fi
}
export NNN_PLUG='p:rsyncpaste'

create_rsyncpaste_plugin() {
    plugin_dir="${XDG_CONFIG_HOME:-$HOME/.config}/nnn/plugins"
    plugin_file="$plugin_dir/rsyncpaste"

    mkdir -p "$plugin_dir"

    cat > "$plugin_file" << 'EOF'
#!/bin/sh

sel=${NNN_SEL:-${XDG_CONFIG_HOME:-$HOME/.config}/nnn/.selection} 

xargs -0 -I % rsync -aru --no-owner --no-group --progress % "$PWD" < "$sel"

# Clear selection
if [ -p "$NNN_PIPE" ]; then
    printf "-" > "$NNN_PIPE"
fi

# POSIX-compliant pause (works in all shells)
printf "Press enter key to continue"
old_stty=$(stty -g)
stty raw -echo
key=$(dd bs=1 count=1 2>/dev/null)
stty "$old_stty"
EOF

    chmod +x "$plugin_file"
    echo "? Plugin 'rsyncpaste' created and made executable at: $plugin_file"
}

# permit_app_sudo /full/path/to/app
# permit_app_sudo /home/super/msv/apptest/key_detector
permit_app_sudo() {
    local APP_PATH="$1"
    local USER_NAME=$(whoami)
    local SUDOERS_DIR="/etc/sudoers.d"
    local RULE_FILE="$SUDOERS_DIR/90-brightnessctl-$USER_NAME"

    # Safety checks
    [[ -x "$APP_PATH" ]] || { echo "Error: '$APP_PATH' not executable"; return 1; }
    [[ "$USER_NAME" == "root" ]] && { echo "Don't run as root"; return 1; }

    # Create rule with strict command path
    echo "# Generated by permit_app_sudo on $(date)
$USER_NAME ALL=(root) NOPASSWD: $(realpath "$APP_PATH")" | sudo tee "$RULE_FILE" >/dev/null

    # Set secure permissions
    sudo chmod 0440 "$RULE_FILE"
    sudo chown root:root "$RULE_FILE"

    echo "✓ Created sudo rule in $RULE_FILE"
    echo "To remove: sudo rm '$RULE_FILE'"
}

# Usage example:
# permit_app_sudo "/path/to/your_app"

permit_video_input() {
    local APP_PATH="$1"
    local USER_NAME="$(whoami)"
    local RULES_FILE="/etc/udev/rules.d/99-${USER_NAME}-video-input.rules"

    if [[ -z "$APP_PATH" ]]; then
        echo "Usage: grant_device_access_for_app /full/path/to/app"
        return 1
    fi

    if [[ ! -x "$APP_PATH" ]]; then
        echo "❌ Error: '$APP_PATH' not found or not executable."
        return 1
    fi

    echo "Creating udev rules for video and input access..."

    sudo bash -c "cat > '$RULES_FILE' <<EOF
KERNEL==\"video*\", SUBSYSTEM==\"video4linux\", OWNER=\"$USER_NAME\", MODE=\"0660\"
KERNEL==\"input*\", SUBSYSTEM==\"input\", OWNER=\"$USER_NAME\", MODE=\"0660\"
EOF"

    sudo udevadm control --reload-rules && sudo udevadm trigger
    echo "Rules applied. Devices assigned to '$USER_NAME'."
}


installnnnlatest() {
    # Create a temporary directory
    local tmp_dir=$(mktemp -d) || { echo "Failed to create temp dir"; return 1; }
    
    echo "? Temporary directory: $tmp_dir"
    cd "$tmp_dir" || return 1

    # Download the latest nnn release
    echo "? Downloading the latest nnn release..."
    if ! curl -sL "https://github.com/jarun/nnn/archive/refs/heads/master.tar.gz" | tar xz --strip-components=1; then
        echo "? Failed to download/extract nnn"
        cd ~ && rm -rf "$tmp_dir"
        return 1
    fi

    # Compile and install
    echo "? Compiling and installing nnn..."
    if make && sudo make install; then
        echo "? Successfully installed nnn!"
    else
        echo "? Compilation/installation failed"
    fi

    # Clean up
    cd ~ && rm -rf "$tmp_dir"
    echo "? Temporary directory removed."
}







# ~/.bashrc - Useful configurations

# Prevent duplicate history entries
HISTCONTROL=ignoredups:erasedups

# Set history size
HISTSIZE=10000
HISTFILESIZE=20000

# Append to history file, don't overwrite
shopt -s histappend

# Check window size and update LINES and COLUMNS
shopt -s checkwinsize

# Enable better globbing
shopt -s extglob

# Colorful and readable 'ls'
alias lsp='ls --color=auto -h'
alias ll='ls -lah'
alias la='ls -A'

# Shortcuts
alias ..='cd ..'
alias ...='cd ../..'
alias ....='cd ../../..'
alias grep='grep --color=auto'

# Prevent accidental overwriting
alias cp='cp -i'
alias mv='mv -i'
alias rm='rm -i'

# Show history with time
export HISTTIMEFORMAT='%F %T '

# Update terminal title
export PROMPT_COMMAND='echo -ne "\033]0;${USER}@${HOSTNAME}: ${PWD}\007"'

# Load changes immediately
alias reload='source ~/.bashrc && openbox --reconfigure && xrdb -merge ~/.Xresources'




installer() {
	sudo apt update && sudo apt upgrade

	sudo apt install mesa-utils ssh sshfs nnn xclip xterm tmux curl wmctrl  openbox
	sudo apt install x11-xserver-utils
	
	#git clone https://github.com/jarun/nnn.git ~/.config/nnn/plugins
	
	sudo apt install libocct-foundation-dev libocct-modeling-data-dev libocct-modeling-algorithms-dev libocct-visualization-dev libocct-data-exchange-dev

	sudo apt update
	sudo apt install build-essential

	installdeb https://dl.google.com/linux/direct/google-chrome-stable_current_amd64.de

	installdeb https://vscode.download.prss.microsoft.com/dbazure/download/stable/cb0c47c0cfaad0757385834bd89d410c78a856c0/code_1.102.0-1752099874_amd64.deb

}

setcfg() {
  local KEYBIND="C-T"
  local COMMAND="tmux"
  local RCFILE="$HOME/.config/openbox/rc.xml"

  # Create file if it doesn't exist
  if [ ! -f "$RCFILE" ]; then
    mkdir -p "$(dirname "$RCFILE")"
    cat > "$RCFILE" <<"EOF"
<?xml version="1.0" encoding="UTF-8"?>
<openbox_config>
  <keyboard>
  </keyboard>
</openbox_config>
EOF
    echo "Created new Openbox config file."
  fi

  # Remove existing keybind block (if any)
  sed -i "/<keybind key=\"$KEYBIND\">/,/<\/keybind>/d" "$RCFILE"

  # Insert new keybind
  sed -i "/<\/keyboard>/i \
  <keybind key=\"$KEYBIND\">\n\
    <action name=\"Execute\">\n\
      <command>$COMMAND</command>\n\
    </action>\n\
  </keybind>" "$RCFILE"

  echo "Keybinding for $KEYBIND set to '$COMMAND'."
  openbox --reconfigure
}


autologin_setup() {
    local USERNAME="$1"
    
    if [ -z "$USERNAME" ]; then
        echo "? Uso: autologin_setup nome_de_utilizador"
        return 1
    fi

    echo "? A configurar auto login para o utilizador: $USERNAME"

    # Cria o diret�rio de override do servi�o getty@tty1
    sudo mkdir -p /etc/systemd/system/getty@tty1.service.d

    # Cria o ficheiro override.conf com o conte�do correto
    sudo tee /etc/systemd/system/getty@tty1.service.d/override.conf > /dev/null <<EOF
[Service]
ExecStart=
ExecStart=-/sbin/agetty --autologin $USERNAME --noclear %I \$TERM
EOF

    # Reaplica as configura��es do systemd
    echo "? A reinicializar o systemd..."
    sudo systemctl daemon-reexec

    echo "? Auto login configurado para '$USERNAME' no TTY1. Reinicia para aplicar:"
    echo "   sudo reboot"
}

raspihdmion(){
local ORIG_DISPLAY="$DISPLAY"
export DISPLAY=:0
sleep 7 && xrandr --output HDMI-2 --auto  && xrandr --output HDMI-1 --auto
 export DISPLAY="$ORIG_DISPLAY"
}




ssh_runapplocal(){
#~ xhost +SI:localuser:$(whoami)
#~ nohup bash -c 'DISPLAY=:0 your-app' > ~/your-app.log 2>&1 < /dev/null &

DISPLAY=:0 nohup "$1" &
}
setup_cfg() {
    #!/bin/bash
    set -euo pipefail

    # Source and destination definitions
    src_dir="./"
    # src_dir="$HOME/missionsave/cfglinux"
    dest_dir="$HOME"
    backup_dir="$HOME/.backupcfg"
    fnames=".tmux.conf .xinitrc .Xresources .bashrc .SciTEUser.properties .config/openbox/ .config/nnn/"

    # Create backup directory if needed
    mkdir -p "$backup_dir"

    for rel_path in $fnames; do
        # Remove trailing slash if present (for directory handling)
        rel_path="${rel_path%/}"
        src_path="$src_dir/$rel_path"
        dest_path="$dest_dir/$rel_path"

        # Skip if source doesn't exist
        if [[ ! -e "$src_path" ]]; then
            echo "NOT FOUND: $src_path"
            continue
        fi

        # Handle new items (no backup needed)
        if [[ ! -e "$dest_path" ]]; then
            echo "COPYING NEW: $src_path → $dest_path"
            mkdir -p "$(dirname "$dest_path")"
            if [[ -d "$src_path" ]]; then
                rsync -a "$src_path/" "$dest_path/"
            else
                rsync -a "$src_path" "$(dirname "$dest_path")/"
            fi
            continue
        fi

        # Compare items
        if diff -rq "$src_path" "$dest_path" &>/dev/null; then
            echo "UNCHANGED: $dest_path"
            continue
        fi

        # Create timestamped backup path
        timestamp=$(date +%Y-%m-%d-%H-%M)
        backup_path="$backup_dir/${rel_path}.${timestamp}"

        if [[ -f "$src_path" ]]; then
            # Handle files - move original to backup
            echo "BACKING UP FILE: $dest_path → $backup_path"
            mkdir -p "$(dirname "$backup_path")"
            mv "$dest_path" "$backup_path"
            
            echo "UPDATING FILE: $src_path → $dest_path"
            mkdir -p "$(dirname "$dest_path")"
            rsync -a "$src_path" "$(dirname "$dest_path")/"
        else
            # Handle directories - copy contents to backup, then sync new contents
            echo "BACKING UP DIRECTORY CONTENTS: $dest_path/ → $backup_path/"
            mkdir -p "$backup_path"
            rsync -a --delete "$dest_path/" "$backup_path/"
            
            echo "UPDATING DIRECTORY: $src_path/ → $dest_path/"
            mkdir -p "$dest_path"
            rsync -a --delete "$src_path/" "$dest_path/"
        fi
    done
}

# My Custom Functions
# -------------------

# Function to create a directory and then change into it
mcd() {
    mkdir -p "$1" && cd "$1"
}

# Function to list files by size (largest first)
lssize() {
    du -sh * | sort -rh
}

# Function for the "continue" prompt you asked about
confirm_continue() {
    while true; do
        read -rp "Do you want to continue? (yes/no): " choice
        case "$choice" in
            yes|Yes|y|Y ) break;;
            no|No|n|N ) echo "Aborting."; return 1;; # Return non-zero for 'no'
            * ) echo "Invalid choice. Please enter 'yes' or 'no'.";;
        esac
    done
    return 0 # Return zero for 'yes'
}



#~ if ! pgrep Xorg >/dev/null; then
    #~ echo "Xorg not loaded. Starting Xorg..."
    #~ exec startx
#~ else
    #~ echo "Xorg is already running."
#~ fi

#~ if ! tmux has-session 2>/dev/null; then
	#~ tmux new-session -A -s dash bash
    #~ #tmux new -d
#~ fi


# usage: bash cfglinux/.bashrc setup
_run_bashrc_setup() {
    if ! confirm_continue; then 
        echo "Setup operation cancelled."
        return 1 # Indicate failure to the caller if needed
    fi

    echo "Running BashRC setup tasks..."
    # --- START Your setup tasks here ---
    # Example:
    # echo "Setting up environment variables..."
    # export MY_VAR="some_value"
    # echo "Updating something..."
    # /path/to/some/script.sh --update


	setup_cfg

	chmod +x ~/.bashrc
	chmod +x ~/.config/openbox/show_windows.sh




    # --- END Your setup tasks here ---

    echo "BashRC setup complete."
    return 0 # Indicate success
}

# Conditional call to the setup function
# usage: source ~/.bashrc setup
# OR: . ~/.bashrc setup
# if [[ "$1" == "setup" && -n "$PS1" ]]; then # Added -n "$PS1" for interactive check
if [[ "$1" == "setup" ]]; then #  -n "$PS1" dont work via ssh
    # Check if this is an interactive shell for extra safety,
    # as bashrc is sometimes sourced in non-interactive contexts.
    _run_bashrc_setup
    # After running the setup, clear the argument to avoid issues if
    # bashrc is sourced again later in the same interactive session
    # with the 'setup' argument still lingering. This is a minor detail.
    # set -- # Clears all positional parameters
fi


#autostart
if [ -z "$DISPLAY" ] && [ "$XDG_VTNR" = 1 ] && [ -z "$SSH_CONNECTION" ]; then
  exec startx
fi

