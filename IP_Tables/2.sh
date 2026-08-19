# blocking incoming ping requests
iptables -A INPUT -p icmp --icmp-type echo-request -j DROP
iptables -A OUTPUT -p icmp --icmp-type echo-reply -j DROP

# blocking in opposite direction i.e. local machine requesting
iptables -A INPUT -p icmp --icmp-type echo-reply -j DROP
iptables -A OUTPUT -p icmp --icmp-type echo-request -j DROP
