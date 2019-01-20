echo "test" >> /home/st
sudo apt-get update
sudo apt-get install make gcc g++ unzip -y
sudo apt-get install zlib1g-dev libboost-all-dev -y
sudo apt-get install libssl-dev libxml2-dev libxml++2.6-dev libxml++2.6-doc -y
sudo apt-get install uuid-dev libaio-dev cmake -y
cd /home/ && wget https://github.com/zyxyoshine/IOStormplus/raw/master/deploy/binary/Agent.linux.zip
cd /home/ && unzip Agent.linux.zip
mkdir /home/IOStormplus/output
mkdir /home/IOStormplus/workload
ln -s /home/IOStormplus/libazurestorage.so.3.1 /usr/lib/libazurestorage.so.3
ln -s /home/IOStormplus/libcpprest.so.2.9 /usr/lib/libcpprest.so.2.9
chmod -R 0777 /home/IOStormplus
cd /home && wget http://brick.kernel.dk/snaps/fio-3.5.tar.bz2
cd /home && tar -xjvf fio-3.5.tar.bz2
cd /home && ./fio-3.5/configure
cd /home/fio-3.5 && make
cd /home/fio-3.5 && make install
sed -i '$inohup /home/IOStormplus/agent '$(hostname --ip-address)' '$2' '$3' \"'$1'\" >/dev/null 2>agent.err &' /etc/rc.local
cd /home/IOStormplus && nohup ./agent $(hostname --ip-address) $2 $3 $1 >/dev/null 2>agent.err &
#create one volume striped over all data disks
disks=($(lsblk -l -p -o NAME | grep "sd" | grep -v "sda" | grep -v "sdb"))
for disk in ${disks[*]}
do
       echo -e "n\np\n1\n\n\nt\nfd\nw" | fdisk $disk
done
pdisks=($(lsblk -l -p -o NAME | grep "sd" | grep -v "sda" | grep -v "sdb" | grep 1))
mdadm --create /dev/md0 --level 0 --raid-devices ${#pdisks[*]} ${pdisks[*]} --force
mkfs -t ext4 /dev/md0
mkdir /data
chmod -R 777 /data
echo -e "/dev/md0\t/data\text4\tdefaults\t0\t2" >> /etc/fstab
mount -a