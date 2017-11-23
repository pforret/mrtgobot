<?php
include_once("tools.inc");
include_once("class_ostools.inc");
include_once("class_cache.inc");

Class Sensor{

	var $params=Array();
	var	$fparts=Array();
	var	$sparts=Array();
	
	function __construct($type=false){
		$c =New Cache;
		$ss=New OStools();
		$urlparts=Array();
		if (php_sapi_name() == "cli") {
			// In cli-mode
			$hostname=cmdline("hostname",false,3600,true);
			$ipaddress=cmdline("ifconfig | grep 'inet ' | grep -v 127.0.0.1 | cut -d: -f2 | cut -d' ' -f1 | head -1",false,3600,true);
		} else {
			// Not in cli-mode - so through web server
			$hostname=gethostname();
			$ipaddress=$c->get("ip","network");
			if(!$ipaddress){
				$ipaddress=gethostbyname($hostname);
				$c->set("ip","network",$ipaddress);
			}
		}
		$system=php_uname();
		$this->add_namepart($hostname);
		$this->params["server"]=$hostname;
		$this->params["uptime"]=$ss->uptime();
		$this->params["mrtg_kmg"]=",k,M,G,T,P";
		if(!$type)	$type="test";
		$this->add_namepart($type);
		$this->params["sys_name"]=$hostname;
		$this->params["sys_address"]=$ipaddress;
		$this->params["sys_details"]=$system;
		$this->params["mrtg_options"]="";
		$this->params["config"]="";
		}

	function add_namepart($text){
		$this->fparts[]=$text;
		$this->sparts[]=substr(preg_replace("#([^\w\d])#","",$text),0,8);
		$fname=implode(" ",$this->fparts);
		$sname=implode("_",$this->sparts);
		$uniq=substr(sha1($fname),0,3);
		$this->params["mrtg_name"]="$sname.$uniq";
		trace("name is now [$sname.$uniq]");
	}
	function set_uptime($text){
		$this->params["uptime"]=$text;
	}
	
	function set_server($text){
		$this->params["server"]=$text;
	}
	function mrtg_output($params,$withconfig=false){
		//header("Content-Type: text/plain; charset=utf-8");
		trace("--- mrtg_output start");
		if(!isset($params["uptime"])){
			$params["uptime"]=$this->params["uptime"];
		}
		if(!isset($params["mrtg_name"])){
			$params["mrtg_name"]=$this->params["mrtg_name"];
		}
		trace($params);
		echo trim($params["value1"]) . "\n";
		echo trim($params["value2"]) . "\n";
		echo trim($params["uptime"]) . "\n";
		echo trim($params["server"]) . "\n";
		if(!$withconfig){
			echo $params["config"] . "\n";
		} else {
			$name=$params["mrtg_name"];
			$name=str_replace("%","p",$name);
			echo "#### MRTG CONFIG $name ####\n";
			echo "#### copy this to your .cfg MRTG config file\n";
			echo "Target[$name]: $params[cmdline]\n";
			echo "Title[$name]: $params[title]\n";
			echo "PageTop[$name]: <h1>$params[title]</h1>\n";
			echo "LegendI[$name]: $params[name1]\n";
			echo "LegendO[$name]: $params[name2]\n";
			echo "YLegend[$name]: $params[mrtg_unit]\n";
			echo "PNGTitle[$name]: $name\n";
			echo "ShortLegend[$name]: $params[mrtg_unit]\n";
			echo "Options[$name]: $params[mrtg_options]\n";
			echo "MaxBytes[$name]: $params[mrtg_maxbytes]\n";
			if(isset($params["mrtg_kmg"])){
				echo "kMG[$name]: $params[mrtg_kmg]\n";	
			}
		}
	}

	function cpuusage($aspercent=true){
		$ss=New OStools();
		$result=$ss->cpuload($aspercent);
		$cpuinfo=$ss->cpuinfo();
		if($result){
			$load1=$result["cpu_1min"];
			$load5=$result["cpu_5min"];
			$load15=$result["cpu_15min"];
			$nbcpu=$cpuinfo["cpu_cores"];
			$this->params["server"]="$nbcpu core(s) x $cpuinfo[cpu_ghz] GHz (bogomips $cpuinfo[bogomips])";
			if(!$aspercent){
				$this->params["value1"]=$load5;
				$this->params["value2"]=$load15;
				$this->params["name1"]="Avg load over 5 min";
				$this->params["name2"]="Avg load over 15 min";
				$this->params["description"]="CPU load (5/15 min - $nbcpu CPUs)";
				$this->params["mrtg_unit"]="load";
				$this->params["mrtg_options"].=",gauge";
				$this->params["mrtg_maxbytes"]="10000";
				$this->params["title"]="CPU " . $cpuinfo["netw_host"];
			} else {
				$this->add_namepart("p");
				$this->params["value1"]=$load5;
				$this->params["value2"]=$load15;
				$this->params["name1"]="% used - 5 min";
				$this->params["name2"]="% used - 15 min";
				$this->params["description"]="CPU usage % (5/15 min - $nbcpu CPUs)";
				$this->params["mrtg_unit"]="%";
				$this->params["mrtg_options"].=",gauge,nopercent";
				$this->params["mrtg_maxbytes"]="100";
				$this->params["title"]="CPU% " . $cpuinfo["netw_host"];
			}
			return $this->params;
		} else {
			return false;
		}

	}

	function memusage($aspercent=false){
	// >free
	//               total         used         free       shared      buffers
	//   Mem:       249184       214376        34808            0        47724
	//  Swap:      2097144       188224      1908920
	// Total:      2346328       402600      1943728
		$ss=New OStools();
		$result=$ss->memload($aspercent);
		$cpuinfo=$ss->cpuinfo();
		if(!$aspercent){
			$this->params["value1"]=$result["mem_used"];
			$this->params["value2"]=$result["mem_total"];
			$this->params["name1"]="Used RAM";
			$this->params["name2"]="Total RAM";
			$this->params["description"]="Memory usage (used/total)";
			$this->params["mrtg_unit"]="B";
			$this->params["mrtg_options"].=",gauge";
			$this->params["mrtg_maxbytes"]=$total;
			$this->params["title"]="Memory " . $cpuinfo["netw_host"];
		} else {
			$this->add_namepart("p");
			$this->params["value1"]=round($result["mem_used"]*100/$result["mem_total"]);
			$this->params["value2"]=100;
			$this->params["name1"]="% RAM used";
			$this->params["name2"]="100%";
			$this->params["description"]="Memory usage (%)";
			$this->params["mrtg_unit"]="%";
			$this->params["mrtg_options"].=",gauge,nopercent";
			$this->params["mrtg_maxbytes"]=100;
			$this->params["title"]="Memory% " . $cpuinfo["netw_host"];
		}
		return $this->params;
	}

	function diskusage($path=false,$aspercent=false){
		if(!$path)	$path=".";
		if(!file_exists($path)){
			trace("diskusage: cannot find [$path]");
			//return false;
		}
		$ss=New OStools();
		$result=$ss->diskfree($path);
		$cpuinfo=$ss->cpuinfo();
		$this->add_namepart($result["disk_drive"]);

		if(!$aspercent){
			$this->params["value1"]=$result["disk_used"];
			$this->params["value2"]=$result["disk_total"];
			$this->params["name1"]="Used on " . $result["disk_name"];
			$this->params["name2"]="Total on " . $result["disk_name"];
			$this->params["description"]="DISK: " . $result["disk_name"];
			$this->params["title"]="DISK: " . $result["disk_name"];
			$this->params["mrtg_unit"]="B";
			$this->params["mrtg_options"].=",gauge";
			$this->params["mrtg_maxbytes"]=$result["disk_total"];
		} else {
			$this->params["value1"]=round($result["disk_used"]/$result["disk_total"]);
			$this->params["value2"]=100;
			$this->params["name1"]="Used disk %";
			$this->params["name2"]="100%";
			$this->params["description"]="DISK%: " . $result["disk_name"];
			$this->params["title"]="DISK%: " . $result["disk_name"];
			$this->params["mrtg_unit"]="%";
			$this->params["mrtg_options"].=",gauge";
			$this->params["mrtg_maxbytes"]=100;
		}
		return $this->params;
	}

	function foldersize($folder,$options){
	// 1043015852032   /share/MASTER/MASTER/
		trace("foldersize:  checking [$folder]");
		if(!file_exists($folder)){
			trace("foldersize: cannot find [$folder]");
			return false;
		}
		$result=cmdline("du -skD $folder",false,60*15);
		if($result){
			$line=$result[0];
			$line=preg_replace("#\s\s*#","\t",$line);
			list($size,$path)=explode("\t",$line);
			if(!$aspercent){
				$this->params["value1"]=$size;
				$this->params["value2"]="0";
				$this->params["name1"]="Folder size";
				$this->params["name2"]="";
				$this->params["description"]="Folder Size [$folder]";
				$this->params["mrtg_unit"]="B";
				$this->params["mrtg_options"].=",gauge,noo,nopercent";
				$this->params["mrtg_maxbytes"]=1000000000;
				$this->params["mrtg_kmg"]="k,M,G,T,P";
			} else {
				$this->params["value1"]=$size;
				$this->params["value2"]="0";
				$this->params["name1"]="Folder size";
				$this->params["name2"]="";
				$this->params["description"]="Folder Size [$folder]";
				$this->params["mrtg_unit"]="B";
				$this->params["mrtg_options"].=",gauge,noo,nopercent";
				$this->params["mrtg_maxbytes"]=1000000000;
				$this->params["mrtg_kmg"]="k,M,G,T,P";
			}
			return $this->params;
		} else {
			return false;
		}
	}

	function filecount($folder,$options){
		if(!file_exists($folder)){
			return false;
		}
		$findopt="";
		$params=$this->parse_options($options);
		if(!$params["recursive"])	$findopt.="-maxdepth 1 ";
		if($params["mtime"]) $findopt.="-mtime " . $params["mtime"] . " ";
		if($params["name"]) $findopt.="-name " . $params["name"] . " ";
		$findopt.="-type f ";
		$result=cmdline("find $folder $findopt | wc -l");
		if($result){
			$line=$result[0];
			$nb=(int)trim($line);
			$desc="File count [$folder]";
			if($options)	$desc.=" [$options]";
			$this->params["value1"]=$nb;
			$this->params["value2"]="0";
			$this->params["name1"]=$desc;
			$this->params["name2"]="";
			$this->params["description"]=$desc;
			$this->params["mrtg_unit"]="file(s)";
			$this->params["mrtg_options"].=",gauge,noo,nopercent";
			$this->params["mrtg_maxbytes"]=1000000;
			$this->params["mrtg_kmg"]=",k,M,G,T,P";
			return $this->params;
		} else {
			return false;
		}
	}
	
	function proccount($filter){
		if($filter){
			$filter=$this->sanitize($filter);
			$result=cmdline("ps | grep \"$filter\" | wc -l");
		} else {
			$result=cmdline("ps | wc -l");
		}
                if($result){
			$desc="Process count";
			if($filter)	$desc.=" [$filter]";
                        $line=$result[0];
                        $nb=(int)trim($line);
			if($filter){
				$nb=$nb-1; // remove the 'grep' process we created ourselves
			} else {
				$nb=$nb-3; // remove 1st line, and our own 'ps' and 'wc' process
			}
                        $this->params["value1"]=$nb;
                        $this->params["value2"]="0";
                        $this->params["name1"]=$desc;
                        $this->params["name2"]="";
                        $this->params["description"]=$desc;
                        $this->params["mrtg_unit"]="proc";
                        $this->params["mrtg_options"].=",gauge,noo,nopercent";
                        $this->params["mrtg_maxbytes"]=1000000;
                        $this->params["mrtg_kmg"]=",k,M,G,T,P";
                        return $this->params;
                } else {
                        return false;
                }
	}

	function pingtime($address,$port=80){
		if(!$address)	return false;
		for($i=0;$i<4;$i++){
			// result in seconds
			$result[$i]=$this->tcpping($address,$port);
		}
		if($result){
			$desc="ping time to $address:$port";
			$min=min($result);
			$max=max($result);
			$this->params["value1"]=(int)($min*1000000);
			$this->params["value2"]=(int)($max*1000000);
			$this->params["name1"]="MIN $desc";
			$this->params["name2"]="MAX $desc";
			$this->params["description"]=$desc;
			$this->params["mrtg_unit"]="sec";
			$this->params["mrtg_options"].=",gauge,nopercent";
			$this->params["mrtg_maxbytes"]=4000000;
			$this->params["mrtg_kmg"]="u,m,,k,M,G";
			return $this->params;
		} else {
			return false;
		}
	}

	function foldercount($folder,$options){
		if(!file_exists($folder)){
			return false;
		}
		$findopt="";
		$params=$this->parse_options($options);
		if(!$params["recursive"])	$findopt.="-maxdepth 1 ";
		if($params["mtime"]) $findopt.="-mtime " . $params["mtime"] . " ";
		if($params["name"]) $findopt.="-name " . $params["name"] . " ";
		$findopt.="-type d ";
		$result=cmdline("find $folder $findopt | wc -l");
		if($result){
			$line=$result[0];
			$nb=(int)trim($line);
			$desc="Folder count [$folder]";
			if($options)	$desc.=" [$options]";
			$this->params["value1"]=$nb;
			$this->params["value2"]="0";
			$this->params["name1"]=$desc;
			$this->params["name2"]="";
			$this->params["description"]=$desc;
			$this->params["mrtg_unit"]="folder(s)";
			$this->params["mrtg_options"].=",gauge,noo,nopercent";
			$this->params["mrtg_maxbytes"]=1000000;
			$this->params["mrtg_kmg"]=",k,M,G,T,P";
			return $this->params;
		} else {
			return false;
		}
	}

	// ---------- INTERNAL FUNCTIONS

	function parse_options($options){
		// parse key1=val1,key2=val2,key3
		if(!$options)	return false;
		$results=Array();
		trace("parse_options: $options");
		$params=explode(",",$options);
		foreach($params as $param){
			$param=$this->sanitize($param);
			if(strstr($param,"=")){
				list($key,$val)=explode("=",$param,2);
			} else {
				$key=$param;
				$val=$param;
			}
			$result[$key]=$val;
			trace("Option: [$key] = [$val]");
		}
		return $result;	
	}
	
	function parse_cmd($cmd,$folder=false,$cachesecs=300,$linenr=1,$fieldnr=1){
		// to replace awk, tail, head, ...
		$result=cmdline($cmd,$folder,$cachesecs);
	}


	function sanitize($text){
		// remove all nasty stuff before passing to bash/sh
		$result=$text;
		$result=str_replace(Array(";",'"'),"",$result);
		return $result;
	}
	
	function tcpping($host,$port=80){
		$timeout=4;
		if(contains($host,":")){
			list($host,$port)=explode(":",$host);
			$port=(int)$port;
		}
		if(!$port) 	return 5555;
		$t1 = microtime(true); 
		$fP = fSockOpen($host, $port, $errno, $errstr, $timeout); 
		if (!$fP) { 
			trace("tcpping: cannot connect to $host:$port - $errstr");
			return 6666; 
			}
		$t2 = microtime(true); 
		trace("TCP PING [$host:$port] - in " . round(($t2 - $t1)*1000) . "msec");
		return ($t2 - $t1); 
	}
}

?>
