#!/usr/bin/env bash
#
# setup_minimal_wifi.sh
# Stand-alone script to install minimal Wi-Fi support and optionally connect to a network.
#
# Usage:
#   sudo ./setup_minimal_wifi.sh                # just install & configure
#   sudo ./setup_minimal_wifi.sh MySSID MyPass  # also connect to MySSID

#set -eo pipefail

setup_minimal_wifi() {
  local SSID="$1"
  local PSK="$2"

  echo "⏳ Installing dependencies..."
  sudo apt-get update -qq
  sudo apt-get install -y ifupdown dhcpcd5 wpasupplicant wpasupplicant-utils wireless-tools

  echo "🚫 Disabling other network managers/services..."
  local services=(
    NetworkManager
    wicd
    connman
    systemd-networkd
    resolvconf
	wpa_supplicant
  )
  for svc in "${services[@]}"; do
    sudo systemctl stop    "$svc"     2>/dev/null || true
    sudo systemctl disable "$svc"     2>/dev/null || true
    sudo update-rc.d -f "${svc,,}" remove 2>/dev/null || true
  done

  echo "🔧 Writing DNS templates..."
  sudo tee /etc/resolv.conf.head >/dev/null <<EOF
search lan

EOF

  sudo tee /etc/resolv.conf.tail >/dev/null <<EOF

nameserver 1.1.1.1
nameserver 8.8.8.8

EOF

  echo "🔧 Generating /etc/dhcpcd.conf..."
  sudo tee /etc/dhcpcd.conf >/dev/null <<EOF

hostname
clientid
persistent
option rapid_commit
option domain_name_servers, domain_name, domain_search, host_name
require dhcp_server_identifier
option interface_mtu
slaac private

EOF

  echo "🔍 Detecting wireless interface..."
  IFACE=$(iw dev 2>/dev/null | awk '$1=="Interface"{print $2; exit}')
  if [[ -z "$IFACE" ]]; then
    echo "❌ No Wi-Fi interface found."
    exit 1
  fi

  echo "🔧 Writing /etc/network/interfaces..."
  sudo tee /etc/network/interfaces >/dev/null <<EOF
auto lo
iface lo inet loopback

allow-hotplug $IFACE
iface $IFACE inet dhcp
    wpa-conf /etc/wpa_supplicant/wpa_supplicant.conf

EOF

  echo "🔧 Ensuring wpa_supplicant config..."
  local CONF="/etc/wpa_supplicant/wpa_supplicant.conf"
  local CTRL="ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev"
  local UPD="update_config=1"

  sudo touch "$CONF"
  if ! grep -Fxq "$CTRL" "$CONF"; then
    echo "$CTRL" | sudo tee -a "$CONF" >/dev/null
  fi
  if ! grep -Fxq "$UPD" "$CONF"; then
    echo "$UPD"  | sudo tee -a "$CONF" >/dev/null
  fi

  echo "👥 Adding \$SUDO_USER to netdev group..."
  sudo usermod -aG netdev "$SUDO_USER"

  if [[ -n "$SSID" && -n "$PSK" ]]; then
    echo "📶 Configuring network: $SSID"

    NETID=$(sudo wpa_cli -i "$IFACE" add_network | tr -d '\r\n')
    sudo wpa_cli -i "$IFACE" set_network "$NETID" ssid "\"$SSID\"" >/dev/null
    sudo wpa_cli -i "$IFACE" set_network "$NETID" psk "\"$PSK\"" >/dev/null
    sudo wpa_cli -i "$IFACE" enable_network "$NETID" >/dev/null
    sudo wpa_cli -i "$IFACE" save_config >/dev/null
    sudo wpa_cli -i "$IFACE" reconfigure >/dev/null

	sudo systemctl unmask dhcpcd.service
	#sudo apt install --reinstall dhcpcd

    echo "🟢 Bringing up interface $IFACE..."
    sudo ifup "$IFACE"
    echo "✅ Connected to $SSID"
  else
    echo "✅ Minimal Wi-Fi setup complete.  To connect later, run:"
    echo "   sudo $0 \"MySSID\" \"MyPassword\""
  fi
}

# Entry point
if [[ $EUID -ne 0 ]]; then
  echo "Please run this script with sudo or as root."
  exit 1
fi

# Pass through any SSID/PSK args
setup_minimal_wifi "$1" "$2"
