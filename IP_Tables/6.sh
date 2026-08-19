iptables -A OUTPUT -p icmp -j LOG --log-prefix "ICMP-OUT: "  --log-level 4
