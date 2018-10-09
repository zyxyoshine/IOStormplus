Param(
    [string] $vmPool
)

$configfile = "config.json"
$config = get-content $configfile | ConvertFrom-Json
$rg = $config.resourcegroup
$acct = $config.storageaccount

#first check if pool with this name already exists. If it does exit with error

$key = Get-AzureRmStorageAccountKey -ResourceGroupName $rg -Name $acct
$ctx = New-AzureStorageContext -StorageAccountName $acct -StorageAccountKey $key.Key1
$context = Get-AzureRMContext;

$vms = get-azurermvm | where {$_.Tags.pool -eq $vmPool} | where ResourceGroupName -eq $rg | sort Name 

$script = {
    Param( $rg, $vm, $context )
    $vmname = $vm.Name
    $vm.NetworkProfile.NetworkInterfaces[0].id -match "Interfaces/(.*)"
    $nicname = $matches[1]
    Write-Output $vm.NetworkProfile.NetworkInterface
    Write-Output $nicname
    Remove-AzureRmVm -Name $vmName -ResourceGroupName $rg -AzureRmContext $context  -force -verbose
    Remove-AzureRmNetworkInterface -Name $nicname -ResourceGroupName $rg -AzureRmContext $context -force -verbose
}

$jobs = @()

Write-Output "Starting VM removal jobs"
$vms | %{ 
    $job = Start-Job -Name $_.Name $script -ArgumentList $rg, $_, $context 
    Write-Output "($_.Name) being removed in background job $($job.Name) ID $($job.Id)"   
    $jobs += @($job)
}


$sleepTime = 30
$timeElapsed = 0
$running = $true
$timeout = 3600

while($running -and $timeElapsed -le $Timeout)
{
	$running = $false
	Write-Output "Checking job status"
	$jobs | % {
		if($_.State -eq 'Running')
		{
			$running = $true
		}
    }
    if( $running) {
	    Start-Sleep -Seconds $sleepTime
        $timeElapsed += $sleepTime
    }
}

#$cont = Get-AzureStorageContainer -Context $ctx -Prefix $vmName
#$cont | remove-azurestoragecontainer -Force
$cont = Get-AzureStorageContainer -Context $ctx -Prefix ('bootdiagnostics-'+ ($vmName -replace "-"))
$cont | remove-azurestoragecontainer -Force
