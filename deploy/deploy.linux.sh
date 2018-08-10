apt-get update
apt-get upgrade -y
apt-get install make gcc g++ unzip zlib1g-dev libboost-all-dev libssl-dev libxml2-dev libxml++2.6-dev libxml++2.6-doc uuid-dev libaio-dev cmake -y
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
cat /dev/null > /etc/profile.d/IOStormplus.sh
echo "nohup /home/IOStormplus/agent $(hostname --ip-address) $2 $3 \"$1\" >/dev/null 2>agent.err &" > /etc/profile.d/IOStormplus.sh
cd /home/IOStormplus && nohup ./agent $(hostname --ip-address) $2 $3 $1 >/dev/null 2>agent.err &
#create one volume striped over all data disks
disks=($(lsblk -l -p -o NAME | grep "sd" | grep -v "sda" | grep -v "sdb"))
pvcreate ${disks[*]}
vgcreate datavg ${disks[*]}
lvcreate -l 100%FREE --type striped -i ${#disks[*]} -n datav datavg
mkfs -t ext4 /dev/mapper/datavg-datav
mkdir /data
mount -t ext4 /dev/mapper/datavg-datav  /data
chmod 777 /data
