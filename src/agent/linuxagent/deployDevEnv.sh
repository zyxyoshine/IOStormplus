### TEST ###
apt-get update
apt-get upgrade -y
apt-get install make gcc g++ unzip zlib1g-dev libboost-all-dev libssl-dev libxml2-dev uuid-dev cmake -y
cd /home/ && wget https://github.com/zyxyoshine/IOStormplus/raw/master/deploy/binary/Agent.linux.zip
cd /home/ && unzip Agent.linux.zip
ln -s /home/IOStormplus/libazurestorage.so.5.0 /usr/lib/libazurestorage.so.5
ln -s /home/IOStormplus/libazurestorage.so.5.0 /usr/local/lib/libazurestorage.so.5.0
ln -s /usr/local/lib/libazurestorage.so.5.0 /usr/local/lib/libazurestorage.so.5
ln -s /usr/local/lib/libazurestorage.so.5 /usr/local/lib/libazurestorage.so
ln -s /home/IOStormplus/libcpprest.so.2.9 /usr/lib/libcpprest.so.2.9
ln -s /home/IOStormplus/libcpprest.so.2.9 /usr/local/lib/libcpprest.so.2.9
ln -s /usr/local/lib/libcpprest.so.2.9 /usr/local/lib/libcpprest.so
chmod -R 0777 /home/IOStormplus
cd /home && wget http://brick.kernel.dk/snaps/fio-3.5.tar.bz2
cd /home && tar -xjvf fio-3.5.tar.bz2
./fio-3.5/configure
cd /home/fio-3.5 && make
cd /home/fio-3.5 && make install
