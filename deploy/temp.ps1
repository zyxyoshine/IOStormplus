$username = "vmadmin"
$password = "!!!!1234abcd"
$credentials = New-Object System.Management.Automation.PSCredential -ArgumentList @($username,(ConvertTo-SecureString -String $password -AsPlainText -Force))
$AgentArguments = @(
    $args[0]
    $args[1]
    $args[2]
    $args[3]
)
whoami | Out-File c:\test.txt
Start-Process "C:\IOStormplus\agent.exe" -ArgumentList $AgentArguments -WorkingDirectory "C:\IOStormplus" -Credential ($credentials) -RedirectStandardOutput "Agent.txt" -RedirectStandardError "AgentError.txt" -Wait