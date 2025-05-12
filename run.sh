
export RTE_SDK=/disk3/jantarmantar/jobhaunt/opt/
export RTE_TARGET=x86_64-linux-gnu

export PKG_CONFIG_PATH=/disk3/jantarmantar/jobhaunt/opt/lib/x86_64-linux-gnu/pkgconfig/
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/disk3/jantarmantar/jobhaunt/opt/lib/x86_64-linux-gnu/

mkdir -p /dev/hugepages
sudo mountpoint -q /dev/hugepages || mount -t hugetlbfs nodev /dev/hugepages
echo '8' | sudo tee -a  /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages

cat /proc/meminfo | grep Huge

sudo tcpdump -i eno1 -w input.pcap

sudo ./dpdk-helloworld -c7 --vdev='net_pcap0,rx_pcap=input.pcap,tx_pcap=output.pcap'  --  -i --nb-cores=2 --nb-ports=1 --total-num-mbufs=2048

#sudo ../../dpdk-21.02/build/app/dpdk-testpmd -c7 --vdev=net_pcap0,iface=eno1  --  -i --nb-cores=2 --nb-ports=1 --total-num-mbufs=256
sudo ../../dpdk-21.02/build/app/dpdk-testpmd -c7 --vdev='net_pcap0,rx_pcap=input.pcap,tx_pcap=output.pcap'  --  -i --nb-cores=2 --nb-ports=1 --total-num-mbufs=2048


sudo nginz -c7 --vdev='net_pcap0,rx_pcap=input.pcap,tx_pcap=output.pcap'  --  -i --nb-cores=2 --nb-ports=1 --total-num-mbufs=2048

