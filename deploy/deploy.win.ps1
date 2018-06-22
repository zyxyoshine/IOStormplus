#$logFile = 'C:\deploylog.txt'
#Start-Transcript $logFile -Append -Force

$Root = "C:\"
$WorkspacePath = $Root + "IOStormplus\"
$WorkloadSharePath = $WorkspacePath + "workload\"
$OutputSharePath = $WorkspacePath + "output\"
$TempSharePath = $WorkspacePath + "temp\"

#Download and unzip agent package

$PackageName = "Agent.zip"
$PackageUrl = "https://github.com/zyxyoshine/IOStormplus/raw/master/deploy/binary/Agent.zip"
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

#Create shares

if( -Not (Get-SMBShare -Name "workload" -ea 0)){
    New-SmbShare -Name "workload" -Path $WorkloadSharePath -FullAccess Everyone
}
if( -Not (Get-SMBShare -Name "output" -ea 0)){
    New-SmbShare -Name "output" -Path $OutputSharePath -FullAccess Everyone
}
if( -Not (Get-SMBShare -Name "temp" -ea 0)){
    New-SmbShare -Name "temp" -Path $TempSharePath -FullAccess Everyone
}

Out-File ($TempSharePath + "controller.tmp")
Out-File ($TempSharePath + "client.tmp")

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

#Initialize data disks
$disks = Get-Disk | Where partitionstyle -eq 'raw' | sort number

$letters = 70..89 | ForEach-Object { [char]$_ }
$count = 0
$label = "data"

foreach ($disk in $disks) {
    $driveLetter = $letters[$count].ToString()
    $disk |
    Initialize-Disk -PartitionStyle MBR -PassThru |
    New-Partition -UseMaximumSize -DriveLetter $driveLetter |
    Format-Volume -FileSystem NTFS -NewFileSystemLabel ($label + $count) -Confirm:$false -Force
    $count++
}

#Start Agent
netsh advfirewall set privateprofile state off
netsh advfirewall set publicprofile state off

$ControllerIP = $args[0]
$VMSize = $args[1]
$VMSize | Out-File ($WorkspacePath + 'vmsize.txt')
$VMIp = foreach($ip in (ipconfig) -like '*IPv4*') { ($ip -split ' : ')[-1]}
$username = 'vmadmin'
$password = '!!!!1234abcd'
$agentName = "agent.exe"
$agentPath = $WorkspacePath + $agentName
$args = ' ' + $ControllerIP + ' ' + $VMIp + ' ' + $VMSize
$action = New-ScheduledTaskAction -Execute $agentPath -Argument $args -WorkingDirectory $WorkspacePath
$trigger = @()
$trigger += New-ScheduledTaskTrigger -Once -At (Get-Date).AddMinutes(1)
$trigger += New-ScheduledTaskTrigger -AtStartup
$settings = New-ScheduledTaskSettingsSet -StartWhenAvailable # -RunOnlyIfNetworkAvailable -DontStopOnIdleEnd -Priority 4
Unregister-ScheduledTask -TaskName "VMIOSTORM" -Confirm:0 -ErrorAction Ignore
Register-ScheduledTask -Action $action -Trigger $trigger -TaskName "VMIOSTORM" -Description "VM iostorm agent" -User "System" -RunLevel Highest -Settings $settings

#Stop-Transcript

#Enable PSRemoting
winrm quickconfig -q
