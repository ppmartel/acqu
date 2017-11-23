#!/bin/bash

if [ `ps aux | grep RunNewAcqu.sh | wc -l` -gt 3 ]
then
echo "Already running"
exit
fi

cd ~/acqu/acqu_user/

TIME=`date +'%Y%m%d %H%M%S' | awk '{print $1"_"$2-100}'`

FILES=`ls -lh --time-style=+'%Y%m%d%H%M%S' scratch/CBTaggTAPS* | awk -v time="$TIME" '{if ($6 < time) print $7}'`

for FILE in $FILES
do
GOAT=`echo $FILE | sed 's/scratch\//output\/Acqu_/' | sed 's/\.xz//' | sed 's/\.dat/\.root/'`
IGNORE=`echo $FILE | sed 's/scratch\//output\/ignored_files\/Acqu_/' | sed 's/\.xz//' | sed 's/\.dat/\.root/'`
if [ ! -e $GOAT ] && [ ! -e $IGNORE ]
then
while [ `ps aux | grep AcquRoot | wc -l` -gt 6 ]
do
echo "Six processes already running - waiting ten minutes" >> RunNewAcqu.log
sleep 600
done
echo "Running $FILE" >> RunNewAcqu.log
LOG=`echo $FILE | sed 's/scratch\//output\/Acqu_/' | sed 's/\.xz//' | sed 's/\.dat/\.log/'`
../build/bin/AcquRoot --nolog --batch data/Offline-CBTaggTAPS.dat --datafile $FILE 2>&1 > $LOG &
fi
done

FILES=`ls -lh --time-style=+'%Y%m%d%H%M%S' scratch/TaggEff* | awk -v time="$TIME" '{if ($6 < time) print $7}'`

for FILE in $FILES
do
GOAT=`echo $FILE | sed 's/scratch\//output\/Acqu_/' | sed 's/\.xz//' | sed 's/\.dat/\.root/'`
IGNORE=`echo $FILE | sed 's/scratch\//output\/ignored_files\/Acqu_/' | sed 's/\.xz//' | sed 's/\.dat/\.root/'`
if [ ! -e $GOAT ] && [ ! -e $IGNORE ]
then
while [ `ps aux | grep AcquRoot | wc -l` -gt 6 ]
do
echo "Six processes already running - waiting ten minutes" >> RunNewAcqu.log
sleep 600
done
echo "Running $FILE" >> RunNewAcqu.log
LOG=`echo $FILE | sed 's/scratch\//output\/Acqu_/' | sed 's/\.xz//' | sed 's/\.dat/\.log/'`
../build/bin/AcquRoot --nolog --batch data/Offline-TaggEff.dat --datafile $FILE 2>&1 > $LOG &
fi
done
