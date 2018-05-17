# IOStromplus
Deploy IOStrom
------------------------------------------------------------------------------------------------------
1.Deploy controller VM and some share resources.
Import deploy/agent.template.controller.json into Azure portal with a new resource group.

2.Deploy test VM for workloads.
IOStromplus support both windows and linux test VM.
For linux,
Import deploy/agent.template.linux.json into controller's RG.
For Windows,
Import deploy/agent.template.win.json into controller's RG.

Configure and run workload
------------------------------------------------------------------------------------------------------
1.Initialize controller and import test VM infomation into controller.
Remote to controller VM and navigate to C:\IOStormplus
Run command "IOStormplus init"

2.Start work.
IOStromplus is supported to run standard fio job (e.g. https://github.com/axboe/fio/tree/master/examples)
We have created the rand-RW job as standard workload.
You can create your own jobs and put them into IOStormplus\workload\{OS} , or change the standard workload by put them into IOStormplus\workload\std\{OS}.

It's very easy to start work by run command "IOStormplus start" or "IOStormplus start -std"

Get result
------------------------------------------------------------------------------------------------------
When all jobs have been done, all 
