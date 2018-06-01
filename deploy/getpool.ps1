Param(
    [string] $vmPool
)

$rg = "iostorm"

$vms = get-azurermvm | where {$_.Tags.pool -eq $vmPool} | where ResourceGroupName -eq $rg

$vms | select name, StatusCode, ProvisioningState | ft

