disks=($(lsblk -l -p -o NAME | grep "sd" | grep -v "sda" | grep -v "sdb"))
sudo pvcreate ${disks[*]}
sudo vgcreate datavg ${disks[*]}
sudo lvcreate -l 100%FREE --type striped -i ${#disks[*]} -n datav datavg
sudo mkfs -t ext4 /dev/mapper/datavg-datav
sudo mkdir /data
sudo mount -t ext4 /dev/mapper/datavg-datav  /data
sudo chmod 777 /data


sudo umount /data
sudo lvremove datavg/datav



get-partition | where DriveLetter -ne "C" | where DriveLetter -ne "D" | remove-partition

$poolname = "datapool"
$vdname = "datavd"
$disks = Get-PhysicalDisk -CanPool $True
$storage = Get-StorageSubSystem
$pool = New-StoragePool -FriendlyName $poolname -PhysicalDisks $disks -StorageSubSystemName $storage.Name
$vdisk = New-VirtualDisk -FriendlyName $vdname `
                -ResiliencySettingName Simple `
                -NumberOfColumns $Disks.Count `
                -UseMaximumSize -Interleave 256KB -StoragePoolFriendlyName $poolname
$vdiskNumber = (Get-disk -FriendlyName $vdname).Number
Initialize-Disk -FriendlyName $vdname
$partition = New-Partition -UseMaximumSize -DiskNumber $vdiskNumber -DriveLetter X
Format-Volume -DriveLetter X -FileSystem ReFS -NewFileSystemLabel "data"



fio --name=testrun --iodepth=64 --rw=randrw --bs=8k --direct=1 --size=1G --numjobs=64 --runtime=60 --group_reporting --write_iops_log=/mnt/logs/testrun --write_lat_log=/mnt/logs/testrun --write_bw_log=/mnt/logs/testrun --log_avg_msec=1000 --ioengine=posixaio --directory=/home/superuser

fio --name=testrun --iodepth=64 --rw=randrw --bs=8k --direct=1 --size=1G --numjobs=4 --runtime=60 --group_reporting --write_iops_log=D:\testrun --write_lat_log=D:\testrun --write_bw_log=D:\testrun --log_avg_msec=1000 --ioengine=windowsaio