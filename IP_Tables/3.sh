# Count packets on Ethernet (eth0)
iptables -A INPUT -i eth0 -j ACCEPT

# Count packets on Wi-Fi (wlan0)
iptables -A INPUT -i wl01 -j ACCEPT

# Count packets going out through Ethernet
iptables -A OUTPUT -o eth0 -j ACCEPT

# Count packets going out through Wi-Fi
iptables -A OUTPUT -o wl01 -j ACCEPT

