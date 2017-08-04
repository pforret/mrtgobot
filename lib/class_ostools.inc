<?php
include_once("tools.inc");

Class OStools {

	function __construct(){
		$this->os=PHP_OS;
		$this->uname=php_uname("s");
		$this->version=php_uname("v");
		$this->host=gethostname();
		//print_r($this);
		$this->info["OS"]=PHP_OS;
		$this->info["uname"]=php_uname("s");
		$this->info["OS_version"]=php_uname("v");
		$this->info["netw_host"]=gethostname();
		$this->info["netw_ip"]=gethostbyname(gethostname());
		$this->uptime();
		trace($this->info);
	}

	function cpuload($aspercent=true){
		$this->info["cpu_1min"]=0;
		$this->info["cpu_5min"]=0;
		$this->info["cpu_15min"]=0;
		switch($this->os){
		case "WINNT":
			trace("OSSensor::cpuload : Windows method - Win32_Processor");
			$wmi = new COM("Winmgmts://");
            $server = $wmi->execquery("SELECT LoadPercentage FROM Win32_Processor");
            $cpu_num = 0;
            $load_total = 0;
            $load_min=10000;
            foreach($server as $cpu){
                $cpu_num++;
                $load_total += $cpu->loadpercentage;
                $load_min=min($load_min,$cpu->loadpercentage);
            }
            $load = round($load_total/$cpu_num);
			$this->info["cpu_load"]=round($load,3);
			$this->info["cpu_1min"]=round($load,3);
			$this->info["cpu_5min"]=round(($load+$load_min)/2,3);
			$this->info["cpu_15min"]=round(($load+2*$load_min)/3,3);
			break;;
		case "Linux":
			if($aspercent){
				$secs=2;
				$lines=cmdline("(cat /proc/stat ; sleep $secs ; cat /proc/stat) | awk '/cpu / {print $2+$3+$4+$7+$8+$9, $5,$6}'");
				trace($lines);
				/*
				[0] => 654149 379971 38391
				[1] => 654354 379971 38391
				*/
				list($busy1,$idle1,$wait1)=explode(" ",$lines[0]);
				list($busy2,$idle2,$wait2)=explode(" ",$lines[1]);
				$dbusy=$busy2-$busy1;
				$didle=$idle2+$wait2-$idle1-$wait1;
				$dtotal=$dbusy+$didle;
				trace("busy: $dbusy; idle=$didle");
				$this->info["cpu_load"]=round(100*$dbusy/$dtotal);
				$this->info["cpu_1min"]=round(100*$dbusy/$dtotal);
				$this->info["cpu_5min"]=round(100*$dbusy/$dtotal);
				$this->info["cpu_15min"]=round(100*$dbusy/$dtotal);
			} else {
				$load=sys_getloadavg();
				$this->info["cpu_load"]=round($load[0],3);
				$this->info["cpu_1min"]=round($load[0],3);
				$this->info["cpu_5min"]=round($load[1],3);
				$this->info["cpu_15min"]=round($load[2],3);
			}
			break;;
			
		case "Darwin":
			trace("OSSensor::cpuload : Linux/MacOSX method - sys_getloadavg");
			$load=sys_getloadavg();
			$this->info["cpu_load"]=round($load[0],3);
			$this->info["cpu_1min"]=round($load[0],3);
			$this->info["cpu_5min"]=round($load[1],3);
			$this->info["cpu_15min"]=round($load[2],3);
			break;;
		default:
			// nothing to do
		}
		trace("OSSensor::cpuload : load is " . $result["1min"]);
		trace($this->info);
		return $this->info;
	}

	function cpuinfo(){
		$this->info["cpu_cores"]=1;
		$this->info["cpu_mhz"]=1000;
		$this->info["cpu_ghz"]=1;
		$this->info["bogomips"]=1000;
		$this->info["cpu_hardware"]="Unknown";
		$this->info["cpu_type"]="Unknown";
		$this->info["cpu_procs"]=0;
		$numCpus = 1;

		switch(true){
		case is_file('/proc/cpuinfo'):
			// Linux machines
			trace("OSSensor::cpuinfo : Linux method: /proc/cpuinfo");
			/*
			Single Core ARM:
			Processor name  : Feroceon 88F6282 rev 1 (v5l) @ 1.6 GHz
			BogoMIPS        : 1589.24
			Processor       : Feroceon 88FR131 rev 1 (v5l)
			BogoMIPS        : 1589.24
			Features        : swp half thumb fastmult edsp
			Hardware        : Synology 6282 board
			
			Dual Core Intel:
			vendor_id       : GenuineIntel
			model name      : Intel(R) Celeron(R) CPU  J1800  @ 2.41GHz
			cpu MHz         : 2408.000
			cpu cores       : 2
			bogomips        : 4833.43
			*/
			
			$this->info["bogomips"]=25*round($this->grepinfo("BogoMIPS",$this->info["bogomips"])/25);
			$this->info["cpu_cores"]=$this->grepinfo("cpu cores",$this->info["cpu_cores"]);
			$defmhz=$this->info["bogomips"]/$this->info["cpu_cores"];
			$this->info["cpu_mhz"]=$this->grepinfo("cpu MHz",$defmhz);
			$this->info["cpu_ghz"]=round($this->info["cpu_mhz"]/1000,1);
			$this->info["cpu_hardware"]=$this->grepinfo("Hardware","Unknown");
			$this->info["cpu_type"]=$this->grepinfo("model name");
			if(!$this->info["cpu_type"]){
				$this->info["cpu_type"]=$this->grepinfo("Processor","Unknown");
			}
			$nbproc=cmdline("ps -A --no-headers | wc -l");
			trace($nbproc);
			if($nbproc){
				$this->info["cpu_procs"]=(int)trim($nbproc[0]);		
			}
			break;;
		case $this->os=="WINNT":
			// Windows machines
			trace("OSSensor::cpuinfo : Windows method: wmic");
			// COMPUTERSYSTEM get Manufacturer = Dell Inc.
			// COMPUTERSYSTEM get Model        = OptiPlex 9010
			// cpu get MaxClockSpeed = 3401
			// cpu get Description = Intel64 Family 6 Model 58 Stepping 9
			// cpu get Name        = Intel(R) Core(TM) i7-3770 CPU @ 3.40GHz
			$this->info["cpu_cores"]=$this->wmic("cpu get NumberOfCores","int");
			$this->info["cpu_mhz"]=$this->wmic("cpu get MaxClockSpeed","int");
			$this->info["cpu_ghz"]=round($this->wmic("cpu get MaxClockSpeed","int")/1000,1);
			$this->info["bogomips"]=$this->info["cpu_cores"] * $this->info["cpu_mhz"];
			$this->info["cpu_hardware"]=$this->wmic("cpu get Description");
			$this->info["cpu_type"]=$this->wmic("cpu get Name");
			$procs=cmdline("tasklist");
			if($procs){
				$this->info["cpu_procs"]=count($procs);		
			}
			break;;
		default:
			trace("OSSensor::cpuinfo : MacOSX method: sysctl");
			$this->info["cpu_cores"]=$this->sysctl("hw.ncpu");
			$this->info["cpu_mhz"]=round($this->sysctl("hw.cpufrequency")/1000000,1);
			$this->info["cpu_ghz"]=round($this->sysctl("hw.cpufrequency")/1000000000,1);
			$this->info["bogomips"]=$this->info["cpu_cores"] * $this->info["cpu_ghz"];
			$nbproc=cmdline("ps -A --no-headers | wc -l");
			if($nbproc){
				$this->info["cpu_procs"]=$nbproc[0];		
			}
		}
		trace($this->info);
		return $this->info;
	}

	function memload(){
		$this->info["mem_used"]=0;
		$this->info["mem_total"]=0;
		$this->info["mem_free"]=0;
		$this->info["mem_php"]=memory_get_usage();
		switch($this->os){
		case "WINNT":
			$this->info["mem_total"]=$this->wmic("ComputerSystem get TotalPhysicalMemory","int");
			$this->info["mem_free"]=$this->wmic("OS get FreePhysicalMemory","int");
			$this->info["mem_used"]=$this->info["mem_total"]-$this->info["mem_free"];
			break;;
		case "Linux":
		case "Darwin":
			$result=cmdline("free -b | grep Mem");
			if($result[0]){
				$line=trim($result[0]);
				$line=preg_replace("#\s\s*#","\t",$line);
				list($mem,$total,$used,$free,$rest)=explode("\t",$line,5);
				$this->info["mem_used"]=$used;
				$this->info["mem_total"]=$total;
				$this->info["mem_free"]=$free;
			}
			break;;
		default:
		}
		return $this->info;
	}
	
	function diskfree($path){
		$this->info["disk_total"]=disk_total_space($path);
		$this->info["disk_free"]=disk_free_space($path);
		$this->info["disk_used"]=$this->info["disk_total"]-$this->info["disk_free"];
		$this->info["disk_name"]="Unknown";
		switch($this->os){
		case "WINNT":
			$output=cmdline("dir \"$path\"");
			if($output){
				// Volume in drive D is DATAPART1
				$line1=trim($output[0]);
				$drive=preg_find("/drive (\w)/",$line1);
				$dname=preg_find("/([^\s\t]*)$/",$line1);
				$this->info["disk_name"]="$drive: [$dname]";
				$this->info["disk_drive"]="$drive";
			}
			break;;
		case "Linux":
		case "Darwin":
			$output=cmdline("df \"$path\"");
			if($output){
				$line2=trim($output[1]);
				$drive=preg_find("/^([^\s\t]*)/",$line2); // first word
				$drive=basename($drive);
				$dname=preg_find("/([^\s\t]*)$/",$line2); //  last word
				$this->info["disk_name"]="$drive: [$dname]";
				$this->info["disk_drive"]="$drive";
			}
			break;;
		default:
		}
		return $this->info;
	}
	
	function uptime(){
		$uptime = 0;

		switch($this->os){
		case "WINNT":
			// Windows machines
			// LastBootUpTime
			// 20150122133550.626398+060
			trace("OSSensor::uptime : Windows method: wmic");
			$lastboot=$this->wmic("os get lastbootuptime");
			$bdate=midstr($lastboot,0,4)."-".midstr($lastboot,4,2)."-".midstr($lastboot,6,2)." ".midstr($lastboot,8,2).":".midstr($lastboot,10,2);
			$btime=strtotime($bdate);
			$since=time()-$btime;
			$sincedays=$since/(3600*24);
			switch(true){
			case $sincedays < 1:	$uptime=round($since/3600,1) . " hours";	break;;
			case $sincedays < 60:	$uptime=round($since/3600*24,1) . " days";	break;;
			case $sincedays < 365:	$uptime=round($since/3600*24*7,1) . " weeks";	break;;
			default:				$uptime=round($since/3600*24*365,1) . " years";
			}
		case "Darwin":
		case "Linux":
			// Linux machines
			trace("OSSensor::uptime : Linux method: /usr/bin/uptime");
			$output = cmdline('/usr/bin/uptime -p');
			if($output){
				$uptime=trim($output[0]);
				$this->info["uptime"]=$uptime;
			}
			$output = cmdline('/usr/bin/uptime -s');
			if($output){
				$this->info["lastboot"]=trim($output[0]);
			}
			break;;
		default:
			trace("OSSensor::uptime : unknown OS");
		}
		trace("OSSensor::uptime : return $uptime");
		return $uptime;
	}

	function battery(){
		$result=Array();
		switch($this->os){
		case "WINNT":
			// Windows machines
			trace("OSSensor::battery : Windows method: wmic");
			// to be implemented
			$lastboot=$this->wmic("os get lastbootuptime");
			$result["error"]="not yet implemented";
		case "Darwin":
			if (file_exists('/usr/sbin/system_profiler')){
				trace("OSSensor::battery : MacOSX method: system_profiler");
				$output=cmdline("system_profiler SPPowerDataType");
				$this->parse_profiler($output);
			} elseif (file_exists('/usr/sbin/AppleSystemProfiler')) {
				trace("OSSensor::battery : MacOSX method: AppleSystemProfiler");
				$output=cmdline("AppleSystemProfiler SPPowerDataType");	
			}
			$parsed=$this->parse_profiler($output);
			// static
			$result["battery_capacity"]	=$this->find_val("Full Charge Capacity",$parsed);
			$result["charger_watt"]		=$this->find_val("AC Charger Information - Wattage",$parsed);

			// boolean
			$result["battery_present"]	=$this->find_bool("Battery Information - Battery Installed",$parsed);
			$result["charger_busy"]		=$this->find_bool("AC Charger Information - Charging",$parsed);
			$result["charger_present"]	=$this->find_bool("AC Charger Information - Connected",$parsed);
			$result["charger_done"]		=$this->find_bool("Fully Charged",$parsed);

			// dynamic
			$result["battery_health"]	=$this->find_val("Health Information - Condition",$parsed);
			$result["battery_cycles"]	=$this->find_val("Cycle Count",$parsed);

			$result["battery_mamp"]		=$this->find_val("Battery Information - Amperage",$parsed);
			$result["battery_mvolt"]	=$this->find_val("Battery Information - Voltage",$parsed);
			$result["battery_charge"]	=$this->find_val("Charge Remaining",$parsed);
			$result["battery_charge_%"]=round(100*$result["battery_charge"]/$result["battery_capacity"],3);
			ksort($result);

		case "Linux":
			// Linux machines
			$result["error"]="not yet implemented";
			break;;
		default:
			$result["error"]="not yet implemented";
			trace("OSSensor::uptime : unknown OS");
		}
		return $result;
	}

	private function wmic($command,$type="string"){
		if($this->os<>"WINNT")	return false;
		$val=false;
		$process = @popen('wmic $command', 'rb');
		if ($process){
			fgets($process); // skip^first line with field title
			switch($type){
			case "int":
			case "integer":
				$val = intval(fgets($process));
				break;;
			default:
				$val = trim(fgets($process));
			}
			pclose($process);
		}
		return $val;
	}

	private function grepinfo($param,$default=false,$file="/proc/cpuinfo"){
		if($this->os<>"Linux")	return false;
		$output=cmdline("grep \"$param\" $file");
		$val=$default;
		if($output){
			foreach($output as $line){
				list($key,$val)=explode(":",$line,2);
				if($val)	$val = trim($val);
			}
		}
		trace("grepinfo: found [$param] = [$val]");
		return $val;
	}

	private function sysctl($key,$type="string"){
		if($this->os<>"Darwin")	return false;
		$val=false;
		$process = @popen('sysctl -a', 'rb');
		if ($process){
			$output = stream_get_contents($process);
			switch($type){
			case "int":
			case "integer":
				preg_match('/$key: (\d+)/', $output, $matches);
				break;;
			default:
				preg_match('/$key: (.+)$/', $output, $matches);
			}
			if ($matches){
				$val=$matches[1][0];
			}
			pclose($process);
		}
		return $val;
	}

	private function parse_profiler($lines){
		$previndent=-1;
		$parsed=Array();
		foreach($lines as $line){
			if(strlen(trim($line)) == 0) continue;
			$indent=strlen($line)-strlen(preg_replace("/^([\s\t]*)/","",$line));
			list($key,$val)=explode(":",trim($line),2);
			$val=trim($val);
			switch(true){
			case strlen($val) > 0:
				$tree=Array();
				foreach($titles as $level => $title){
					if($level < $indent AND strlen($title) > 0) $tree[]=$title;
				}
				$tree[]=$key;
				//$tree[]=$indent;
				$combined=implode(" - ",$tree);
				//$parsed[$combined]=$val;
				$parsed[]="$combined: $val";
				//trace("$combined :: $val");
			case $indent < $previndent:
				//$titles[$previndent]="";
				$titles[$indent]=$key;
				$previndent=$indent;
				break;
			case $indent > $previndent:
				$titles[$indent]=$key;
				break;
			}

		}
		return $parsed;
	}
	
	private function find_val($pattern,$subject){
		$results=preg_grep("/$pattern/", $subject);
		if(!$results){
			trace("find_val: cannot find [$pattern]");
			return false;
		}
		foreach($results as $result){
			trace($result);
			list($key,$val)=explode(":",$result,2);
		}
		$val=trim($val);
		return $val;
	}

	private function find_bool($pattern,$subject){
		$results=preg_grep("/$pattern/", $subject);
		if(!$results)	return false;
		foreach($results as $result){
			list($key,$val)=explode(":",$result,2);
		}
		$val=strtoupper(trim($val));
		switch($val){
			case 1:
			case "TRUE":
			case "YES":
			case "OUI":
				trace("find_bool:  found $val => 1");
				return 1;
				break;;
			case 0:
			case "NON":
			case "NO":
			case "FALSE":
				trace("find_bool:  found $val => 0");
				return 0;
				break;;
		}
		trace("find_bool:  found $val => ?");
		if(strtoupper(substr($val,0,1)) == "N")	return 0;
		return 1;
	}

}

?>
