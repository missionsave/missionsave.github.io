#!/bin/bash

# Ensure script is run as root
if [ "$(id -u)" != "0" ]; then
   echo "This script must be run as root (use sudo)." 1>&2
   exit 1
fi

# Create udev rule file
cat <<EOF > /etc/udev/rules.d/90-backlight.rules
# Allow 'video' group to control real backlight brightness
ACTION=="add", SUBSYSTEM=="backlight", RUN+="/bin/chgrp video /sys/class/backlight/%k/brightness"
ACTION=="add", SUBSYSTEM=="backlight", RUN+="/bin/chmod g+w /sys/class/backlight/%k/brightness"
EOF

# Reload udev and apply
udevadm control --reload
udevadm trigger

cat <<EOF > /etc/udev/rules.d/91-input.rules
# Allow 'input' group to read input devices
KERNEL=="event*", GROUP="input", MODE="0640"
EOF

# Add user to 'video' group (optional)
if id "$SUDO_USER" &>/dev/null; then
    echo "Adding user '$SUDO_USER' to group 'video' and 'input'..."
    usermod -aG video "$SUDO_USER"
    usermod -aG input "$SUDO_USER"
else
    echo "Warning: could not detect non-root user for group assignment."
fi

echo "✔ Udev rule installed. Reboot or re-login for group changes to take effect."
