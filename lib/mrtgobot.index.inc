#script library
PERL=/usr/bin/perl
MRTG=/opt/bin/mrtg
IDXM=/opt/bin/indexmaker

EXTIP=$(wget -qO- http://ipecho.net/plain ; echo)
PWD=$(dirname $0)
cd $PWD
for CFG in *.cfg ; do
	NAME=$(basename $CFG .cfg)
	echo ==== $NAME
	WORKDIR=$(grep -i workdir $CFG | cut -d: -f2 | sed 's/ //g')
	INDEX=$WORKDIR/index.php
	LOG="../log/$NAME.log"
	env LANG=C $IDXM \
	--title "$NAME mrtg"  \
	--subtitle "<code>External IP: $EXTIP<br />Generated on `date`</code>" \
	--nolegend \
	--show=week \
	--headeradd="<script src='//netdna.bootstrapcdn.com/bootstrap/3.1.1/js/bootstrap.min.js'></script><link href='../mrtg_bootstrap.css' rel='stylesheet'>" \
	--columns=2 \
	$CFG > $INDEX
done
