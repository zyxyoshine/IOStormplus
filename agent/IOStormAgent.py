from azure.storage.table import TableService
from azure.storage.table import Entity
from azure.storage.blob  import BlockBlobService
import datetime, time
import subprocess
import logging, sys
import yaml

logging.basicConfig(level=logging.INFO, stream=sys.stdout)

def WriteConfig( params ):
    config = dict(
        Account = dict (
            Name = params[2],
            Key = params[3],
            Endpoint = params[4],
        ),
        Node = dict (
            Pool = params[5],
            Name = params[6],
            IP = params[7],
            OS = params[8],
            Size = params[9],
            Disk = params[10],
            DiskSize = params[11]
        )
    )

    with open('config.yml', 'w') as outfile:
        yaml.dump(config, outfile, default_flow_style=False)

if( len(sys.argv) > 1 ):
    WriteConfig( sys.argv )
    exit(0)        

config = yaml.safe_load(open("config.yml"))
Interval = 10
WorkloadContainer = "workload"
OutputContainer = "output"
WorkloadPath = "./workload/"
OutputPath   = "./output/"

class Tables:
    NodeTable  = 'IOStormNodes'
    ExecTable  = 'IOStormExec'
    TaskTable  = 'IOStormTask'

class NodeState:
    Ready     = 'READY'
    Executing = 'EXECUTING'
    Paused    = 'PAUSED'
    Error     = 'ERROR'

class ExecState:
    Executing = 'EXECUTING'
    Completed = 'COMPLETED'
    Canceled  = 'CANCELED'
    Error     = 'ERROR'


#retrieve the locations and secret for connecting to storage account 
class Account:
    Name = config['Account']['Name']
    Key = config['Account']['Key']
    Endpoint  = config['Account']['Endpoint']

class Node:    
    def __init__( self, pool, name, ip, os, size, disk, disksize ):
        self.Pool = pool
        self.Name = name
        self.IP = ip
        self.OS = os
        self.Size = size
        self.State = NodeState.Ready
        self.Execution = None

    def Refresh( self ):
        logging.debug( "Node state refresh: " + self.State )
        if( self.State == NodeState.Ready ):
            self.UpdateState( NodeState.Ready )
        if( self.State == NodeState.Executing ):
            #Check if program is still executing
            logging.debug( "Checking execution progress" )
            self.Execution.Poll()
            if( self.Execution.State == ExecState.Executing ):
                logging.debug( "Execution in progress" )
                self.UpdateState( NodeState.Executing )
            else:
                logging.debug( "Execution completed" )
                self.UpdateState( NodeState.Ready )

    def GetCommand( self ):
        #get any pending command from queue
        entityPartitionKey = (self.Pool + "_" + self.Name)
        commands = tablesvc.query_entities( Tables.TaskTable, filter = "PartitionKey eq '" + entityPartitionKey + "'", num_results = 10  )    
        logging.info( "Commands retrieved. Processing..." )
        for command in commands:
            temp = tablesvc.delete_entity( Tables.TaskTable, entityPartitionKey, command.RowKey)
        return commands

    def UpdateState( self, newstate ):
        #update the status of this node
        self.State = newstate
        logging.info( 'STATUS CHANGE: ' + self.State )
        status  = Entity()
        status.PartitionKey = self.Pool
        status.RowKey = self.Name
        status.Command = ""
        status.State  = self.State
        status.IP = self.IP
        status.OS = self.OS
        status.Size = self.Size
        temp = tablesvc.insert_or_replace_entity( Tables.NodeTable, status )

    def ExecuteCommand( self, command ):
        #execute the command 
        logging.info( "Executing command " + command.CommandLine )
        self.Execution = Execution( command )
        self.Execution.Run()
        self.UpdateState(  NodeState.Executing )

    def CancelCommand( self ):
        logging.info( "Canceling command..." )    
        self.Execution.Kill()
        self.Execution = None
        self.UpdateState( NodeState.Ready )

    def Pause( self ):
        #do not take any new commands, except !resume  
        logging.info( "Pausing..." )    
        self.UpdateState( NodeState.Paused )

    def Resume( self ):
        #resume operations
        logging.info( "Resuming..." )    
        self.UpdateState( NodeState.Ready )

    def Reset( self ):
        logging.info( "Resetting..." )    
        if( self.State == NodeState.Executing ):
            self.CancelCommand()
        self.UpdateState( NodeState.Ready )

     
class Execution:
    def __init__( self, command ):
        self.Command = command
        self.State  = ExecState.Executing
        self.Output  = self.Command.RowKey + self.Command.PartitionKey
        self.Process = None

    def UpdateState( self, newstate ):
        logging.info( "Execution status update: " + newstate )
        self.State = newstate
        execrec  = Entity()
        execrec.PartitionKey = self.Command.RowKey
        execrec.RowKey = self.Command.PartitionKey
        execrec.Executable = self.Command.CommandLine
        execrec.State = self.State    
        execrec.Output = self.Output
        temp = tablesvc.insert_or_replace_entity( Tables.ExecTable, execrec )

    def Run( self ):
        filepath = WorkloadPath + self.Command.File
        logging.info( "Copying blob " + self.Command.File + " to file " + filepath )
        temp = blobsvc.get_blob_to_path( WorkloadContainer, self.Command.File, filepath )
        commandline = self.Command.CommandLine + ' --output "' + OutputPath + self.Output + '" --output-format=json --lat_percentiles=1 "' + filepath + '"'
        logging.info( "Executing: " + commandline )
        self.Process = subprocess.Popen( commandline, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        self.UpdateState( ExecState.Executing )

    def Poll( self ):
        logging.info( "Checking command" )
        p = self.Process.poll()
        if( p == None):
            logging.info( "Command running")
            self.UpdateState( ExecState.Executing )
        else:
            if( p == 0 ):
                logging.info( "Command completed")
                self.UpdateState( ExecState.Completed )
            else:
                logging.info( "Command exited with error. Exit code " + str(p) )
                self.UpdateState( ExecState.Error )        

            blobsvc.create_blob_from_path( OutputContainer, self.Output, OutputPath + self.Output  )

        return self.State


    def Kill( self ):
        logging.info( "Killing process" )
        self.Process.Kill()
        self.Process = None
        self.UpdateState( ExecState.Canceled )

    def CollectLogs( self ):
        return


node = None

#get table service
tablesvc = TableService(
    account_name=Account.Name, 
    account_key=Account.Key,
    endpoint_suffix=Account.Endpoint
)

blobsvc = BlockBlobService(
    account_name=Account.Name, 
    account_key=Account.Key,
    endpoint_suffix=Account.Endpoint
)


def CheckTable():
    #check if table exists
    #if table does not exist create it 
    if( not tablesvc.exists( Tables.NodeTable )): 
        tablesvc.create_table( Tables.NodeTable )
    if( not tablesvc.exists( Tables.ExecTable )): 
        tablesvc.create_table( Tables.ExecTable )
    if( not tablesvc.exists( Tables.TaskTable )): 
        tablesvc.create_table( Tables.TaskTable )





def Main():
    node = Node( 
        pool = config['Node']['Pool'],
        name = config['Node']['Name'],
        ip   = config['Node']['IP'],
        os   = config['Node']['OS'],
        size = config['Node']['Size'], 
        disk = config['Node']['Disk'], 
        disksize = config['Node']['DiskSize']
    )
    while ( True ):
        #   TODO periodically CheckTable() && CheckQueue()
        commands = node.GetCommand()
        logging.info( "CURRENT STATE: " + node.State )
        for command in commands:
            logging.info( command.RowKey + ' ' + command.Command + " " + command.CommandLine )
            if( command.Command == "RESET"):
                node.Reset()
            if( node.State == NodeState.Ready ):
                if( command.Command == "EXECUTE" ):            
                    node.ExecuteCommand( command )            
                if( command.Command == "PAUSE"):
                    node.Pause()
            if( node.State == NodeState.Executing ):
                if( command.Command == "CANCEL" ):            
                    node.CancelCommand()
            if( node.State == NodeState.Paused ):
                if( command.Command == "RESUME" ):            
                    node.Resume()
        node.Refresh()
        time.sleep( Interval )



CheckTable()        
Main()