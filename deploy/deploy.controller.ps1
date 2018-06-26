$Root = "C:\"
$Workspace = $Root + "IOStormplus\"
$ShareName = "agents"
$SharePath = $Workspace + $ShareName

#Download and unzip controller package

$PackageName = "Controller.zip"
$PackageUrl = "https://github.com/zyxyoshine/IOStormplus/raw/master/deploy/binary/Controller.zip"
[Net.ServicePointManager]::SecurityProtocol = "tls12, tls11, tls"

Invoke-WebRequest -Uri $PackageUrl -OutFile ($Root + $PackageName)
Expand-Archive ($Root + $PackageName) -DestinationPath $Root
Remove-Item ($Root + $PackageName)


# Install AzureRM 
Install-PackageProvider -Name NuGet -MinimumVersion 2.8.5.201 -Force -Scope CurrentUser
Set-PSRepository -Name "PSGallery" -InstallationPolicy Trusted
Install-Module -Name AzureRm.BootStrapper -Force -Scope CurrentUser 
Use-AzureRmProfile -Profile 2017-03-09-profile -Force -Scope CurrentUser


#Enable PSRemoting

#winrm quickconfig -q
#winrm s winrm/config/client '@{TrustedHosts="*"}'

$DNSName = $env:COMPUTERNAME 
Enable-PSRemoting -Force   

New-NetFirewallRule -Name "WinRM HTTPS" -DisplayName "WinRM HTTPS" -Enabled True -Profile "Any" -Action "Allow" -Direction "Inbound" -LocalPort 5986 -Protocol "TCP"    

$thumbprint = (New-SelfSignedCertificate -DnsName $DNSName -CertStoreLocation Cert:\LocalMachine\My).Thumbprint   
$cmd = "winrm create winrm/config/Listener?Address=*+Transport=HTTPS @{Hostname=""$DNSName""; CertificateThumbprint=""$thumbprint""}" 

cmd.exe /C $cmd  



