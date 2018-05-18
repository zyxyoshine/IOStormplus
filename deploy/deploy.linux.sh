apt-get update
apt-get upgrade -y
apt-get install make
apt-get install gcc -y
apt-get install g++ -y
apt-get install smbclient
cd /home/vmroot && wget http://brick.kernel.dk/snaps/fio-3.5.tar.bz2
cd /home/vmroot && tar -xjvf fio-3.5.tar.bz2
./fio-3.5/configure
cd /home/vmroot/fio-3.5 && make
cd /home/vmroot/fio-3.5 && make install
apt-get install -y samba samba-common python-glade2 system-config-samba
rm -f /etc/samba/smb.conf
cd /etc/samba && wget https://raw.githubusercontent.com/zyxyoshine/IOStromplus/master/deploy/smb.conf
mkdir -p /samba/workload
mkdir -p /samba/temp
mkdir -p /samba/output
mkdir -p /samba/info
mount -t cifs -o username=vmadmin,password='!!!!1234abcd' //$1/agents /samba/info
chmod -R 0777 /samba/workload
chmod -R 0777 /samba/temp
chmod -R 0777 /samba/output
chown -R nobody:nogroup /samba/workload
chown -R nobody:nogroup /samba/temp
chown -R nobody:nogroup /samba/output
cat /dev/null  > /samba/temp/controller.tmp
cat /dev/null  > /samba/temp/client.tmp
chmod -R 0777 /samba/temp/controller.tmp
chmod -R 0777 /samba/temp/client.tmp
chown -R nobody:nogroup /samba/temp/controller.tmp
service smbd restart
mkdir /home/vmroot/fiojob
cd /home/vmroot/fiojob && wget https://raw.githubusercontent.com/zyxyoshine/IOStromplus/master/src/agent.linux.cpp
cd /home/vmroot/fiojob && g++ -std=c++11 agent.linux.cpp -o agent
chmod 0777 /home/vmroot/fiojob/agent
nohup ./agent $(hostname --ip-address) $2 >agent.log 2>agent.err &
