#/bin/sh
VERSION='V407'

tar -cvz --exclude=.svn -f "bin/dcl2_Linux_X86_$VERSION.tar.gz" dcl2 changes.txt dcl.ini  license.txt  login.dcl logout.dcl examples/*.dcl help/*.hlp

tar -cvz --exclude=.svn -f "bin/dcl2_Linux_Curses_X86_$VERSION.tar.gz" dcl2c changes.txt dcl.ini  license.txt  login.dcl logout.dcl examples/*.dcl help/*.hlp

