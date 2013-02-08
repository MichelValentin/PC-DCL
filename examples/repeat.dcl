$   count = 0
$   repeat
$       count = count + 1
$       write sys$output count
$       count2 = 0
$       repeat
$           count2 = count2 + 1
$           write sys$output "  "+count2
$       until count2 .eq. 2
$   until count .eq. 5
