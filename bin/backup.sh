#!/bin/bash
BAK1=`date +%m%d%Y` # Get current date
BAK2=`date +%H%M`   # Get current time
DATE=`echo ${BAK1}_${BAK2}` # String the two variables together

#echo ${BAK1}_${BAK2} # For test only
#echo $DATE           # For test only

OLD_PWD=`pwd`         # Get the current working directory

#echo $OLD_PWD        # For test only

cd $HOME              # Go to the home directory
tar --exclude *.o --exclude *.ex --gzip -cvf podnuts_$DATE.tgz podnuts podnuts.log # Create the backup file
mv podnuts_$DATE.tgz $HOME/backup # Move the backup file to the backup directory
cd $OLD_PWD # Go back to whatever directory we started in
