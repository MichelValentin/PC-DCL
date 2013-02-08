on error then continue
set verify

$!******************
$!*     F$CHR      *
$!******************
!write sys$output f$chr(7) + "." + f$chr(7) + "." + f$chr(7) + "."

$!******************
$!*     F$CVSI     *
$!******************
A = "+..."
X = f$cvsi(0,4,a)
show symbol X

$!******************
$!*     F$CVUI     *
$!******************
A = "++++"
X = f$cvui(0,4,a)
show symbol X
                
$!***********************
$!*     F$DIRECTORY     *
$!***********************
write sys$output f$directory()

$!***********************
$!*     F$EDIT          *
$!***********************
edit_test = "   This is a   TEST   !comment  "
write sys$output "<"+f$edit(edit_test,"COLLAPSE")+">"
write sys$output "<"+f$edit(edit_test,"COMPRESS")+">"
write sys$output "<"+f$edit(edit_test,"LOWERCASE")+">"
write sys$output "<"+f$edit(edit_test,"TRIM")+">"
write sys$output "<"+f$edit(edit_test,"UNCOMMENT")+">"
write sys$output "<"+f$edit(edit_test,"UPCASE")+">"

$!***********************
$!*     F$ELEMENT       *
$!***********************
list = "ELEM1/ELEM2/ELEM3/ELEM4/ELEM5"
write sys$output  f$element(2,"/",list)
write sys$output  f$element(1,"/","ELEM1/ELEM2/ELEM3/ELEM4/ELEM5")
write sys$output  f$element(99,"/",list)

$!***********************
$!*     F$EXTRACT       *
$!***********************
write sys$output f$extract(3,3,"ABCDEFGHIJ")
write sys$output f$extract(99,3,"ABCDEFGHIJ")
write sys$output f$extract(0,99,"ABCDEFGHIJ")

$!***********************
$!*     F$FAO           *
$!***********************
write sys$output f$fao("!4(10AS)","a","b","c","d")
write sys$output f$fao("!20%d",0)
write sys$output f$fao("!4(10OL)",1,-1,0,32768)
write sys$output f$fao("!4(10XL)",1,-1,0,32768)
write sys$output f$fao("!4(10ZL)",1,-1,0,32768)
write sys$output f$fao("!4(10UL)",1,-1,0,32768)
write sys$output f$fao("!4(10SL)",1,-1,0,32768)
write sys$output " "
write sys$output f$fao("!4(10OW)",1,-1,0,32768)
write sys$output f$fao("!4(10XW)",1,-1,0,32768)
write sys$output f$fao("!4(10ZW)",1,-1,0,32768)
write sys$output f$fao("!4(10UW)",1,-1,0,32768)
write sys$output f$fao("!4(10SW)",1,-1,0,32768)
write sys$output " "
write sys$output f$fao("!4(10OB)",1,-1,0,32768)
write sys$output f$fao("!4(10XB)",1,-1,0,32768)
write sys$output f$fao("!4(10ZB)",1,-1,0,32768)
write sys$output f$fao("!4(10UB)",1,-1,0,32768)
write sys$output f$fao("!4(10SB)",1,-1,0,32768)

$!***********************
$!*     F$INTEGER       *
$!***********************
ten = 10
write sys$output f$integer(ten)
write sys$output f$integer(list)
write sys$output f$integer(123)
write sys$output f$integer("123")
write sys$output f$integer("ABC")

$!***********************
$!*     F$LENGTH        *
$!***********************
write sys$output f$length("1234567890")
write sys$output f$length(list)
                  
$!***********************
$!*     F$LOCATE        *
$!***********************
write sys$output f$locate("aaa","bfdgdaaahfhfaaa")
write sys$output f$locate("xyz","bfdgdaaahfhfaaa")
write sys$output f$locate("ELEM3",list)

$!***********************
$!*     F$MODE          *
$!***********************
write sys$output f$mode()

$!***********************
$!*     F$NODE          *
$!***********************
write sys$output f$node()

$!***********************
$!*     F$PID           *
$!***********************
write sys$output f$pid()

$!***********************
$!*     F$PROCESS       *
$!***********************
write sys$output f$process()

$!***********************
$!*     F$UNIQUE        *
$!***********************
write sys$output f$unique()

$OPEN /WRITE TEMP_FILE F$UNIQUE()
$ DIRECTORY *.DAT
$ CLOSE /DISPOSITION=DELETE TEMP_FILE
$ DIRECTORY *.DAT
      

$!***********************
$!*     F$USER          *
$!***********************
write sys$output f$user()

$!***********************
$!*     F$STRING        *
$!***********************
write sys$output f$string(ten * 10)
write sys$output f$string(list)

$!***********************
$!*     F$TRNLNM        *
$!***********************
write sys$output f$trnlnm("SYS$LOGIN")

$!***********************
$!*     F$TYPE          *
$!***********************
write sys$output f$type(list)
write sys$output f$type(ten)
write sys$output f$type(f$time())

$!***********************
$!*     F$VERIFY        *
$!***********************
write sys$output f$verify()

$!***********************
$!*     F$CVTIME        *
$!***********************
write sys$output f$cvtime(,,)
write sys$output f$cvtime(,,"DATE")
write sys$output f$cvtime(,,"DATETIME")
write sys$output f$cvtime(,,"DAY")
write sys$output f$cvtime(,,"HOUR")
write sys$output f$cvtime(,,"HUNDRETH")
write sys$output f$cvtime(,,"MINUTE")
write sys$output f$cvtime(,,"MONTH")
write sys$output f$cvtime(,,"SECOND")
write sys$output f$cvtime(,,"TIME")
write sys$output f$cvtime(,,"WEEKDAY")
write sys$output f$cvtime(,,"YEAR")

write sys$output f$cvtime(,"ABSOLUTE","DATE")
write sys$output f$cvtime(,"ABSOLUTE","DATETIME")
write sys$output f$cvtime(,"ABSOLUTE","DAY")
write sys$output f$cvtime(,"ABSOLUTE","HOUR")
write sys$output f$cvtime(,"ABSOLUTE","HUNDRETH")
write sys$output f$cvtime(,"ABSOLUTE","MINUTE")
write sys$output f$cvtime(,"ABSOLUTE","MONTH")
write sys$output f$cvtime(,"ABSOLUTE","SECOND")
write sys$output f$cvtime(,"ABSOLUTE","TIME")
write sys$output f$cvtime("YESTERDAY","ABSOLUTE","WEEKDAY")
write sys$output f$cvtime("TODAY","ABSOLUTE","WEEKDAY")
write sys$output f$cvtime("TOMORROW","ABSOLUTE","WEEKDAY")
write sys$output f$cvtime(,"ABSOLUTE","YEAR")
write sys$output f$cvtime("+1-","ABSOLUTE","WEEKDAY")
write sys$output f$cvtime("+31-","ABSOLUTE","MONTH")

$!***********************
$!*     F$ENVIRONMENT   *
$!***********************
write sys$output f$environment("CONTROL")
write sys$output f$environment("DEFAULT")
write sys$output f$environment("DEPTH")
write sys$output f$environment("INTERACTIVE")
write sys$output f$environment("MAX_DEPTH")
write sys$output f$environment("PROCEDURE")
write sys$output f$environment("PROMPT")
write sys$output f$environment("SYMBOL_SCOPE")
write sys$output f$environment("VERB_SCOPE")
write sys$output f$environment("VERIFY_IMAGE")
write sys$output f$environment("VERIFY_PROCEDURE")
write sys$output f$environment("VERSION")

$!******************************
$!*     F$FILE_ATTRIBUTES      *
$!******************************
write sys$output f$file_attributes("lexical.dcl","ATT")
write sys$output f$file_attributes("lexical.dcl","DAT")
write sys$output f$file_attributes("lexical.dcl","DEV")
write sys$output f$file_attributes("lexical.dcl","DIR")
write sys$output f$file_attributes("lexical.dcl","EXT")
write sys$output f$file_attributes("lexical.dcl","NAM")
write sys$output f$file_attributes("lexical.dcl","SIZ")

$!********************
$!*     F$PARSE      *
$!********************
write sys$output "Parsing ""LEXICAL.DCL""
write sys$output "SYNTAX:" + f$parse("LEXICAL.DCL","sys$login:*.*",,,"SYNTAX_ONLY")
write sys$output "NODE  :" + f$parse("LEXICAL.DCL","sys$login:*.*",,"NODE","SYNTAX_ONLY")
write sys$output "DEV   :" + f$parse("LEXICAL.DCL","sys$login:*.*",,"DEVICE","SYNTAX_ONLY")
write sys$output "DIR   :" + f$parse("LEXICAL.DCL","sys$login:*.*",,"DIRECTORY","SYNTAX_ONLY")
write sys$output "NAME  :" + f$parse("LEXICAL.DCL","sys$login:*.*",,"NAME","SYNTAX_ONLY")
write sys$output "TYPE  :" + f$parse("LEXICAL.DCL","sys$login:*.*",,"TYPE","SYNTAX_ONLY")
write sys$output "VER   :" + f$parse("LEXICAL.DCL","sys$login:*.*",,"VERSION","SYNTAX_ONLY")

$!***********************
$!*     F$GETDVI        *
$!***********************
write sys$output f$getdvi("SYS$DISK","SIZ")
write sys$output f$getdvi("SYS$DISK","FRE")
write sys$output f$getdvi("SYS$DISK","VOL")
write sys$output f$getdvi("SYS$DISK","SYS")
write sys$output f$getdvi("SYS$DISK","TYP")
write sys$output f$getdvi("SYS$DISK","SER")

$!***********************
$!*     F$GETJPI        *
$!***********************
write sys$output f$getjpi("JOBTYPE")
write sys$output f$getjpi("MODE")
write sys$output f$getjpi("LOGINTIM")
write sys$output f$getjpi("MASTER_PID")
write sys$output f$getjpi("PID")
write sys$output f$getjpi("OWNER")
write sys$output f$getjpi("NODENAME")
write sys$output f$getjpi("USERNAME")

$!***********************
$!*     F$GETSYI        *
$!***********************
write sys$output f$getsyi("CPU")
write sys$output f$getsyi("SPEED")
write sys$output f$getsyi("TOTMEM")
write sys$output f$getsyi("FREMEM")
write sys$output f$getsyi("ARCH_TYPE")
write sys$output f$getsyi("AVAILCPU_CNT")
write sys$output f$getsyi("CODEPAGE")

$!***********************
$!*     F$MATCH_WILD     *
$!***********************
write sys$output f$match_wild ("This is a candidate","*c%%d*")
write sys$output f$match_wild ("This is a candidate text", "*candi*)
write sys$output f$match_wild ("This is a candidate text", "*kiki*)

$!***********************
$!*     F$MESSAGE       *
$!***********************
v = f$verify(0)
error = 0
loop:
    write sys$output error + "  " + f$message(error)
    error = error + 1
    if error .LT. 36 then goto loop
dummy = f$verify(v)

$!***********************
$!*     F$SEARCH        *
$!***********************
v = f$verify(0)
loop1:
    fn = f$search("*.dcl")
    write sys$output fn
    if fn .NES. "" then goto loop1
dummy = f$verify(v)
        
