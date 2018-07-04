$Root = "C:\"
$Workspace = $Root + "IOStormplus\"
$ShareName = "agents"
$SharePath = $Workspace + $ShareName

#Download and unzip controller package

$PackageName = "Controller.zip"
$PackageUrl = "https://github.com/zyxyoshine/IOStormplus/raw/master/deploy/binary/Controller.zip"
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

if( -Not (Get-SMBShare -Name $ShareName -ea 0)){
    New-SmbShare -Name $ShareName -Path $SharePath -FullAccess Everyone
}

#Enable PSRemoting
winrm quickconfig -q
winrm s winrm/config/client '@{TrustedHosts="*"}'
