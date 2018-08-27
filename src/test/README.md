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
```
### Workload
IOStormplus is supported to run standard fio job, and create a summary report by analyse data from each job result.
```
Test case 2-1
Steps:
1. Start standard test on controller by run C:\IOStormplus\IOStormplus_Controller.exe start -std
2. Compare detail reports and summary data.
Check point:
1. No agent / controller crash.
2. All logs and results store in blob.
3. The summary data is correct.
```
```
Test case 2-2
Steps:
1. Setup a custom job target data disk pool.
2. Start custom test on controller by run C:\IOStormplus\IOStormplus_Controller.exe start
Check point:
The same as case 2-1.
```
### Error Handling
IOStormplus should handle agents/controller crash/VM down, Azure storage not work / incorrect and incorrect job file.
```
Test case 3-1
Steps:
1. Start standard test on controller by run C:\IOStormplus\IOStormplus_Controller.exe start -std
2. Restart some test VMs.
Check point:
1. Controller resend commands to the restarted VMs, and get correct result.
```
```
Test case 3-2
Steps:
1. Start standard test on controller by run C:\IOStormplus\IOStormplus_Controller.exe start -std
2. Kill agent process on some VMs.
3. Run show command on controller.
Check point:
1. Controller print time out warning for those VMs.
2. Controller can track those VMs(do not display them)
```
```
Test case 3-3
Steps:
1. Setup incorrect storage connect key / endpoint on controller & VMs.
2. Run agents & controller.
Check point:
1. Agents and controller print correct error messages.
```
```
Test case 3-4
Steps:
1. Setup an incorrect job name in workload.json.
2. Run agents & controller.
Check point:
1. Agents and controller print correct error messages.
```
```
Test case 3-5
Steps:
1. Setup an incorrect job file.(eg. add ioengine=errtest)
Check point:
1. Agents print correct error messages in output file.
2. Controller print N/A on summary.
```