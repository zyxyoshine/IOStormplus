$Root = "C:\"

#Download and unzip controller package

$PackageName = "Controller.zip"
$PackageUrl = "https://github.com/zyxyoshine/IOStromplus/raw/master/deploy/binary/Controller.zip"
[Net.ServicePointManager]::SecurityProtocol = "tls12, tls11, tls"
Invoke-WebRequest -Uri $PackageUrl -OutFile ($Root + $PackageName)

Add-Type -AssemblyName System.IO.Compression.FileSystem
function Unzip
{
    param([string]$zipfile, [string]$outpath)

    [System.IO.Compression.ZipFile]::ExtractToDirectory($zipfile, $outpath)
}

Unzip ($Root + $PackageName) $Root
