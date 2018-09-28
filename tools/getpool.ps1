Param(
    [string] $rg,
    [string] $vmPool
)

if( $vmPool )
{
    $vms = get-azurermvm | where {$_.Tags.pool -eq $vmPool} | where ResourceGroupName -eq $rg
}
else {
    $vms = get-azurermvm | where {$_.Tags.pool -ne $Null} | where ResourceGroupName -eq $rg
}

$vms | select {$_.Tags.pool}, Name, StatusCode, ProvisioningState | ft

