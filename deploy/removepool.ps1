Param(
    [string] $rg,
    [string] $vmPool
)


#first check if pool with this name already exists. If it does exit with error
#lookup the resource group from a setting that is saved when iostorm controller is created
$acct = "sa"+$rg

$vms = get-azurermvm | where {$_.Tags.pool -eq $vmPool} | where ResourceGroupName -eq $rg
$nics = Get-AzureRmNetworkInterface | where Name -like $vmPool* | where ResourceGroupName -eq $rg

$vms | Remove-AzureRmVm -force -verbose
$nics | Remove-AzureRmNetworkInterface -force -verbose

#also delete the storage containers

$key = Get-AzureRmStorageAccountKey -ResourceGroupName $rg -Name $acct
$ctx = New-AzureStorageContext -StorageAccountName $acct -StorageAccountKey $key.Key1
$cont = Get-AzureStorageContainer -Context $ctx -Prefix $vmPool
$cont | remove-azurestoragecontainer -Force

$cont = Get-AzureStorageContainer -Context $ctx -Prefix ('bootdiagnostics-'+$vmPool)
$cont | remove-azurestoragecontainer -Force
