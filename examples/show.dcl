$   write sys$output "TEST SHOW COMMAND"
$!
$   on error then continue
$   set verify
$!
$   show cpu
$!
$   show default
$!
$   show device sys$disk:
$!
$   show devices
$!
$   show dosvar
$!
$   show dosvar COMPUTERNAME
$!
$   show key /full /all
$!
$   show license
$!
$   show logical
$!
$   show logical sys$disk:
$!
$   show memory
$!
$   show process
$   show process /all
$!
$   show status
$!
$   show symbol
$!
$   show system /programs
$   show system /drivers
$   show system /all
$!
$   show term
$!
$   show time
$!
$   show translation sys$disk:
$!
$   show user
$!
$   show version
$!
