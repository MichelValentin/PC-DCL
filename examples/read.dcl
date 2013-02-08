$   write sys$output "*** TEST READ ***"
$   create test.dat /log
LINE 1
LINE 2
LINE 3
$
$   on error then continue
$   open test test.dat /read /error=err_label
$read:
$   read test data /end=eof_label
$   write sys$output data
$   goto read
$eof_label:
$   close test
$   delete test.dat /log
$   write sys$output "*** TEST READ RANDOM ***"
$   open test test.dat /write /error=err_label /length=16
$   write test "1111111111111111"
$   write test "2222222222222222"
$   write test "3333333333333333"
$   write test "4444444444444444"
$   close test
$!
$   open test test.dat /read /error=err_label /length=16
$   read test data /rrn=2
$   write sys$output data
$   read test data /rrn=1
$   write sys$output data
$   read test data /rrn=4
$   write sys$output data
$   close test
$   delete test.dat /log
$   exit
$! error handling
$err_label:
$w f$fao("ERROR !UL - !AS",'$STATUS',f$message('$STATUS'))

