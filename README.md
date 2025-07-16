# Raspberry Home – A Secure Private Network for Small IoT Devices
Raspberry Home is a security-focused IoT gateway framework that lets you connect, monitor, and manage small IoT devices like ESP8266 and ESP32 safely within an isolated private network. This project demonstrates how to build a Layer 3 gateway using a Raspberry Pi to provide device isolation, encrypted communication, secure OTA updates, and hassle-free remote access — all without exposing your IoT devices directly to the internet.

# Key Features

Layer 3 Gateway & Private Network ->
Creates a dedicated subnet for your IoT devices using dnsmasq and hostapd. IoT nodes are fully isolated from the main network, reducing the attack surface.

TLS-Encrypted MQTT Broker ->
Hosts a local Mosquitto broker with TLS enabled for secure publish/subscribe messaging between sensors, actuators, and the gateway.

Secure OTA Firmware Updates ->
Uses an HTTPS-enabled Apache server for delivering firmware updates to ESP devices. OTA version checks and updates run safely over your private network.

Firewall & Traffic Control ->
Combines iptables and UFW to enforce strict access rules. Blocks unauthorized traffic and prevents IoT devices from reaching the internet directly.

Remote Access via Tailscale VPN ->
Integrates Tailscale to enable zero-trust, peer-to-peer remote access to your gateway and device data from anywhere, without risky port forwarding or public IPs.

DIY-Friendly & Lightweight ->
Uses open-source tools and is optimized for constrained IoT hardware. Ideal for labs, smart home enthusiasts, and students.


# What’s Included
Full Raspberry Pi configuration: Wi-Fi Access Point, DHCP, DNS, and NAT.

Mosquitto broker setup with self-signed TLS certs.

Apache server for hosting OTA firmware with HTTPS.

Example ESP8266/ESP32 sketches for:

TLS-secured MQTT publishing.

OTA update from HTTPS server.

iptables & UFW rules for isolation and security.

Tailscale VPN setup for remote, secure access.


# Getting Started

1.Flash Raspberry Pi OS and install required packages: hostapd, dnsmasq, mosquitto, apache2, ufw, tailscale.
2.Configure Wi-Fi as an Access Point with hostapd.
3.Set up dnsmasq for DHCP and DNS within the private subnet.
4.Secure your MQTT broker with TLS certs.
5.Enable HTTPS on Apache for OTA.
6.Connect your ESP devices to the private network, test MQTT and OTA.
7.Enable Tailscale to access your Pi securely from anywhere.

