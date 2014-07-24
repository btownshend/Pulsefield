#!/bin/sh 
# Run tests, saving results in performance.csv
FRAMERANGE=2000
TESTS=(fixedperson.ferec notmoving.ferec multi1.ferec multi3.ferec multi4.ferec multi6.ferec multi7.ferec ghost.ferec zero1-4541.ferec)
/bin/echo -n "Notes? " 
read notes
{
    echo '------- Running tests '$(date) --------
    echo "Notes:  $notes"
    git diff
} >> testing.log    
echo >>performance.csv
for test in ${TESTS[*]}
do
    ./frontend -P -p ../Recordings/$test -x10 -F $FRAMERANGE -B 8 -c "$notes"
done
echo '---------- Done '$(date) ------- >>testing.log
