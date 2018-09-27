$Root = "C:\"
$Workspace = $Root + "IOStormplus\"

#Download and unzip controller package

$PackageName = "Controller.zip"
$PackageUrl = "https://github.com/zyxyoshine/IOStormplus/raw/master/deploy/binary/Controller.zip"
[Net.ServicePointManager]::SecurityProtocol = "tls12, tls11, tls"

Invoke-WebRequest -Uri $PackageUrl -OutFile ($Root + $PackageName)
Expand-Archive ($Root + $PackageName) -DestinationPath $Root
Remove-Item ($Root + $PackageName)

#Create Azure Storage configuration file
$storageConfigFileName = "AzureStorage.config"
$storageAccountBuf = 'NAME=' + $args[0]
$storageAccountKeyBuf = 'KEY=' + $args[1]
$storageEndpointSuffixBuf = 'ENDPOINTSUF=' + $args[2]
($storageAccountBuf + [Environment]::NewLine + $storageAccountKeyBuf + [Environment]::NewLine + $storageEndpointSuffixBuf) |  Out-File -encoding ASCII ($Workspace + $storageConfigFileName)

# Install AzureRM 
Install-PackageProvider -Name NuGet -MinimumVersion 2.8.5.201 -Force
Set-PSRepository -Name "PSGallery" -InstallationPolicy Trusted
Install-Module -Name AzureRm.BootStrapper -Force  
Use-AzureRmProfile -Profile 2017-03-09-profile -Force

#Install SSH client
Add-WindowsCapability -Online -Name OpenSSH.Client~~~~0.0.1.0

#Enable PSRemoting

$DNSName = $env:COMPUTERNAME 
Enable-PSRemoting -Force   

New-NetFirewallRule -Name "WinRM HTTPS" -DisplayName "WinRM HTTPS" -Enabled True -Profile "Any" -Action "Allow" -Direction "Inbound" -LocalPort 5986 -Protocol "TCP"    

$thumbprint = (New-SelfSignedCertificate -DnsName $DNSName -CertStoreLocation Cert:\LocalMachine\My).Thumbprint   
$cmd = "winrm create winrm/config/Listener?Address=*+Transport=HTTPS @{Hostname=""$DNSName""; CertificateThumbprint=""$thumbprint""}" 

cmd.exe /C $cmd  

