Param(
    [string] $ResourceGroupName,
    [string] $UserName,
    [string] $Password
)


#first check if pool with this name already exists. If it does exit with error
#lookup the resource group from a setting that is saved when iostorm controller is created
$storageEndpointSuffix= ((Get-AzureRmContext).Environment | Get-AzureRmEnvironment).StorageEndpointSuffix

New-AzureRmResourceGroupDeployment -Name "controllerdeployment" -ResourceGroupName $ResourceGroupName `
                                   -TemplateFile .\umd\controller.template.json `
                                   -StorageEndpointSuffix $storageEndpointSuffix `
                                   -vmAdminUserName $UserName -vmAdminPassword $Password `
                                   -verbose