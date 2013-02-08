$       write sys$output "*** TEST ASSIGN ***"
$       assign sys$login: testdisk1 /log
$       show logical testdisk1
$       dir testdisk1:dcl.*
$       deassign testdisk
$       show logical testdisk

