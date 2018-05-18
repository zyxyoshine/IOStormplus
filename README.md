# IOStormplus

## Deploy IOStormplus
------
### 1.Deploy controller VM and some resources.
Open Azure portal, and deploy template `deploy/controller.template.json`.<br>
Note: You need to create a new resource group for IOStormplus.
### 2.Deploy test VM.
IOStormplus support both windows and linux test VM. <br>
For linux,deploy `deploy/agent.template.linux.json`. <br>
For Windows,deploy `deploy/agent.template.win.json`. <br>
Note:<br>
1. Make sure the controller IP is correct.<br>
2. If you want to deploy several times (e.g deploy both linux and windows test VM), you need to configure `VM Index` to avoid index conflict.<br>
For example, at the first time you deploy 2 linux VM with the template (VM Index set to 0 by default), it will create testvm0 and testvm1. If you want to deploy 2 windows VM also, you need to set `VM Index` to 2 to make the VM as testvm2 and testvm3.
## Configure and run workload
-----
### 1.Initialize controller and import test VM infomation into controller.
Remote to controller VM (username:vmadmin,password:!!!!1234abcd) and open a console then navigate to `C:\IOStormplus`. <br>
If you have deployed windows test VM, you need to configure and start agent manually. (Will improve soon).
Run script for each windows test VM.
```PowerShell
PS C:\IOStormplus> .\StartWindowsAgent.ps1 {test VM IP}
```
Wait about 2 minutes after command done, when it has been done, you will find a file is named as testvm{id} in the `agents` folder.
After all agents have been set up,run command
```PowerShell
IOStormplus init
```
### 2.Start work.
IOStormplus is supported to run standard fio job (e.g.https://github.com/axboe/fio/tree/master/examples ). <br>
We have created the rand-RW job as standard workload. <br>
You can create your own jobs and upload them into `IOStormplus\workload\{OS}`. <br>
It's very easy to start work by run command
```PowerShell
IOStormplus start
```
or
```PowerShell
IOStormplus start -std
```
## Get result
------------------------------------------------------------------------------------------------------
When all jobs have been done (the standard job will cost about 6 minutes)
, all detail reports will be named as `testvm{id}_{job name}.out` and put into `output` folder. <br>
IOStormplus will create a summary report `{date}_summary.out` by analyse IOPS data, it can be direct import into Excel.
