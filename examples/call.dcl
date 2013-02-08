$       write sys$output "*** TEST CALL ***"
$       on warning then continue
$       set verify
$       call label1 "This is P1"
$       call label1 /output=call.lis "This is P1"
$       type call.lis
$       delete call.lis /log
$       
$label1:    subroutine
$       write sys$output p1
$       exit
$       endsubroutine
$!
