$       write sys$output "*** TEST DEFINE LOGICAL ***"
$       define testdisk1 ini$home: /log
$       show logical testdisk1
$       dir testdisk1:dcl.*
$       deassign testdisk
$       show logical testdisk
$       write sys$output "*** TEST DEFINE KEY ***"
$       define /key SF1 "delete /key sf1 /log" /echo /erase /log /terminate
$       show key
$       write sys$output "Press SF1 to test DEFINE /KEY"
