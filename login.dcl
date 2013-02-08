$!       set verify
$       w :== WRITE SYS$OUTPUT
$       if f$mode() .eqs. "INTERACTIVE"
$               dir :== dir /size /date
$               del :== del /log
$               set term /insert /clear
$               set prompt $p_$
$               show version
$               w "Type LOgout to quit..."
$               w " "
$               tst :== "set def sys$dcl:[examples]"
$       endif
$       define /key/nolog/erase/terminate f10 "recall /all"
$       define /key/nolog/erase f11 "recall "
$       define /key/nolog/erase/term/noecho f12 cls
$       define /key/nolog/erase/term/noecho sf12 step
