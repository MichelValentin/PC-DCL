$       write sys$output "*** TEST open ***"
$   create test.dat /log
LINE 1
LINE 2
LINE 3
$
$   on error then continue
$   open test test.dat /read /error=err_label
$   close test
$   delete test.dat /log
$   open test test.dat /read /error=err_label
$   exit
$! error handling
$err_label:
$w f$fao("ERROR !UL - !AS",'$STATUS',f$message('$STATUS'))
