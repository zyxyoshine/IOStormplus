apt-get update
apt-get upgrade -y

#install libraries 
apt-get install make gcc g++ unzip zlib1g-dev libboost-all-dev libssl-dev libxml2-dev libxml++2.6-dev libxml++2.6-doc uuid-dev libaio-dev cmake -y

#install fio
cd /home && wget http://brick.kernel.dk/snaps/fio-3.5.tar.bz2
cd /home && tar -xjvf fio-3.5.tar.bz2
cd /home && ./fio-3.5/configure
cd /home/fio-3.5 && make
cd /home/fio-3.5 && make install

#configure python libs
apt-get install python3-pip -y 
pip3 install azure
pip3 install azure-storage 

#install IOStorm agent
cd /home/ 
mkdir IOStorm
cd IOStorm
wget https://raw.githubusercontent.com/zyxyoshine/IOStormplus/master/agent/IOStormAgent.py
mkdir /home/IOStorm/output
mkdir /home/IOStorm/workload

#configure agent
#create config.yml file 


#start agent
#sed -i '$inohup /home/IOStormplus/agent '$(hostname --ip-address)' '$2' '$3' \"'$1'\" >/dev/null 2>agent.err &' /etc/rc.local
#cd /home/IOStormplus && nohup ./agent $(hostname --ip-address) $2 $3 $1 >/dev/null 2>agent.err &

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