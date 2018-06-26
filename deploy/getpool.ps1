Param(
    [string] $vmPool=""
)

$rg = "iostorm"

if( $vmPool -eq "" )
{
    $vmPool = "_control"
    $vms = get-azurermvm | where {$_.Tags.pool -ne $vmPool} | where ResourceGroupName -eq $rg
    $vms.Tags.pool | sort | get-unique
}
else
{
    $vms = get-azurermvm | where {$_.Tags.pool -eq $vmPool} | where ResourceGroupName -eq $rg
    $vms | select name, StatusCode, ProvisioningState | ft
}



