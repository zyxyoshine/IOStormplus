$Root = "C:\IOStormplus\"
$WorkloadShare = $Root + "workload"
$OutputShare = $Root + "output"
$TempShare = $Root + "temp"

#Create directories
if ( -Not (Test-Path -Path $Root))
{
    New-Item -ItemType directory -Path $Root
    if ( -Not (Test-Path -Path $WorkloadShare))
    {
        New-Item -ItemType directory -Path $WorkloadShare
    }
    if ( -Not (Test-Path -Path $OutputShare))
    {
        New-Item -ItemType directory -Path $OutputShare
    }
    if ( -Not (Test-Path -Path $TempShare))
    {
        New-Item -ItemType directory -Path $TempShare
    }
}

if(!(Get-SMBShare -Name "workload" -ea 0)){
    New-SmbShare -Name "workload" -Path $WorkloadShare -FullAccess Everyone
}
if(!(Get-SMBShare -Name "output" -ea 0)){
    New-SmbShare -Name "output" -Path $OutputShare -FullAccess Everyone
}
if(!(Get-SMBShare -Name "temp" -ea 0)){
    New-SmbShare -Name "temp" -Path $TempShare -FullAccess Everyone
}

#Download agent
$AgentBinaryName = "agent.exe"
$AgentUrl = "https://github.com/zyxyoshine/IOStromplus/raw/master/deploy/binary/agent.exe"
[Net.ServicePointManager]::SecurityProtocol = "tls12, tls11, tls"
Invoke-WebRequest -Uri $AgentUrl -OutFile ($Root + $AgentBinaryName)


#Download and install fio
$FioBinaryName = "fio-3.5-x64.msi"
$FioUrl = "https://github.com/zyxyoshine/IOStromplus/raw/master/deploy/binary/fio-3.5-x64.msi"
Invoke-WebRequest -Uri $FioUrl -OutFile ($Root + $FioBinaryName)

$DataStamp = get-date -Format yyyyMMdd
$logFile = '{0}-{1}.log' -f ($Root + $FioBinaryName),$DataStamp
$FioMSIArguments = @(
    "/i"
    ('"{0}"' -f ($Root + $FioBinaryName))
    "/qn"
    "/norestart"
    "/L*v"
    $logFile
)
Start-Process "msiexec.exe" -ArgumentList $FioMSIArguments -Wait -NoNewWindow



