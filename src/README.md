# Architecture
## Overall
![image](https://github.com/zyxyoshine/IOStormplus/raw/master/src/diagram/architecture_overall.jpg)
## Agent
![image](https://github.com/zyxyoshine/IOStormplus/raw/master/src/diagram/architecture_agent.jpg)
# Development

## Deploy environment

### 1.Windows environment
Controller and Windows agent are implemented by Microsoft Visual Studio 2017 with Windows SDK 10.0.17134.0. </br>
You can install them by Visual Studio Installer.</br>
All packages can be restored from nuget.org.

### 2.Linux environment
You should deploy linux environment (Ubuntu 16.04 LTS) for implement linux agent. </br>
1. If you want to try new Azure Storage Client please follow https://github.com/Azure/azure-storage-cpp and https://github.com/Microsoft/cpprestsdk/wiki/How-to-build-for-Linux to build `libcpprest.so` and `libazurestorage.so`, then create soft links to /usr/lib/ like:
``` shell
ln -s /home/IOStormplus/libazurestorage.so.5.0 /usr/lib/libazurestorage.so.5
ln -s /home/IOStormplus/libcpprest.so.2.9 /usr/lib/libcpprest.so.2.9
```
Install fio:
``` shell
cd /home && wget http://brick.kernel.dk/snaps/fio-3.5.tar.bz2
tar -xjvf fio-3.5.tar.bz2
./fio-3.5/configure
cd /home/fio-3.5 && make
make install
```
2. If you just want to dev with current library (was 3.1.0), you can just run script `/agent/linuxagent/deployDevEnv.sh` to deploy environment.</br>
NOTE: Azure Stack only supports 3.1.0.
## Build
For controller and Windows agent, build VS solution target x64 platform.  </br>
For Linux agent, run command `make` at `linuxagent` floder. </br>
Note: we only tested build Linux agent with g++ 5.4.0 .

## Deploy and test
1. Compress your executable file and libraries to a zip file (For example, `IOStormplus\deploy\binary\*.zip`). </br>
2. Upload zip file. </br>
3. If you changed file or floder path/name/struct, please don't forget to change deploy scripts and templates. </br>
4. Use templates or scripts to deploy and test your work.
