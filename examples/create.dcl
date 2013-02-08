$       write sys$output "*** TEST CREATE ***"
$   create /dir createdir /log
$   create [.createdir]test.dat /log
LINE 1
LINE 2
LINE 3
$
$   type [.createdir]test.dat
$   delete [.createdir]test.dat /log
$   delete createdir /log
$
$ x = f$edit(f$trnlnm("INI$FTYPE"), "UPCASE")
$
$ if x .eq. "VMS" then exit

$   create /dir create.dir /log
$   create create.dir/test.dat /log
LINE 1
LINE 2
LINE 3
$
$   type create.dir/test.dat
$   delete create.dir/test.dat /log
$   delete create.dir /log
$ endif
