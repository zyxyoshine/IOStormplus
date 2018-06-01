Param(
    [string] $vmPool
)


#first check if pool with this name already exists. If it does exit with error
#lookup the resource group from a setting that is saved when iostorm controller is created
$rg = "iostorm" 
$acct = ""

$vms = get-azurermvm | where {$_.Tags.pool -eq $vmPool} | where ResourceGroupName -eq $rg
$nics = Get-AzureRmNetworkInterface | $| where {$_.Tags.pool -eq $vmPool} | where ResourceGroupName -eq $rg

$vms | Remove-AzureRmVm -force 
$nics | Remove-AzureRmNetworkInterfaceIpConfig -force 

#also delete the storage containers
