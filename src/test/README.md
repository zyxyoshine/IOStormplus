# Test cases

## Basic Scenario

### Deployment
User can deploy IOStormplus by run script `deploy/deploy*.ps1` or import template `deploy/umd/*` on portal.
```
Test case 1-1
Steps:
1. Use deploy/umd/controller.template.json to deploy controller.
2. Use deploy/umd/agent.template/agent.template.linux.json to deploy linux VM pools contain 1/2/4 VMs with 4/64/128/256 GB disk size and 1/4/8/16 disk count.
3. Deploy deploy/umd/agent.template/agent.template.win.json with the same setting as 2.
Check point:
1. Deployment status is 'Succeeded' on portal.
2. Login controller VM and run C:\IOStormplus\IOStormplus_Controller.exe test, the test reslut should be 'succeeded'.
```
```
Test case 1-2
Run deploy/deploycontroller.ps1 & deploy/deploypool.ps1 to do the same things as case 1-1.
```
```
Test case 1-3
Steps:
1. Run deploy/stoppool.ps1 to stop VMs in a pool.
2. Run deploy/startpool.ps1 to restart the pool.
Check point:
Run C:\IOStormplus\IOStormplus_Controller.exe show, the reslut should include the restarted pool.
```
```
Test case 1-4
Steps:
1. Run deploy/getpool.ps1 to display all VMs in a pool.
2. Run deploy/scalepool.ps1 to add/remove(eg.1/-1) VM in a pool.
3. Run deploy/removepool.ps1 to remove a pool.
Check point:
No script throw exception, VMs can be tracked on controller by 'show' command.

### Workload
IOStormplus is supported to run standard fio job, and create a summary report by analyse data from each job result.
```
Test case 2-1
Steps:
1. Start standard test on controller by run C:\IOStormplus\IOStormplus_Controller.exe stard -std
2. Compare detail reports and summary data.
Check point:
1. No agent / controller crash.
2. All logs and results store in blob.
3. The summary data is correct.
```

1. Run standard test by IOStormplus_Controller.exe test
### Error Handling

1. Run script 
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
2. If you just want to dev with current library, you can just run script `/agent/linuxagent/deployDevEnv.sh` to deploy environment.

## Build
For controller and Windows agent, build VS solution target x64 platform.  </br>
For Linux agent, run command `make` at `linuxagent` floder. </br>
Note: we only tested build Linux agent with g++ 5.4.0 .

## Deploy and test
1. Compress your executable file and libraries to a zip file (For example, `IOStormplus\deploy\binary\*.zip`). </br>
2. Upload zip file. </br>
3. If you changed file or floder path/name/struct, please don't forget to change deploy scripts and templates. </br>
4. Use templates or scripts to deploy and test your work.
