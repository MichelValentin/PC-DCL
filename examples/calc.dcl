$ ! Procedure to calculate expressions. If you enter an
$ ! assignment statement, then CALC.COM evaluates the expression
$ ! and assigns the result to the symbol you specify. In the next
$ ! iteration, you can use either your symbol or the symbol Q to
$ ! represent the current result.
$ !
$ ! If you enter an expression, then CALC.COM evaluates the
$ ! expression and assigns the result to the symbol Q. In
$ ! the next iteration, you can use the symbol Q to represent
$ ! the current result.
$ !
$ SAVE_VERIFY_IMAGE = F$ENVIRONMENT("VERIFY_IMAGE")
$ SAVE_VERIFY_PROCEDURE = F$VERIFY(0)
$ START:
$ ON WARNING THEN GOTO START
$ INQUIRE STRING "Calc" 
$ IF STRING .EQS. "" THEN GOTO CLEAN_UP
$ IF F$LOCATE("=",STRING) .EQ. F$LENGTH(STRING) THEN GOTO EXPRESSION
$ !
$ ! Execute if string is in the form symbol = expression
$ STATEMENT:
$ 'STRING' ! Execute assignment statements
$ SYMBOL = F$EXTRACT(0,F$LOCATE("=",STRING)-1,STRING) ! get symbol name
$ Q='SYMBOL' ! Set up q for future iterations
$ LINE = F$FAO("Decimal = !SL Hex = !-!XL Octal = !-!OL",Q)
$ WRITE SYS$OUTPUT LINE
$ GOTO START
$ !
$ !
$ ! Execute if string is an expression
$ EXPRESSION:
$ Q = F$INTEGER('STRING') ! Can use Q in next iteration
$ LINE = F$FAO("Decimal = !SL Hex = !-!XL Octal = !-!OL",Q)
$ WRITE SYS$OUTPUT LINE
$ GOTO START
$ !
$ CLEAN_UP:
$ SAVE_VERIFY_PROCEDURE = F$VERIFY(SAVE_VERIFY_PROCEDURE,SAVE_VERIFY_IMAGE)
$ EXIT