write sys$output "Parsing ""LEXICAL.DCL""
write sys$output "SYNTAX:" + f$parse("LEXICAL.DCL","sys$login:*.*",,,"SYNTAX_ONLY")
write sys$output "NODE  :" + f$parse("LEXICAL.DCL","sys$login:*.*",,"NODE","SYNTAX_ONLY")
write sys$output "DEV   :" + f$parse("LEXICAL.DCL","sys$login:*.*",,"DEVICE","SYNTAX_ONLY")
write sys$output "DIR   :" + f$parse("LEXICAL.DCL","sys$login:*.*",,"DIRECTORY","SYNTAX_ONLY")
write sys$output "NAME  :" + f$parse("LEXICAL.DCL","sys$login:*.*",,"NAME","SYNTAX_ONLY")
write sys$output "TYPE  :" + f$parse("LEXICAL.DCL","sys$login:*.*",,"TYPE","SYNTAX_ONLY")
write sys$output "VER   :" + f$parse("LEXICAL.DCL","sys$login:*.*",,"VERSION","SYNTAX_ONLY")
