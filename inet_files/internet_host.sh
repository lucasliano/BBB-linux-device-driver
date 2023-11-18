sudo sysctl -w net.ipv4.ip_forward=1
sudo iptables -t nat -A POSTROUTING -o wlo1 -j MASQUERADE
sudo iptables -A FORWARD -i enx04a316e8f797 -o wlo1 -j ACCEPT
sudo iptables -A FORWARD -i wlo1 -o enx04a316e8f797 -j ACCEPT
sudo iptables -S | grep wlo1
