
# Raspberry Pi WAP + Internet Sharing Setup

This script turns your Raspberry Pi into a Wi-Fi Access Point (WAP) and shares internet with connected IoT devices.

## 🔧 Features
- Static IP assignment to wlan0
- Configures hostapd (Wi-Fi Access Point)
- Configures dnsmasq (DHCP server)
- Enables IP forwarding and NAT for internet access
- Sets up firewall rules using iptables

## 🚀 Usage
1. Upload `setup_rpi_wap_internet.sh` to your Raspberry Pi.
2. Make it executable:
   ```bash
   chmod +x setup_rpi_wap_internet.sh
   ```
3. Run it:
   ```bash
   ./setup_rpi_wap_internet.sh
   ```

📌 Default Wi-Fi SSID: `Rasp_Home`  
🔑 Password: `Password123`

## 💡 Tip
To restore a secure/private IoT-only mode, disable IP forwarding and flush the iptables NAT rules.
