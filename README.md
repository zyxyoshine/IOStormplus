# IOStormplus

## Deploy IOStormplus
### 1.Deploy controller VM and some resources.
Open Azure portal, and deploy template `deploy/controller.template.json`.<br>
Note: You need to create a new resource group for IOStormplus.
### 2.Deploy test VM.
IOStormplus support both windows and linux test VM. <br>
For linux,deploy `deploy/agent.template/agent.template.linux.json`. <br>
For Windows,deploy `deploy/agent.template/agent.template.win.json`. <br>
## Configure and run workload
### 1.Initialize controller and import test VM infomation into controller.
Remote to controller VM and open a console then navigate to `C:\IOStormplus`. <br>
After all agents have been set up,run command
```PowerShell
IOStormplus_Controller init
```

### 2.Start work.
IOStormplus is supported to run standard fio job (e.g.https://github.com/axboe/fio/tree/master/examples ). <br>
We have created the rand-RW job as standard workload. <br>
You can create your own jobs and upload them into `IOStormplus\workload\` and specify a VM pool to run it by edit `workload.json`.
It's very easy to start work by run command
```PowerShell
IOStormplus_Controller start
```
or 
```PowerShell
IOStormplus_Controller start -std
```
NOTE: Standard test will ignore all custom pool settings and run `std` workload.<br>
NOTE2: If you don't specify any workload for a pool, it will run `std` workload.
### 3.Other features
1. Display test vm status.
```PowerShell
IOStormplus_Controller init
```
or 
```PowerShell
IOStormplus_Controller agent show
```
## Get result
When all jobs have been done (the standard job will cost about 6 minutes)
, all detail reports will be named as `testvm{id}_{job name}.out` and put into `output` folder and blob. <br>
IOStormplus will create a summary report `{date}_summary.out` by analyse IOPS data, it can be direct import into Excel.
