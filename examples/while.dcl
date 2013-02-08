$   count = 0
$   while count .LT. 5 do
$       count = count + 1
$       write sys$output count
$       count2 = 0
$       while count2 .LT. 2 do
$           count2 = count2 + 1
$           write sys$output "   "+count2
$       endwhile    
$   endwhile    

