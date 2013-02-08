$       write sys$output "*** TEST COPY ***"
$       on error then continue
$       copy copy.dcl test.$$$ /log 
$       copy copy.dcl test.$$$ /log /norepl
$       copy copy.dcl test.$$$ /log /new
$       copy *.dcl *.$$$ /conf /log
$       dir *.$$$
$       delete *.$$$; /log
