$       write sys$output "*** TEST APPEND ***"
$       append *.dcl pipo.tmp /text /log
$       dir pipo.tmp /size
$       delete pipo.tmp; /log
