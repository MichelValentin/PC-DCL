$ ! Procedure to accumulate programmer names and document file names.
$ ! After all programmer names and file names are entered, they are
$ ! sorted in alphabetic order by programmer name and printed on
$ ! the system printer.
$ !
$! SAVE_VERIFY_IMAGE = F$ENVIRONMENT("VERIFY_IMAGE")
$! SAVE_VERIFY_PROCEDURE = F$VERIFY(0)
$ !
$ OPEN/WRITE OUTFILE DATA.TMP ! Create output file "
$ !
$ ! Loop to obtain programmers’ last names and file names,
$ ! and write this data to DATA.TMP.
$ !
$ LOOP:
$ INQUIRE NAME "Programmer (press Return to quit)"
$ IF NAME .EQS. "" THEN GOTO FINISHED
$ INQUIRE FILE "Document file name"
$ RECORD := ""
$ RECORD[0,20]:='NAME'
$ RECORD[21,20]:='FILE'
$ WRITE OUTFILE RECORD
$ GOTO LOOP
$ FINISHED:
$ CLOSE OUTFILE
$ !
$ OPEN/WRITE OUTFILE DOCUMENT.DAT
$ WRITE OUTFILE "Programmer Files as of ",F$TIME()
$ WRITE OUTFILE ""
$ RECORD := ""
$ RECORD[0,20]:="Programmer Name"
$ RECORD[21,20]:="File Name"
$ WRITE OUTFILE RECORD
$ WRITE OUTFILE ""
$ !
$ CLOSE OUTFILE
$ APPEND DATA.TMP DOCUMENT.DAT
$ type DOCUMENT.DAT
$ !
$ INQUIRE CLEAN_UP "Delete temporary files [Y,N]"
$ IF CLEAN_UP THEN DELETE DATA.TMP;*,DOC.SRT;*
$ SAVE_VERIFY_PROCEDURE = F$VERIFY(SAVE_VERIFY_PROCEDURE,SAVE_VERIFY_IMAGE)
$ EXIT