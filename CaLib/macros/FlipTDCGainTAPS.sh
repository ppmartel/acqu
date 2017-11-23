#!/bin/bash

### Change these lines for your calibration ###

DBASE="CaLib_2013_10-Present"
CALIB="2014_04_Compton_Martel"

### Change these lines if on another system ###

HOST="a2calib.online.a2.kph"
USER="calib"
PASS="basel10"

### Do NOT change these lines ###

mysqldump -u $USER -p$PASS --host=$HOST --databases $DBASE --tables taps_t1 veto_t1 --no-create-info --replace --where="calibration='$CALIB'" > Old.sql

sed '/REPLACE INTO /s/,-0\./,0\./g' Old.sql > New.sql

echo
echo "/////////////////////"
echo "// Old TAPS Values //"
echo "/////////////////////"
echo
grep "REPLACE INTO" Old.sql | grep taps_t1 | sed 's/;//g' | sed 's/ /;/g' | sed 's/)//g' | sed 's/(//g' | sed 's/,/ /g' | awk '{x=3; y=0; while ( x<=NF ) {if ( y==0 ) {print "Files "$x" to "$(x+1);x+=3;y++} else if ( y<=510 ) {if ( $x!=0 ) printf "%.6f  ",$x;x++;y++} else {printf "\n\n";y=0;x+=2}}; printf "\n\n"}'

echo "/////////////////////"
echo "// New TAPS Values //"
echo "/////////////////////"
echo
grep "REPLACE INTO" New.sql | grep taps_t1 | sed 's/;//g' | sed 's/ /;/g' | sed 's/)//g' | sed 's/(//g' | sed 's/,/ /g' | awk '{x=3; y=0; while ( x<=NF ) {if ( y==0 ) {print "Files "$x" to "$(x+1);x+=3;y++} else if ( y<=510 ) {if ( $x!=0 ) printf "%.6f  ",$x;x++;y++} else {printf "\n\n";y=0;x+=2}}; printf "\n\n"}'

echo "/////////////////////"
echo "// Old Veto Values //"
echo "/////////////////////"
echo
grep "REPLACE INTO" Old.sql | grep veto_t1 | sed 's/;//g' | sed 's/ /;/g' | sed 's/)//g' | sed 's/(//g' | sed 's/,/ /g' | awk '{x=3; y=0; while ( x<=NF ) {if ( y==0 ) {print "Files "$x" to "$(x+1);x+=3;y++} else if ( y<=510 ) {if ( $x!=0 ) printf "%.6f  ",$x;x++;y++} else {printf "\n\n";y=0;x+=2}}; printf "\n\n"}'

echo "/////////////////////"
echo "// New Veto Values //"
echo "/////////////////////"
echo
grep "REPLACE INTO" New.sql | grep veto_t1 | sed 's/;//g' | sed 's/ /;/g' | sed 's/)//g' | sed 's/(//g' | sed 's/,/ /g' | awk '{x=3; y=0; while ( x<=NF ) {if ( y==0 ) {print "Files "$x" to "$(x+1);x+=3;y++} else if ( y<=510 ) {if ( $x!=0 ) printf "%.6f  ",$x;x++;y++} else {printf "\n\n";y=0;x+=2}}; printf "\n\n"}'

echo -n "Change database values? (Confirm with y, Y, yes, Yes, or YES; otherwise cancel) - "
read CHANGE
if [ "$CHANGE" == "y" -o "$CHANGE" == "Y" -o "$CHANGE" == "yes" -o "$CHANGE" == "Yes" -o "$CHANGE" == "YES" ]
then
mysql -u $USER -p$PASS --host=$HOST $DBASE < New.sql
echo
echo "//////////////////////"
echo "// Database changed //"
echo "//////////////////////"
echo
fi
