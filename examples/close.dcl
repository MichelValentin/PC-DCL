$       write sys$output "*** TEST CLOSE ***"
$   create test.dat /log
LINE 1
LINE 2
LINE 3
$
$   on error then continue
$   open test test.dat /read
$   close test
$   close test /log
$   close test /error=err_label
$   delete test.dat /log
$   exit
$! error handling
$err_label:
$write sys$output f$fao("ERROR !UL - !AS",$STATUS,f$message($STATUS))
