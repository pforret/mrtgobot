<?php
$debug=false;
$time_pagestarted=microtime(true);

function trace($info,$level="DEBUG"){
	global $debug;
	global $time_pagestarted;
	$level=strtoupper($level);
	if(!$debug AND $level=="DEBUG"){
		// only print info if in debug mode
		return false;
	}
	$secs=round(microtime(true) - $time_pagestarted,3);
	if(is_array($info)){
		if(PHP_SAPI=="cli"){
			printf("#- %s @ %06.3fs:\n",$level,$secs);
			print_r($info);
			echo "#-----\n";
		} else {
			// html formatted
			printf("\n<-- %s @ %06.3fs:\n",$level,$secs);
			print_r($info);
			echo "\n-->\n";
		}
	} else {
		$info=preg_replace('#([^-])([<>])([^-])#','\1 \2 \3',$info);
		if(PHP_SAPI=="cli"){
			printf("#- %s @ %06.3fs: %s\n",$level,$secs,$info);
		} else {
			printf("\n<-- %s @ %06.3fs: %s -->\n",$level,$secs,$info);
		}
	}
}

function warning($scope,$message,$exit=false){
	if($exit){
		$errmsg=sprintf("%8s: WARNING: %s: %s",date("H:i:s"),$scope,$message);
		error_log($errmsg);
		error_log("*** EXIT");
		exit(1);
	} else {
		$errmsg=sprintf("%8s: ERROR  : %s: %s",date("H:i:s"),$scope,$message);
		error_log($errmsg);
	}
}

function progress($scope,$message){
	$errmsg=sprintf("%8s: %s: %s",date("H:i:s"),$scope,$message);
	error_log($errmsg);
}

function cmdline($text,$folder=false,$cachesecs=30){
	if($folder){
		$path=realpath($folder);
		if(!$path){
			trace("cannot find folder [$folder]","ERROR");
			return false;
		}
		$line="cd \"$path\"; $text";
	} else {
		$line=$text;
	}
	if($cachesecs>0){
		$cc=New Cache;
		$result=$cc->get_arr($line,"bash",$cachesecs);
		trace("cmdline: [$line] = " . count($result) . " from cache");
		if($result)	return $result;
	}
	$stdout=Array();
	$result=exec("$line",$stdout);
	trace("cmdline: [$line] = " . count($stdout) . " lines returned");
	if($cachesecs>0 AND $stdout){
		$cc=New Cache;
		$cc->set_arr($line,"bash",$stdout);
	}
	return $stdout;
}

function getparam($name,$default=false){
	if(isset($_GET[$name])){
		$value=$_GET[$name];
		trace("getparam: [$name] = [" . htmlspecialchars(substr($value,0,20)) . "]");
		return $value;
	}
	$value=$default;
	trace("getparam: [$name] = [" . htmlspecialchars(substr($value,0,20)) . "] (default)");
	return $value;
}

function preg_find($pattern,$subject){
	preg_match($pattern,$subject,$matches);
	if($matches){
		trace($matches);
		return $matches[0][1];
	} else {
		trace("preg_find: $pattern not found");
		return false;
	}
}

?>
