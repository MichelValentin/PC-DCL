$       inquire cond /timeout=0:0:10
$       if cond then 
$               write sys$output cond + " NOT EQUAL 0"
$               if 1
$                       write sys$output "Nested if"
$               endif
$       else
$               write sys$output cond + " EQUAL 0"
$       endif
$       count = 0
$loop:
$       count = count + 1
$       write sys$output count
$       if count .lt. 10 then goto loop
$       write sys$output "Count = " + count

