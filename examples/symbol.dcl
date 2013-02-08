$       write sys$output "*** TEST SYMBOL ASSIGN ***"
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
$       show symbol s*
