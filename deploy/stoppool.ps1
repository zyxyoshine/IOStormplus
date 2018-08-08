Param(
    [string] $rg,
    [string] $vmPool
)

#first check if pool with this name already exists. If it does exit with error
#lookup the resource group from a setting that is saved when iostorm controller is created


#check if the state of the pool machiknes in the pool is not "starting". 
$vms = get-azurermvm | where {$_.Tags.pool -eq $vmPool} | where ResourceGroupName -eq $rg

$vms | Stop-AzureRmVM -Force