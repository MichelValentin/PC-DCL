$       today == f$extract(0,11,f$time())
$       open /read/error=sethost infile "sys$login:bar.dat"
$       read /error=sethost /end=sethost infile data
$       if f$extract(0,11,data) .EQS. today then goto close_file
$sethost:
$       run STH         ! start terminal emulation
$       open /write/error=file_error outfile "sys$login:bar.dat"
$       write /error=file_error outfile today
$close_file:
$       close infile
$       close outfile
$       goto exit
$file_error:
$       write sys$output f$fao("ERROR !UL - !AS",'$STATUS',f$message('$STATUS'))
$exit:
