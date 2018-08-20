# IOStormplus

## Deploy IOStormplus
### 1.Deploy controller VM and some resources.
Open Azure portal, and deploy template `deploy/umd/controller.template.json`.<br>
Note: You need to create a new resource group for IOStormplus.
### 2.Deploy test VM.
IOStormplus support both windows and linux test VM. <br>
For linux,deploy `deploy/umd/agent.template/agent.template.linux.json`. <br>
For Windows,deploy `deploy/umd/agent.template/agent.template.win.json`. <br>
We also support deploy and manage test VM by scripts: <br>
`deploy/deploycontroller.ps1` <br>
`deploy/deploypool.ps1` <br>
`deploy/getpool.ps1` <br>
`deploy/removepool.ps1` <br>
`deploy/scalepool.ps1` <br>
`deploy/startpool.ps1` <br>
`deploy/stoppool.ps1`
## Configure and run workload
### 1.Setup your workload.
IOStormplus is supported to run standard fio job (e.g.https://github.com/axboe/fio/tree/master/examples ). <br>
We have created the rand-RW job as standard workload. <br>
You can create your own jobs and upload them into `IOStormplus\workload\` and specify a VM pool to run it by edit `workload.json`.
``` JSON
{
	"count":2,
	"value":
		[
			{
				"pool":"std",
				"count":1,
				"jobs":
				[
					"fio-rand-RW.job"
				]
			},
			{
				"pool":"your_pool_A",
				"count":2,
				"jobs":
				[
					"your_job_1.job",
					"your_job_2.job"
				]
			}
		]
}
```
### 2.Start work.
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
1. Display alive test VM status.
```PowerShell
IOStormplus_Controller show
```
2. Test VM health by pre-sync test.
```PowerShell
IOStormplus_Controller test
```
## Get result
When all jobs have been done (the standard job will cost about 6 minutes)
, all detail reports will be named as `{timestamp}_{testvm}_{job name}.out` and put into `output` folder and blob. <br>
IOStormplus will create a summary report `{timestamp}_summary.out` by analyse IOPS data, it can be direct import into Excel.
