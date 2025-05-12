

#### DPDK build guide

```
meson setup -Dexamples=all -Dprefix=/opt build
ninja 
meson install
export PKG_CONFIG_PATH=/opt/lib/x86_64-linux-gnu/pkgconfig/
```
```
meson setup -Dexamples=all build
ninja 
meson install
```

#### Environment

```
mkdir -p /dev/hugepages
sudo mountpoint -q /dev/hugepages || mount -t hugetlbfs nodev /dev/hugepages
echo '8' | sudo tee -a  /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages

cat /proc/meminfo | grep Huge
```

#### pcap ethernet device

```
sudo tcpdump -i eno1 -w input.pcap
```

#### NUMA node

```
echo 0 > /sys/devices/pci0000\:00/<pci_id>/numa_node
```

#### Invokation


```
sudo build/reactordpdk -c3 --vdev='net_pcap0,rx_pcap=/tmp/input.pcap,tx_pcap=output.pcap'
```

