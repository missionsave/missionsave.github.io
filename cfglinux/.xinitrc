#setxkbmap pt

xrdb -merge ~/.Xresources

if grep -q "VirtualBox" /sys/class/dmi/id/product_name; then
	VBoxClient --clipboard
fi


ARCH=$(uname -m)

if [[ "$ARCH" == "aarch64" ]]; then
    echo "Running on ARM64"
    # Launch ARM-specific apps or settings
    
	if ! tmux has-session 2>/dev/null; then
		tmux new-session -A -s aarch64 bash
	fi

    
elif [[ "$ARCH" == "x86_64" ]]; then
    echo "Running on x86_64"
    # Launch x86-specific apps or settings

	if ! tmux has-session 2>/dev/null; then
		tmux new-session -A -s x86_64 bash
	fi    
    
    
else
    echo "Unknown architecture: $ARCH"
fi

#exec xterm > ./xlog1.txt 2>&1
exec openbox-session > ./xlog1.txt 2>&1

