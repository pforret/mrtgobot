#script library
PERL=/usr/bin/perl
MRTG=/opt/bin/mrtg
IDXM=/opt/bin/indexmaker

PWD=$(dirname $0)
cd $PWD
echo "$(date) - [$0] as [$USER]" >> ../log/cron_mrtg.log
for CFG in *.cfg ; do
	NAME=$(basename $CFG .cfg)
	echo ==== $NAME
	LOG="../log/$NAME.log"
	env LANG=C $PERL $MRTG $CFG --logging $LOG
done
