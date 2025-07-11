xrdb -merge ~/.Xresources

if grep -q "VirtualBox" /sys/class/dmi/id/product_name; then
	VBoxClient --clipboard
fi


#exec xterm > ./xlog1.txt 2>&1
exec openbox-session > ./xlog1.txt 2>&1

