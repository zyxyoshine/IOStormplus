﻿Configuration ConfigureVM {
    param (
        [Parameter(Mandatory)]
        [string]$ControllerIP,
        [Parameter(Mandatory)]
        [string]$VMSize
    )

    
    netsh advfirewall set privateprofile state off
    netsh advfirewall set publicprofile state off

    $PSPath = $PSCommandPath
    # DSC Script Resource
    Script VMIOAll {
        TestScript = { $false }

        GetScript = { return @{}}

        SetScript = {
            $Root = "C:\IOStormplus\"
            $WorkloadShare = $Root + "workload"
            $OutputShare = $Root + "output"
            $TempShare = $Root + "temp"

            #Create directories
            if ( -Not (Test-Path -Path $Root))
            {
                New-Item -ItemType directory -Path $Root
            }
            if ( -Not (Test-Path -Path $WorkloadShare))
            {
                New-Item -ItemType directory -Path $WorkloadShare
            }
            if ( -Not (Test-Path -Path $OutputShare))
            {
                New-Item -ItemType directory -Path $OutputShare
            }
            if ( -Not (Test-Path -Path $TempShare))
            {
                New-Item -ItemType directory -Path $TempShare
            }
            if( -Not (Get-SMBShare -Name "workload" -ea 0)){
                New-SmbShare -Name "workload" -Path $WorkloadShare -FullAccess Everyone
            }
            if( -Not (Get-SMBShare -Name "output" -ea 0)){
                New-SmbShare -Name "output" -Path $OutputShare -FullAccess Everyone
            }
            if( -Not (Get-SMBShare -Name "temp" -ea 0)){
                New-SmbShare -Name "temp" -Path $TempShare -FullAccess Everyone
            }

            Out-File ($TempShare + "\controller.tmp")
            Out-File ($TempShare + "\client.tmp")

            #Download agent
            $AgentBinaryName = "agent.exe"
            $AgentUrl = "https://github.com/zyxyoshine/IOStromplus/raw/master/deploy/binary/agent.exe"
            [Net.ServicePointManager]::SecurityProtocol = "tls12, tls11, tls"
            Invoke-WebRequest -Uri $AgentUrl -OutFile ($Root + $AgentBinaryName)

            #Download temp script
            $TempScriptName = "temp.ps1"
            $TempScriptUrl = "https://github.com/zyxyoshine/IOStromplus/raw/master/deploy/temp.ps1"
            Invoke-WebRequest -Uri $TempScriptUrl -OutFile ($Root + $TempScriptName)

            #Download and install fio
            $FioBinaryName = "fio-3.5-x64.msi"
            $FioUrl = "https://github.com/zyxyoshine/IOStromplus/raw/master/deploy/binary/fio-3.5-x64.msi"
            Invoke-WebRequest -Uri $FioUrl -OutFile ($Root + $FioBinaryName)

            $DataStamp = get-date -Format yyyyMMdd
            $logFile = '{0}-{1}.log' -f ($Root + $FioBinaryName),$DataStamp
            $FioMSIArguments = @(
                "/i"
                ('"{0}"' -f ($Root + $FioBinaryName))
                "/qn"
                "/norestart"
                "/L*v"
                $logFile
            )
            Start-Process "msiexec.exe" -ArgumentList $FioMSIArguments -Wait -NoNewWindow

            #Initialize data disks
            $disks = Get-Disk | Where partitionstyle -eq 'raw' | sort number

            $letters = 70..89 | ForEach-Object { [char]$_ }
            $count = 0
            $label = "data"

            foreach ($disk in $disks) {
                $driveLetter = $letters[$count].ToString()
                $disk | 
                Initialize-Disk -PartitionStyle MBR -PassThru |
                New-Partition -UseMaximumSize -DriveLetter $driveLetter |
                Format-Volume -FileSystem NTFS -NewFileSystemLabel ($label + $count) -Confirm:$false -Force
                $count++
            }

            #Start Agent
            $ControllerIP = $using:ControllerIP
            $VMname = hostname
            $VMSize = $using:VMSize
            $VMIp = foreach($ip in (ipconfig) -like '*IPv4*') { ($ip -split ' : ')[-1]} 
            $VMIp | Out-File -Append C:\tt.txt
            $AgentArguments = @(
                $ControllerIP
                $VMname
                $VMIp
                $VMSize
            )
            $username = "vmadmin" 
            $password = "!!!!1234abcd"

            whoami | Out-File C:\WHOAMI.txt

            $psPath = $using:PSPath
            $psScriptDir = Split-Path -Parent -Path $psPath
            $psScriptName = "temp.ps1"
            $psScriptPath = "$psScriptDir\$psScriptName"
            $args = ' -NoProfile -WindowStyle Hidden -Command "& ' + "'" + $psScriptPath + "' '" + $ControllerIP + "' '" + $VMname + "' '" + $VMIp + "' '" + $VMSize + "'" + '"'
            $action = New-ScheduledTaskAction -Execute "Powershell.exe" -Argument $args
            $trigger = @()
            $trigger += New-ScheduledTaskTrigger -Once -At (Get-Date).AddMinutes(2)
            if($vmFixedIops -ne 0) {
                $trigger += New-ScheduledTaskTrigger -AtStartup
            }
            $settings = New-ScheduledTaskSettingsSet -StartWhenAvailable -RunOnlyIfNetworkAvailable -DontStopOnIdleEnd -RestartCount 3 -RestartInterval 240 -Priority 4 -ErrorAction Ignore 
            Unregister-ScheduledTask -TaskName "VMAgent" -Confirm:0 -ErrorAction Ignore
            Register-ScheduledTask -Action $action -Trigger $trigger -TaskName "VMAgent" -Description "VM iostorm" -User $username -Password $password -RunLevel Highest -Settings $settings
        }
    }

}
