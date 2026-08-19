iptables -Z
echo "Printing packet counts for each rule:"
iptables -L -v -n