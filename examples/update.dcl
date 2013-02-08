$   write sys$output "*** TEST FILE UPDATE ***"
$   open test test.dat /write /error=err_label /length=16
$   write test f$fao("!16*1")
$   write test f$fao("!16*2")
$   write test f$fao("!16*3")
$   write test f$fao("!16*4")
$   close test
$   type test.dat
$!
$   open test test.dat /read /write /error=err_label /length=16
$   read test data /rrn=2
$   write sys$output data
$   write test "AAAAAAAAAAAAAAAA"
$   read test data /rrn=2
$   write sys$output data
$   close test
$   type test.dat
$   delete test.dat /log
$   exit
$! error handling
$err_label:
$w f$fao("ERROR !UL - !AS",'$STATUS',f$message('$STATUS'))


