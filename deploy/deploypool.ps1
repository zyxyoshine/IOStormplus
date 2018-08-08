Param(
    [string] $rg,
    [string] $vmPool,
    [int] $vmCount=1,
    [string] $vmOS="windows",
    [string] $vmSize="Standard_DS2", 
    [int] $vmDataDisks=1, 
    [int] $vmDataDiskGB=4,
    [string] $vmAdminUserName,
    [string] $vmAdminPassword
)


#first check if pool with this name already exists. If it does exit with error
#lookup the resource group from a setting that is saved when iostorm controller is created
$dn=$rg+$vmPool
$storageEndpointSuffix= ((Get-AzureRmContext).Environment | Get-AzureRmEnvironment).StorageEndpointSuffix

New-AzureRmResourceGroupDeployment -Name $dn -ResourceGroupName $rg `
                                   -TemplateFile .\umd\agent.template\agent.template.$vmOS.json `
                                   -StorageEndpointSuffix $storageEndpointSuffix -vmPool $vmPool `
                                   -vmCount $vmCount -vmSize $vmSize `
                                   -vmDataDiskCount $vmDataDisks -vmDataDiskSizeInGB $vmDataDiskGB `
                                   -vmAdminUserName $vmAdminUserName -vmAdminPassword $vmAdminPassword `
                                   -verbose