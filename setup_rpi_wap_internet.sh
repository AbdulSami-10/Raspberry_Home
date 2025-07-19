#!/bin/bash

echo "📡 Setting up Raspberry Pi as Wi-Fi Access Point + Internet Gateway..."

# Step 1: Install required packages
echo "📦 Installing hostapd, dnsmasq, and iptables..."
sudo apt update
sudo apt install hostapd dnsmasq iptables-persistent -y

# Step 2: Set static IP for wlan0
echo "🛜 Configuring static IP for wlan0..."
sudo sed -i '/^interface wlan0/d' /etc/dhcpcd.conf
echo -e "\ninterface wlan0\nstatic ip_address=192.168.4.1/24\nnohook wpa_supplicant" | sudo tee -a /etc/dhcpcd.conf
sudo service dhcpcd restart

# Step 3: Configure hostapd
echo "📶 Setting up hostapd (Wi-Fi AP)..."
sudo bash -c 'cat > /etc/hostapd/hostapd.conf' <<EOF
interface=wlan0
driver=nl80211
ssid=Rasp_Home
hw_mode=g
channel=7
wmm_enabled=0
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=Password123
wpa_key_mgmt=WPA-PSK
rsn_pairwise=CCMP
EOF

sudo sed -i 's|^#DAEMON_CONF=.*|DAEMON_CONF="/etc/hostapd/hostapd.conf"|' /etc/default/hostapd
sudo systemctl unmask hostapd
sudo systemctl enable hostapd
sudo systemctl start hostapd

# Step 4: Configure dnsmasq
echo "🌐 Configuring DHCP with dnsmasq..."
sudo mv /etc/dnsmasq.conf /etc/dnsmasq.conf.backup
sudo bash -c 'cat > /etc/dnsmasq.conf' <<EOF
interface=wlan0
dhcp-range=192.168.4.10,192.168.4.100,255.255.255.0,24h
EOF
sudo systemctl restart dnsmasq

# Step 5: Enable IP forwarding
echo "🔁 Enabling IP forwarding..."
sudo sed -i 's/#net.ipv4.ip_forward=1/net.ipv4.ip_forward=1/' /etc/sysctl.conf
sudo sysctl -p

# Step 6: Set up NAT and forwarding rules
echo "🔥 Setting up NAT for internet sharing..."
sudo iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
sudo iptables -A FORWARD -i eth0 -o wlan0 -m state --state RELATED,ESTABLISHED -j ACCEPT
sudo iptables -A FORWARD -i wlan0 -o eth0 -j ACCEPT
sudo netfilter-persistent save

# Step 7: Final reboot
echo "✅ Setup complete. Rebooting now..."
sudo reboot
