$!
$       write sys$output "*** TEST DELETE FILE ***"
$!
$       create /dir test
$       copy *.dcl test /log
$       delete [.test]*.dcl /log
$       delete test /log
$!
$       write sys$output "*** TEST DELETE SYMBOL ***"
$!
$       s1 := test1
$       s2 :== test2
$       s3 = -1
$       s4 = 1
$       s5 = s1
$       s6 == s1
$       sho symbol /global
$       sho symbol /local
$       sho symbol s1
$       delete /symbol s1 /log
$       delete /symbol /local /log /all
$       delete /symbol s* /global /log
$       show symbol /all

$!
$       write sys$output "*** TEST DELETE KEY ***"
$!
$       define /key F1 "test1" /echo /erase /log /noterminate
$       define /key F2 "test2" /echo /erase /log /noterminate
$       define /key F3 "test3" /echo /erase /log /noterminate
$       define /key F4 "test4" /echo /erase /log /noterminate
$       show key
$       delete /key /log f4
$       delete /key /log /all
$       show key
