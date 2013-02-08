$       write sys$output "*** TEST ALLOCATE ***"
$       allocate sys$login: testdisk1: /log
$       show logical testdisk1
$       dir testdisk1:*.*
$       deallocate testdisk1:
$       sho logical testdisk1

