apt-get update
apt-get upgrade -y
apt-get install make
apt-get install gcc -y
apt-get install g++ -y
apt-get install smbclient
apt-get install unzip -y
cd /home && wget https://github.com/zyxyoshine/IOStormplus/archive/master.zip
cd /home && unzip IOStormplus-master.zip
cd /home/IOStormplus-master/src/agent/linuxagent && make
mkdir /home/fiojob
cp -pf /home/IOStormplus-master/src/agent/linuxagent/agent /home/fiojob/
cd /home && wget http://brick.kernel.dk/snaps/fio-3.5.tar.bz2
cd /home && tar -xjvf fio-3.5.tar.bz2
./fio-3.5/configure
cd /home/fio-3.5 && make
cd /home/fio-3.5 && make install
apt-get install -y samba samba-common python-glade2 system-config-samba
rm -f /etc/samba/smb.conf
cd /etc/samba && wget https://raw.githubusercontent.com/zyxyoshine/IOStormplus/master/deploy/smb.conf
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
chmod 0777 /home/fiojob/agent
cd /home/fiojob && nohup ./agent $(hostname --ip-address) $2 >agent.log 2>agent.err &
