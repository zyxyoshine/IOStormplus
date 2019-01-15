$logFile = 'C:\deploylog.txt'
Start-Transcript $logFile -Append -Force

$Root = "C:\"
$WorkspacePath = $Root + "IOStormplus\"

#Download and unzip agent package

$PackageName = "Agent.win.zip"
$PackageUrl = "https://github.com/zyxyoshine/IOStormplus/raw/master/deploy/binary/Agent.win.zip"
[Net.ServicePointManager]::SecurityProtocol = "tls12, tls11, tls"
Invoke-WebRequest -Uri $PackageUrl -OutFile ($Root + $PackageName)

Add-Type -AssemblyName System.IO.Compression.FileSystem
function Unzip
{
    param([string]$zipfile, [string]$outpath)

    [System.IO.Compression.ZipFile]::ExtractToDirectory($zipfile, $outpath)
}

Unzip ($Root + $PackageName) $Root
Remove-Item ($Root + $PackageName)

#Install fio

$FioBinaryName = "fio-3.5-x64.msi"
$DataStamp = get-date -Format yyyyMMdd
$logFile = '{0}-{1}.log' -f ($WorkspacePath + $FioBinaryName),$DataStamp
$FioMSIArguments = @(
    "/i"
    ('"{0}"' -f ($WorkspacePath + $FioBinaryName))
    "/qn"
    "/norestart"
    "/L*v"
    $logFile
)
Start-Process "msiexec.exe" -ArgumentList $FioMSIArguments -Wait -NoNewWindow
Remove-Item ($WorkspacePath + $FioBinaryName)

#Initilize data disks
$disks = Get-Disk | Where partitionstyle -eq 'raw' | sort number

$letters = 70..89 | ForEach-Object { [char]$_ }
$count = 0
$label = "data"

foreach ($disk in $disks) {
    $driveLetter = $letters[$count].ToString()
    $disk |
    Initialize-Disk -PartitionStyle MBR -PassThru |
    New-Partition -UseMaximumSize -DriveLetter $driveLetter |
    Format-Volume -FileSystem NTFS -NewFileSystemLabel ($label + $count) -Confirm:$false -Force -AllocationUnitSize 65536
    $count++
}

#Create Azure Storage connection string
$storageAccountName = 'AccountName=' + $args[0] + ';'
$storageAccountKey = 'AccountKey=' + $args[1] + ';'
$storageEndpointSuffix = 'EndpointSuffix=' + $args[2]
$storageConnectionString = 'DefaultEndpointsProtocol=https;' + $storageAccountName + $storageAccountKey + $storageEndpointSuffix

#Start Agent
netsh advfirewall set privateprofile state off
netsh advfirewall set publicprofile state off

$VMSize = $args[3]
$VMPool = $args[4]
$VMSize | Out-File ($WorkspacePath + 'vmsize.txt')
$VMIp = (Get-NetIPAddress -AddressFamily IPv4 | where { $_.InterfaceAlias -notmatch 'Loopback'} | where { $_.InterfaceAlias -notmatch 'vEthernet'}).IPAddress   
$agentName = "agent.exe"
$agentPath = $WorkspacePath + $agentName
$args = ' ' + $VMIp + ' ' + $VMSize + ' ' + $VMPool + ' ' + $storageConnectionString
$action = New-ScheduledTaskAction -Execute $agentPath -Argument $args -WorkingDirectory $WorkspacePath
$trigger = @()
$trigger += New-ScheduledTaskTrigger -Once -At (Get-Date).AddMinutes(1)
$trigger += New-ScheduledTaskTrigger -AtStartup
$settings = New-ScheduledTaskSettingsSet -StartWhenAvailable -RunOnlyIfNetworkAvailable -DontStopOnIdleEnd
Unregister-ScheduledTask -TaskName "VMIOSTORM" -Confirm:0 -ErrorAction Ignore
Register-ScheduledTask -Action $action -Trigger $trigger -TaskName "VMIOSTORM" -Description "VM iostorm agent" -User "System" -RunLevel Highest -Settings $settings

Stop-Transcript

#Enable PSRemoting

$DNSName = $env:COMPUTERNAME 
Enable-PSRemoting -Force   

New-NetFirewallRule -Name "WinRM HTTPS" -DisplayName "WinRM HTTPS" -Enabled True -Profile "Any" -Action "Allow" -Direction "Inbound" -LocalPort 5986 -Protocol "TCP"    

$thumbprint = (New-SelfSignedCertificate -DnsName $DNSName -CertStoreLocation Cert:\LocalMachine\My).Thumbprint   
$cmd = "winrm create winrm/config/Listener?Address=*+Transport=HTTPS @{Hostname=""$DNSName""; CertificateThumbprint=""$thumbprint""}" 

cmd.exe /C $cmd  
