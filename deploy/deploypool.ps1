Param(
    [string] $vmPool,
    [int] $vmCount=1,
    [string] $vmOS="win",
    [string] $vmSize="Standard_DS2", 
    [int] $vmDataDisks=1, 
    [int] $vmDataDiskGB=4,
    [string] $storageEndpointSuffix="core.windows.net"
)


#first check if pool with this name already exists. If it does exit with error
#lookup the resource group from a setting that is saved when iostorm controller is created
$rg="iostorm"
$dn=$rg+$vmPool
$controller="10.0.0.4"


New-AzureRmResourceGroupDeployment -Name $dn -ResourceGroupName $rg `
                                   -TemplateFile .\umd\agent.template.$vmOS.json `
                                   -ControllerIP 10.0.0.4 -StorageEndpointSuffix $storageEndpointSuffix -vmPool $vmPool `
                                   -vmCount $vmCount -vmSize Standard_DS2 `
                                   -vmDataDiskCount $vmDataDisks -vmDataDiskSizeInGB $vmDataDiskGB `
                                   -verbose