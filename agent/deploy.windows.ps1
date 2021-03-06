﻿$logFile = 'C:\deploylog.txt'
Start-Transcript $logFile -Append -Force

#Enable PSRemoting

$DNSName = $env:COMPUTERNAME 
Enable-PSRemoting -Force   

New-NetFirewallRule -Name "WinRM HTTPS" -DisplayName "WinRM HTTPS" -Enabled True -Profile "Any" -Action "Allow" -Direction "Inbound" -LocalPort 5986 -Protocol "TCP"    

$thumbprint = (New-SelfSignedCertificate -DnsName $DNSName -CertStoreLocation Cert:\LocalMachine\My).Thumbprint   
$cmd = "winrm create winrm/config/Listener?Address=*+Transport=HTTPS @{Hostname=""$DNSName""; CertificateThumbprint=""$thumbprint""}" 

cmd.exe /C $cmd  

$Root = "C:\"
$Directory = "IOStorm\"
$WorkspacePath = $Root + $Directory
md $WorkspacePath

[Net.ServicePointManager]::SecurityProtocol = "tls12, tls11, tls"


#Install fio
$PackageName = $WorkspacePath + "fio-3.9-x64.msi"
$PackageUrl  = "https://bluestop.org/files/fio/releases/fio-3.9-x64.msi"

Invoke-WebRequest -Uri $PackageUrl -OutFile $PackageName
$DataStamp = get-date -Format yyyyMMdd
$logFile = '{0}-{1}.log' -f $PackageName, $DataStamp
$FioMSIArguments = @(
    "/i"
    ('"{0}"' -f $PackageName)
    "/qn"
    "/norestart"
    "/L*v"
    $logFile
)
Start-Process "msiexec.exe" -ArgumentList $FioMSIArguments -Wait -NoNewWindow
Remove-Item ($PackageName)

#Create storage space over data disks
$poolname = "datapool"
$vdname = "datavd"
$disks = Get-PhysicalDisk | where BusType -eq "SAS"
$storage = Get-StorageSubSystem
$pool = New-StoragePool -FriendlyName $poolname -PhysicalDisks $disks -StorageSubSystemName $storage.Name
$vdisk = New-VirtualDisk -FriendlyName $vdname `
                -ResiliencySettingName Simple `
                -NumberOfColumns $Disks.Count `
                -UseMaximumSize -Interleave 65536 -StoragePoolFriendlyName $poolname
$vdiskNumber = (Get-disk -FriendlyName $vdname).Number
Initialize-Disk -FriendlyName $vdname -PartitionStyle GPT -PassThru
$partition = New-Partition -UseMaximumSize -DiskNumber $vdiskNumber -DriveLetter X
Format-Volume -DriveLetter X -FileSystem NTFS -NewFileSystemLabel "data" -AllocationUnitSize 65536

#install python 
Set-ExecutionPolicy Bypass -Scope Process -Force; iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
choco install python -y

#configure python libs
C:\Python37\Scripts\pip install azure 
C:\Python37\Scripts\pip install azure-storage 
C:\Python37\Scripts\pip install pyyaml

#install IOStorm agent
$PackageName = "IOStormAgent.py"
$PackageUrl = "https://raw.githubusercontent.com/zyxyoshine/IOStormplus/master/agent/IOStormAgent.py"
Invoke-WebRequest -Uri $PackageUrl -OutFile ($WorkspacePath + $PackageName)
md ($WorkspacePath + "\output")
md ($WorkspacePath + "\workload")

#Configure Agent
$AccName = $args[0]
$AccKey  = $args[1]
$AccEP   = $args[2]
$VMPool = $args[3]
$VMOS   = $args[4]
$VMSize = $args[5]
$VMDisks = $disks.Count
$VMDiskSize = ($disks[0].Size / 1GB)
$VMIp = (Get-NetIPAddress -AddressFamily IPv4 | where { $_.InterfaceAlias -notmatch 'Loopback'} | where { $_.InterfaceAlias -notmatch 'vEthernet'}).IPAddress   
$VMName = hostname
cd $WorkspacePath
C:\Python37\python.exe .\IOStormAgent.py config $AccName $AccKey $AccEP $VMPool $VMName $VMIP $VMOS $VMSize $VMDisks $VMDiskSize

#Schedule the agent 
$agentParams="/C C:\Python37\python .\IOStormAgent.py >console.log 2>agent.err "
$action = New-ScheduledTaskAction -Execute "cmd" -Argument $agentParams -WorkingDirectory $WorkspacePath
$trigger = @()
$trigger += New-ScheduledTaskTrigger -Once -At (Get-Date).AddMinutes(1)
$trigger += New-ScheduledTaskTrigger -AtStartup
$settings = New-ScheduledTaskSettingsSet -StartWhenAvailable -RunOnlyIfNetworkAvailable -DontStopOnIdleEnd
Unregister-ScheduledTask -TaskName "VMIOSTORM" -Confirm:0 -ErrorAction Ignore
Register-ScheduledTask -Action $action -Trigger $trigger -TaskName "VMIOSTORM" -Description "VM iostorm agent" -User "System" -RunLevel Highest -Settings $settings

Start-ScheduledTask -TaskName "VMIOSTORM"

Stop-Transcript

