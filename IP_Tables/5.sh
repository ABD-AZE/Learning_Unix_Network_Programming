# dropping packets received from google.com (8.8.8.8 is its ip)
iptables -t filter -A INPUT -s 8.8.8.8 -j DROP