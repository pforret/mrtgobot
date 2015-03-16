<?php
include_once("lib/settings.inc");
$shortopts=Array(
	"v",	// verbose
	"s",	// silent
	"?",	// help/usage
	);
$options = getopt(implode("",$shortopts));
print_r($options);

if(isset($options["v"]))	$debug=true;

/*
mrtgobot.php SETUP [FILE.cfg]

mrtgobot.php INDEX [FILE.cfg|FOLDER]

mrtgobot.php RUN [FILE.cfg|FOLDER]

mrtgobot.php CRONTAB [FILE.cfg|FOLDER]

mrtgobot.php RSYNC [FILE.cfg|FOLDER]
	we need a #RSYNC TARGET:[url] in the CFG file

mrtgobot.php PROBE [SENSOR] [OPTIONS]
	[SENSOR] can be [CPU|LOAD|MEM|DISK|FOLDERSIZE|FILESIZE|NETWORK]

*/
print_r($argv);

if(isset($argv[1])){
	$action=strtoupper($argv[1]);
} else {
	$action="HELP";
}
if(isset($options["?"]))	$action="HELP";

switch($action){
case "HELP":
case "USAGE":
	show_usage();
	exit(0);
	break;;
	
case "SETUP":
	break;;
	
case "RUN":
	break;;
	
case "INDEX":
	break;;
	
case "RSYNC":
	break;;
	
case "PROBE":
	break;;
	
default:
	// unknown command
}

function show_usage(){
	echo "
mrtgobot.php [ACTION] [PARAMS]

mrtgobot.php SETUP [config.cfg]
	create base MRTG config
";
}
?>