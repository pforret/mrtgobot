<?php
include_once("class_cache.inc");

Class SysTools{

	var $sapi=false;
	var $os=false;
	var $eol=false;
	var $debug=false;
	var $inittime=microtime(true);
	
	function __construct(){
		
		if(PHP_SAPI=="cli"){
			$this->sapi="cli";
			$this->eol=PHP_EOL;
		} else {
			$this->sapi="web";
			$this->eol="\n";
		}
		switch(PHP_OS){
			case "WINNT":	$this->os="windows";	break;;
			case "Darwin":	$this->os="mac";	break;;
			case "Linux":	$this->os="linux";	break;;
		}
	}
	
	function trace($info,$level="DEBUG"){
		// level = DEBUG: only show when $debug = true
		// level = WARNING: always show
		// level = ERROR: always show & stop program afterwards
		$level=strtoupper($level);
		if(!$this->$debug AND $level=="DEBUG"){
			// not in debug mode: show nothing
			return false;
		}
		$secs=round(microtime(true) - $this->inittime,3);
		$isarray=is_array($info);
		$isobject=is_object($info);
		$isresource=is_resource($info);
		$isboolean=is_boolean($info);
		$istext=(!$isarray AND !$isobject AND !$boolean AND !$isresource);
		switch(true){
		case $this->sapi == "cli" AND $isboolean:
			if($info){		$infoval="true";	
			} else {		$infoval="false";}
			$line=sprintf("#- %s @ %06.3fs: [%s]",$level,$secs,$infoval);
			fwrite(STDERR,$line . $this->eol);
			break;;
			
		case $this->api == "cli" AND $istext:
			//$info=preg_replace('#([^-])([<>])([^-])#','\1 \2 \3',$info);
			$line=sprintf("#- %s @ %06.3fs: [%s]",$level,$secs,$info);
			fwrite(STDERR,$line . $this->eol);
			break;;
			
		case $this->api == "cli" AND $isarray:
			$lines[]=printf("#- %s @ %06.3fs:",$level,$secs);
			$lines[]=print_r($info,true);
			$lines[]="#-----";
			foreach($lines as $line){
				fwrite(STDERR,$line . $this->eol);
			}
			break;;
			
		case $this->api == "cli" AND $isobject:
			$lines[]=printf("#- %s @ %06.3fs:",$level,$secs);
			$lines[]=var_export($info,true);
			$lines[]="#-----";
			foreach($lines as $line){
				fwrite(STDERR,$line . $this->eol);
			}
			break;;
			
		case $this->sapi <> "cli" AND $isboolean:
			if($info){		$infoval="true";	
			} else {		$infoval="false";}
			$line=sprintf("<-- %s @ %06.3fs: [%s] -->",$level,$secs,$infoval);
			fwrite(STDERR,$line . $this->eol);
			break;;
			
		case $this->sapi <> "cli" AND $istext:
			$line=sprintf("<-- %s @ %06.3fs: [%s] -->",$level,$secs);
			fwrite(STDERR,$line . $this->eol);
			break;;
			
		case $this->api <> "cli" AND $isarray:
			$lines[]=printf("#<-- %s @ %06.3fs:",$level,$secs);
			$lines[]=print_r($info,true);
			$lines[]="-->";
			foreach($lines as $line){
				fwrite(STDERR,$line . $this->eol);
			}
			break;;
			
		case $this->api <> "cli" AND $isobject:
			$lines[]=printf("#<-- %s @ %06.3fs:",$level,$secs);
			$lines[]=var_export($info,true);
			$lines[]="-->";
			foreach($lines as $line){
				fwrite(STDERR,$line . $this->eol);
			}
			break;;
				
		default:
			// do nothing
		}
		if($level == "ERROR"){
			if($this->api == "cli"){
				fwrite(STDERR, "#- ****** ERROR - STOP PROGRAM *****" . $this->eol);
			} else {
				fwrite(STDERR, "<!-- ***** ERROR - STOP PROGRAM ***** -->" . $this->eol);
			}
			exit(1); // A response code other than 0 is a failure
		}
	}

	function cmdline($text,$folder=false,$cachesecs=30){
		if($folder){
			$path=realpath($folder);
			if(!$path){
				trace("SysTools::cmdline : cannot find folder [$folder]","ERROR");
				return false;
			}
			$line="cd \"$path\"; $text";
		} else {
			$line=$text;
		}
		if($cachesecs>0){
			$cc=New Cache;
			$result=$cc->get_arr($line,"cmd",$cachesecs);
			trace("SysTools::cmdline: [$line] = " . count($result) . " lines from cache");
			if($result)	return $result;
		}
		$stdout=Array();
		$result=exec("$line",$stdout);
		trace("SysTools::cmdline: [$line] = " . count($stdout) . " lines returned");
		if($cachesecs>0 AND $stdout){
			$cc=New Cache;
			$cc->set_arr($line,"cmd",$stdout);
		}
		return $stdout;
	}


}

?>