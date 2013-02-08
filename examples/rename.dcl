$!
$       write sys$output "*** TEST RENAME ***"
$!
$       create /dir test1 /log
$       create /dir test2 /log
$       copy *.dcl test1 /log
$       rename [.test1]*.dcl [.test2] /log
$       delete [.test1]*.dcl  /log
$       delete [.test2]*.dcl  /log
$       delete test1 /log
$       delete test2 /log
