$       set term /color=(yellow,blue) /clear
$       line = 1
$       col  = 6
$       while line .lt. 24 do
$               set term /color=(yellow,blue)
$               write /position=line,col sys$output "ÊDCL" 
$               set term /color=(green,blue)
$               write /position=(24-line,col) sys$output "ÊDCL" 
$               line = line + 1
$               col = col + 3
$       endwhile

$       set term /color=(black,white)
$       inquire /position=(25,30) /length=1 yn "Continue (Y/N) " 
$       if yn .eqs. "N" then exit
           
$       cls
$       write sys$output f$fao("!#**<!10* >!#**",20,20)
$       call testif "TRUE"
$       call testif "FALSE"

$testif:    subroutine
$       if p1 .eqs. "TRUE" 
$       then    write sys$output "TRUE 1"
$               write sys$output "TRUE 2"
$       else    write sys$output "FALSE 1"
$               write sys$output "FALSE 2"
$       endif
$       exit
$endsubroutine
