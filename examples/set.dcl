$   write sys$output "TEST SET COMMAND"
$!
$   set verify
$   set default c:\
$   show default
$   set default sys$login:
$   show default
$!
$   on warning then write sys$output "ERROR"
$   set noon
$   type fgfgfgf.lll
$   set on
$   type fgfgfgf.lll
$!
$   on error then exit
$   create test.dat
$   
$   set file test.dat /attr=RS /log
$   dir test.dat /all /attr
$   set file test.dat /attr=WA /log
$   dir test.dat /all /attr
$   set file test.dat /date=tomorrow /log
$   dir test.dat /time
$   delete test.dat
$!
$   set prompt "-->"
