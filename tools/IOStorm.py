from azure.storage.table import TableService
from azure.storage.table import Entity 
from azure.storage.blob  import BlockBlobService
import datetime, time

class Config:
    NodeTable  = 'IOStormNodes'
    ExecTable  = 'IOStormExec'
    CommTable  = 'IOStormComm'
    Interval   = 10


#retrieve the locations and secret for connecting to storage account 
class Account():
    Name = 'saiostorm'
    Key = 'Jnh74fG573TaZww1L8QYhnGKym1J+JxCrOKl8F5T28i81tuoVrANrN4DQQLUSxRpOIUNRuybHr1HYx1ORgc3xw=='
    Endpoint  = 'redmond.azurestack.corp.microsoft.com'

#get table service
tablesvc = TableService(
    account_name=Account.Name, 
    account_key=Account.Key,
    endpoint_suffix=Account.Endpoint
)

def CheckTable():
    #check if table exists
    #if table does not exist create it 
    if( not tablesvc.exists( Config.NodeTable )): 
        tablesvc.create_table( Config.NodeTable )
    if( not tablesvc.exists( Config.ExecTable )): 
        tablesvc.create_table( Config.ExecTable )
    if( not tablesvc.exists( Config.CommTable )): 
        tablesvc.create_table( Config.CommTable )



def AddTask( pool, node, command, commandLine ):
    task  = Entity()
    task.PartitionKey = pool + "_" + node
    task.RowKey = str(time.time())
    task.Command = command
    task.CommandLine = commandLine
    tablesvc.insert_entity( Config.CommTable, task )


class Pool:
    #Pool actions and properties
    def Load( self ):
        #load the pool from DB
        #check and compare with resources

class Node: 
    #Node actions and properties
    def Load( self ):
        #load the node from database
        #check and compare with resources



