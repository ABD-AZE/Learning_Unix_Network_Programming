# used state and recent modules for this question 
iptables -A INPUT -p tcp --dport 22 -m state --state NEW -m recent --set --name SSH

iptables -A INPUT -p tcp --dport 22 -m state --state NEW -m recent --update --hitcount 4 --name SSH -j DROP

iptables -A INPUT -p tcp --dport 22 -j ACCEPT
