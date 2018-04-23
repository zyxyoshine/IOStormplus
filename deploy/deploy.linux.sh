sudo -s
apt-get update
apt-get upgrade -y
wget http://brick.kernel.dk/snaps/fio-3.5.tar.bz2
tar -xjvf fio-3.5.tar.bz2
apt-get install make
apt-get install gcc -y
apt-get install g++ -y
./fio-3.5/configure
cd ~/fio-3.5 && make
cd ~/fio-3.5 && make install
apt-get install -y samba samba-common python-glade2 system-config-samba
rm -f /etc/samba/smb.conf
cd /etc/samba && wget https://raw.githubusercontent.com/zyxyoshine/IOStromplus/master/deploy/smb.conf
mkdir -p /samba/workload
mkdir -p /samba/temp
mkdir -p /samba/output
chmod -R 0777 /samba/workload
chmod -R 0777 /samba/temp
chmod -R 0777 /samba/output
chown -R nobody:nogroup /samba/workload
chown -R nobody:nogroup /samba/temp
chown -R nobody:nogroup /samba/output
cat /dev/null  > /samba/temp/controller.tmp
chmod -R 0777 /samba/temp/controller.tmp
chown -R nobody:nogroup /samba/temp/controller.tmp
service smbd restart
mkdir ~/fiojob
cd ~/fiojob && wget https://raw.githubusercontent.com/zyxyoshine/IOStromplus/master/src/agent.linux.cpp
g++ -std=c++11 agent.linux.cpp -o agent