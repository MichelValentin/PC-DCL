$!
$   write sys$output "TEST INQUIRE"
$!
$   inquire /global /punctuation /timeout=0:0:30 i1
$   inquire /local /nopunctuation /timeout=0:0:30 i2 "i2 = "
$   inquire /timeout=0:0:30 i3
$   show symbol i1
$   show symbol i2
$   show symbol i3
$   delete /symbol i1
$   delete /symbol i2
$   delete /symbol i3

