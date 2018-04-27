$Root = "C:\IOStormplus\"
$Workload = $Root + "workload"
$output = $Root + "output"

#Create directories
if ( -Not (Test-Path -Path $Root))
{
    New-Item -ItemType directory -Path $Root
    if ( -Not (Test-Path -Path $Workload))
    {
        New-Item -ItemType directory -Path $Workload
    }
    if ( -Not (Test-Path -Path $output))
    {
        New-Item -ItemType directory -Path $output
    }
}

#Download agent
$AgentBinaryName = "fio-3.5-x64.msi"
$AgentUrl = "https://github.com/zyxyoshine/IOStromplus/raw/master/deploy/binary/fio-3.5-x64.msi"
Invoke-WebRequest -Uri $AgentUrl -OutFile $Root + $AgentBinaryName


#Download and install fio
$FioBinaryName = "fio-3.5-x64.msi"
$FioUrl = "https://github.com/zyxyoshine/IOStromplus/raw/master/deploy/binary/fio-3.5-x64.msi"
Invoke-WebRequest -Uri $FioUrl -OutFile $Root + $FioBinaryName

$DataStamp = get-date -Format yyyyMMdd
$logFile = '{0}-{1}.log' -f $Root + $FioBinaryName,$DataStamp
$FioMSIArguments = @(
    "/i"
    ('"{0}"' -f $Root + $FioBinaryName)
    "/qn"
    "/norestart"
    "/L*v"
    $logFile
)
Start-Process "msiexec.exe" -ArgumentList $FioMSIArguments -Wait -NoNewWindow

#Create shares


