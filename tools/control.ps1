Param(
    [string] $Object,
    [string] $Command="", 
    [string] $Params=""
)

$AccountName = 'saiostorm'
$AccountKey = 'Jnh74fG573TaZww1L8QYhnGKym1J+JxCrOKl8F5T28i81tuoVrANrN4DQQLUSxRpOIUNRuybHr1HYx1ORgc3xw=='
$AccountEndpoint  = 'redmond.azurestack.corp.microsoft.com'

$NodeTableName  = 'IOStormNodes'
$ExecTableName  = 'IOStormExec'
$TaskTableName  = 'IOStormTask'
$JobTableName   = 'IOStormJob'

$ctx = New-AzureStorageContext -StorageAccountName $AccountName -StorageAccountKey $AccountKey -Endpoint $AccountEndpoint

$tables = Get-AzureStorageTable -Context $ctx

$NodeTable = $tables | where Name -eq $NodeTableName 
if( -not $NodeTable ) { New-AzureStorageTable -Name $NodeTableName -Context $ctx }
$ExecTable = $tables | where Name -eq $ExecTableName 
if( -not $ExecTable ) { New-AzureStorageTable -Name $ExecTableName -Context $ctx}
$TaskTable = $tables | where Name -eq $TaskTableName 
if( -not $TaskTable ) { New-AzureStorageTable -Name $TaskTableName -Context $ctx}
$JobTable = $tables | where Name -eq $JobTableName 
if( -not $JobTable ) { New-AzureStorageTable -Name $JobTableName -Context $ctx}


function EntityToObject ($item)
{
    $p = new-object PSObject
    $p | Add-Member -Name ETag -TypeName string -Value $item.ETag -MemberType NoteProperty 
    $p | Add-Member -Name PartitionKey -TypeName string -Value $item.PartitionKey -MemberType NoteProperty
    $p | Add-Member -Name RowKey -TypeName string -Value $item.RowKey -MemberType NoteProperty
    $p | Add-Member -Name Timestamp -TypeName datetime -Value $item.Timestamp -MemberType NoteProperty

    $item.Properties.Keys | foreach { 
        $type = $item.Properties[$_].PropertyType;
        $value = $item.Properties[$_].PropertyAsObject; 
        Add-Member -InputObject $p -Name $_ -Value $value -TypeName $type -MemberType NoteProperty -Force 
    }
    $p
}

$pools = @()
function LoadPools()
{
    $global:pools = @()
    $query = New-Object "Microsoft.WindowsAzure.Storage.Table.TableQuery"
    $data = $NodeTable.CloudTable.ExecuteQuery($query)
    $pooldata = $data | group PartitionKey 
    foreach( $poolentry in $pooldata )
    {
        $pool = [PSCustomObject]@{
            Name = $poolentry.Name
            Nodes = @()
            State = 'NA'
        }
        $global:pools += @( $pool )
        if( $poolentry.Group.Length -gt 0 ) { $pool.State = "READY"} 
        foreach( $nodeentry in $poolentry.Group )
        {
            $node = [PSCustomObject]@{
                Pool = $nodeentry.PartitionKey
                Name = $nodeentry.RowKey
                Timestamp = $nodeentry.TimeStamp
                State = $nodeentry.Properties["State"].StringValue
                IP = $nodeentry.Properties["IP"].StringValue
                OS = $nodeentry.Properties["OS"].StringValue
                Size = $nodeentry.Properties["Size"].StringValue               
            }
            if( $node.State -ne "READY" ) { $pool.State = "NOT READY"} 
        }
        $pool.Nodes += @($node)
    }
}

function ListPools()
{
    LoadPools
    foreach( $pool in $global:pools )
    {
        Write-Host "Pool: " $pool.Name " " $pool.State
        $pool.Nodes | select Name, State, IP, OS, Size, Timestamp | ft
    }

}

function StartJob( $Params )
{
    #$params format:
    #pool1=fiojobfile1 pool2=fiojobfile2

    LoadPools

    $jobid = ("job " + ( Get-Date -Format u ))
    $pooljobs = @()
    foreach( $param in $Params.split( ' ' ) )
    {
        $poolparam = $param.split( '=' )
        $pool = $global:pools | where Name -eq $poolparam[0] | where State -eq "READY"
        if( -not $pool) 
        {
            Write-Host "Pool "  $poolparam[0] " does not exist or is not ready to accept jobs"
            return
        }

        $pooljob = [PSCustomObject]@{
            Id      = $jobId
            Pool    = $pool 
            Command = "EXECUTE"
            CommandLine = "fio"
            JobFile = $poolparam[1]            
        }
        $pooljobs += @($pooljob)
    }

    foreach( $pooljob in $pooljobs )
    {
        #copy file $pooljob.FileName to blob storage 
        Write-Host "Executing job: " $pooljob.JobFile " on pool: " $pooljob.Pool.Name 
        foreach( $node in $pooljob.Pool.Nodes )
        {
            StartTask $node $pooljob
        }


    }

    $entity = New-Object "Microsoft.WindowsAzure.Storage.Table.DynamicTableEntity" `
                $job.Id, `
                '', `
                @{   `
                    "Command"=$job.Command; `
                    "Params"=$job.CommandLine; `
                    "File"=$job.JobFile `
                }                      
    JobTable.CloudTable.Execute([Microsoft.WindowsAzure.Storage.Table.TableOperation]::Insert($entity))        
}

function GetJob( $Params )
{
    Write-Host "Get a job"
}

function ListJobs()
{
    Write-Host "List jobs"
}

function StartTask( $node, $job )
{
    Write-Host "  Starting task: " $job.Command "|" $job.CommandLine "| on node: " $Node.Name  

    $entity = New-Object "Microsoft.WindowsAzure.Storage.Table.DynamicTableEntity" `
                        $node.Pool + "_" + $node.Name, `
                        $job.Id, `
                        @{   `
                            "Command"=$job.Command; `
                            "Params"=$job.CommandLine; `
                            "File"=$job.JobFile `
                        }                      
    TaskTable.CloudTable.Execute([Microsoft.WindowsAzure.Storage.Table.TableOperation]::Insert($entity))

}

switch( $object ){
    "job" {
        switch( $command ){
            "get"{
                if( $Params -eq "" ){
                    ListJobs
                }
                else{
                    GetJob $Params
                }
                break
            }
            "start"{
                StartJob $Params
                break
            }
            default{
                Write-Host "ERROR: unrecognized command"
            }    
        }
    }

    "pool"{
        switch( $command ){
            "get"{
                if( $Params -eq "" ){
                    ListPools
                }
                else{
                    GetPool $Params
                }
                break
            }
            "enable"{
                EnablePool $Params
                break
            }
            "disable"{
                DisablePool $Params
                break
            }
            default{
                Write-Host "ERROR: unrecognized command"
            }    
        }
    }

    default{
        Write-Host "ERROR: unrecognized object"
    }
}

