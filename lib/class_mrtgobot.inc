<?php
include_once("tools.inc");

Class Mrtgobot{
	function __construct($inifolder="config",$inifile="mrtgobot.ini"){
		$this->inifolder=$inifolder;
		$this->inifile="$inifolder/$inifile";
		$this->loaded=false;
		
		$this->filecopy=Array();
		// copy to folder WorkDirRoot/ upon creation
		$this->filecopy["html_root"]=listfiles("config/global");
		// copy to folder WorkDirRoot/cfgname/ upon creation
		$this->filecopy["html_work"]=listfiles("config/percfg");
		$this->hostname=gethostname();
		$this->hostip=$this->getexternalip();
		trace("HOSTNAME = [$this->hostname] - IP = [$this->hostip]");
	}
	
	function init_ini($cfgfolder,$htmlroot){
		// first command to run
		$inifile=basename($this->inifile);
		if(!file_exists($this->inifolder)){
			mkdir($this->inifolder);
		}
		$vars["FolderCFG"]=$cfgfolder;
		progress($inifile,"folder for input .cfg files : [$cfgfolder]");
		if(!file_exists($cfgfolder)){
			//printf("* %10s: created folder\r\n",$cfgfolder);
			mkdir($cfgfolder,0777,true);
		}
		progress($inifile,"folder for output html files: [$htmlroot]");
		if(!file_exists($htmlroot)){
			//printf("* %10s: created folder\r\n",$htmlroot);
			mkdir($htmlroot,0777,true);
		}
		$vars[";; comment"]="These are the default values for creation of .cfg files";
		$vars["WorkDirRoot"]=realpath($htmlroot);
		$vars["PNGTitle"]="MRTGoBot";
		$vars["PageIconURL"]="../mrtgobot.png";
		$vars["PageCssURL"] ="../mrtgobot.css";
		$vars["Interval"]=5; // default is 5 minutes
		$vars["LegendI"]="I";
		$vars["LegendO"]="O";
		$vars["Legend1"]="I";
		$vars["Legend2"]="O";
		$vars["Legend3"]="PeakI";
		$vars["Legend4"]="PeakO";
		$vars["Colours"]="Orange#FF8C00,Blue#0079B2,Orange2#FFB75F,Blue2#70D1FF";
		$vars["EnableIPv6"]="no";
		$vars["MaxBytes"]=1000000000;
		$vars["Options"]="growright,nobanner,nolegend,nopercent";
		
		$vars["prog_mrtg"]=$this->check_prog("mrtg");
		$vars["prog_index"]=$this->check_prog("indexmaker");
		$vars["prog_cfg"]=$this->check_prog("cfgmaker");
		
		progress($inifile,"configuration saved to [$this->inifile]");
		trace($vars);
		$this->save_ini_file($this->inifile,$vars);
		$this->copy_if_needed($this->filecopy["html_root"],$htmlroot);
	}
	
	function load_ini(){
		if(!file_exists($this->inifolder)){
			warning(__METHOD__ ,"no ini folder [$this->inifolder] found");
			return false;
		}
		if(!file_exists($this->inifile)){
			warning(__METHOD__,"no ini file [$this->inifile] found ");
			return false;
		}
		trace(__METHOD__ . "- reading from $this->inifile");
		$this->settings=parse_ini_file($this->inifile);
		if($this->settings){
			$this->loaded=true;
		}
	}
	
	function copy_if_needed($files,$destination){
		foreach($files as $file){
			$new=$destination . "/" . basename($file);
			if(file_exists($file) AND !file_exists($new)){
				trace(__METHOD__."- copy file $file to $destination");
				copy($file,$new);
			}
		}
	}
	function get_setting($name,$default=false){
		if(!isset($this->settings[$name])){
			return $default;
		} else {
			return $this->settings[$name];
		}
	}
	
	function save_ini_file($fname,$vars){
		$lines[]=";; INI FILE [$fname]";
		$lines[]=";; generated on " . date("c");
		ksort($vars);
		foreach($vars as $key => $val){
			$lines[]=sprintf('%-12s "%s"',"$key=",$val);
		}
		file_put_contents($fname,implode("\r\n",$lines)."\r\n");
	}
	
	function new_cfg($name,$title=false){
		$prog=$_SERVER["SCRIPT_FILENAME"];
		$this->load_ini();
		$cname=basename($name,".cfg");
		if(!$title){
			$title=trim(preg_replace("/[^\d\w]/"," ",$cname));
		}
		$cname=preg_replace("/[^\d\w]/","",$cname); // get rid of funny characters é@/$
		if(!$this->loaded){
			warning(__METHOD__,"ini not loaded yet ");
			return false;
		}
		$dcfg=$this->get_setting("FolderCFG");
		if(!$dcfg){
			warning(__METHOD__,"need CFG folder in INI file ");
			return false;
		}
		$fcfg="$dcfg/$cname.cfg";
		$dhtml=$this->get_setting("WorkDirRoot");
		$workdir="$dhtml/$cname";
		trace(__METHOD__ . "- WorkDir will be [$workdir]");
		if(!file_exists($workdir)){
			mkdir($workdir);
		}
		$this->copy_if_needed($this->filecopy["html_work"],$workdir);
		$lines=Array();
		$lines[]="###### $cname .cfg file - generated on " . date("c");
		$lines[]="## --- Global options";
		$lines[]="## Title: $title @ $this->hostname";
		$lines[]="WorkDir: $workdir";
		$lines[]="EnableIPv6: no";
		$lines[]="";
		$lines[]="PageTop[$]: <div class='navbar'><a href='./'>Overview</a></div>";
		$ImgURL=$this->get_setting("PageIconURL");
		//$lines[]="PageTop[^]: <img src='$ImgURL' style='float: right; height: 120px' title='MRTGoBot'/>";
		$lines[]="AddHead[_]: " . $this->get_setting("AddHead","<link rel='stylesheet' href='../mrtgobot.css' />");
		$lines[]="LegendI[_]: " . $this->get_setting("LegendI","I");
		$lines[]="LegendO[_]: " . $this->get_setting("LegendO","O");
		$lines[]="Legend1[_]: " . $this->get_setting("Legend1","I");
		$lines[]="Legend2[_]: " . $this->get_setting("Legend2","O");
		$lines[]="Legend3[_]: " . $this->get_setting("Legend3","PeakI");
		$lines[]="Legend4[_]: " . $this->get_setting("Legend4","PeakO");
		$lines[]="WithPeak[_]:" . $this->get_setting("WithPeak","ym");
		$lines[]="YLegend[_]: " . $this->get_setting("YLegend","/s");
		$lines[]="ShortLegend[_]: " . $this->get_setting("ShortLegend","/s");
		$lines[]="Options[_]: " . $this->get_setting("Options","growright,nobanner,nolegend,nopercent");
		$lines[]="MaxBytes[_]: " . $this->get_setting("MaxBytes","1000000000");
		$lines[]="Title[_]: $title - MRTGoBot";
		$lines[]="XSize[_]: 600"; // 600px is max width you can take
		$lines[]="YSize[_]: 165"; // 165 + 35 extra = 200px total height
		$lines[]="";
		$lines[]="## --- Per graph options";
		$lines[]="# Target[example]: `$prog PROBE CPU`";
		$lines[]="# Title[example]: Title";
		$lines[]="# PageTop[example]: <h1>Title</h1>";
		$lines[]="";
		progress($cname,"save to [$fcfg]");
		file_put_contents($fcfg,implode("\r\n",$lines));
	}

	function run_rsync($profile,$path){
		// rsync -az /var/www/html/mrtgobot/ cinemapub@dcp.cinemapub.be:apps/monitor/rpmonitor/R-KBXL-601/
		$this->load_ini();
		$WorkDirRoot=$this->get_setting("WorkDirRoot");
		cmdline("/usr/bin/rsync -az $WorkDirRoot/* $path",false,0);
	}
	
	function indexmaker($profile){
		$this->load_ini();
		if(is_dir ($profile)){
			$cfgs=listfiles($profile,".cfg");
		} else {
			$cfgs=Array(realpath($profile));
		}
		
		// create the individual index files
		foreach($cfgs as $cfgfile){
			$cfgname=basename($cfgfile,".cfg");
			$cfg=$this->load_cfg($cfgfile);
			$idxprog=$this->get_setting("prog_index");
			$folder=$cfg["common"]["WorkDir"];
			$parent=dirname($folder);
			$idxfile="$folder/index.html";
			$idx2file="$folder/$cfgname.html";
			$root[$parent][]=$idx2file;
			$hostdesc=php_uname();
			$params=Array();
			$params[]="\"$idxprog\"";
			$params[]="--nolegend";
			$params[]="--show=day";
			$params[]="--width=400"; // images on 50%
			$params[]="--columns=3";	// all screens can show 1200px
			if(isset($cfg["blocks"]["_"]["AddHead"])){
				$params[]="--addhead=\"" . $cfg["blocks"]["_"]["AddHead"] . "\"";
			}
			if(isset($cfg["blocks"]["^"]["PageTop"])){
				$params[]="--pagetop=\"" . $cfg["blocks"]["^"]["PageTop"] . "\"";
			}
			$time=date("c");
			$params[]="--pageend=\"<footer><i class='fab fa-github'></i> Generated by <a href='https://github.com/pforret/mrtgobot'>MRTGoBot</a><br /><i class='fas fa-clock'></i> Index created at <code>$time</code><br /><i class='fas fa-server'></i> Server: <code>$hostdesc</code></footer>\"";
			if(isset($cfg["blocks"]["_"]["Title"])){
				$params[]="--title=\"" . $cfg["blocks"]["_"]["Title"] . "\"";
			} else {
				$params[]="--title=$cfgname";
			}
			if(isset($cfg["blocks"]["_"]["Subtitle"])){
				$params[]="--subtitle=\"" . $cfg["blocks"]["_"]["Subtitle"] . "\"";
			} else {
				//$params[]="--subtitle=\"<div class='subtitle'><i class='fas fa-location-arrow'></i> $this->hostname - $this->hostip</div>\"";
			}
			$params[]="\"$cfgfile\"";
			$idxline=implode("  ",$params);
			$idxline.="> $idxfile";
			progress("INDEX","Create index for [$cfgname]");
			cmdline($idxline,false,0);
			$isize=filesize($idxfile);
			if($isize > 0){
				trace("indexmaker: index $cfgname is " . round($isize/1000) . " KB");
				copy($idxfile,$idx2file);
			} else {
				progress("INDEX","WARNING: index of [$cfgname] is empty");
			}
		}
		
		// create the overview/parent index file
		foreach($root as $parent => $kids){
			$pname=basename($parent);
			$pfile="$parent/index.html";
			$pover="$parent/overview.html";
			$overview=false;
			$navbar="<html>\n";
			$navbar.="<head><title>$pname @ $this->hostname //  Overview</title></head>\n";
			$navbar.="<link rel='stylesheet' href='mrtgobot.css' />\n";
			$navbar.="<link HREF='favicon.ico' rel='shortcut icon' >";
			$navbar.="<body><div style='text-align: center; background: #EEE'><strong>MRTGoBot @ $this->hostname</strong>: \n";
			$lastfound="";
			foreach($kids as $indexfile){
				$dname=basename(dirname($indexfile));
				$iname=basename($indexfile);
				$bname=basename($indexfile,".html");
				if(file_exists($indexfile)){
					$html=file_get_contents($indexfile);
					$head1=stripos($html,"<HEAD>");
					$head2=stripos($html,"</HEAD>",$head1);
					if(!$overview AND $head2){
						$head=substr($html,$head1,($head2-$head1)+7);
						$overview="<html>\n$head\n<body>";
					}
					if($overview){
						$title=preg_find("|<title>([^<]*)</title>|i",$html);
						$overview.="<h2><a target='theframe' href='$dname/$iname'>$bname</a> ($title)</h2>";
						preg_match_all("|<DIV><B>([^<]*)</B></DIV>|i",$html,$matches);
						if($matches){
							$overview.="<small><code><nobr>" . implode("</nobr> &bull; <nobr>",$matches[1]) . "</nobr></code></small>\n";
						}
					}
					$navbar.="[<a target='theframe' href='$dname/$iname'>$bname</a>] \n";	
					$lastfound="$dname/$iname";
				} else {
					$navbar.="[<i>$dname</i>] \n";	
				}
			}
			if($overview){
				$overview.="</body></html>";
			}
			$navbar.="</div>\n";
			$navbar.="<iframe name='theframe' style='width:100%; height:99%; border:0px' src='overview.html'></iframe>\n";
			$navbar.="</body>\n";
			$navbar.="</html>\n";
			progress("INDEX","Create parent index for [$pname]");
			file_put_contents($pfile,$navbar);
			file_put_contents($pover,$overview);
			$isize=filesize($pfile);
			if($isize > 0){
				trace("indexmaker: parent index [$pname] is " . round($isize/1000,1) . " KB");
			} else {
				progress("INDEX","WARNING: parent index of [$pname] is empty");
			}
		}
	}
	
	function runcfg($profile,$bg=false){
		$t_start=microtime(true);
		if(is_dir ($profile)){
			$cfgs=listfiles($profile,".cfg");
		} else {
			$cfgs=Array(realpath($profile));
		}
		$tottest=0;
		$totcfg=0;
		foreach($cfgs as $cfgfile){
			$teststart=microtime(true);
			$totcfg++;
			$bname=basename($cfgfile,".cfg");
			$this->load_ini();
			$cfg=$this->load_cfg($cfgfile);
			$nbtest=$cfg["stats"]["tests"];
			if($nbtest<1)	$nbtest=1;
			//trace($cfg);
			$tottest+=$nbtest;
			$mrtgprog=$this->get_setting("prog_mrtg");
			$folder=$cfg["common"]["WorkDir"];
			$day=date("Ymd");
			$logfile="$folder/$bname.$day.log";
			$params=Array();
			$params[]="env LANG=C";
			$params[]="\"$mrtgprog\"";
			$params[]="\"$cfgfile\"";
			$params[]="2>> \"$logfile\"";
			$idxline=implode("  ",$params);
			progress("RUN","Run MRTG on [$bname]");
			cmdline($idxline,false,0);	
			$testend=microtime(true);
			$secs=round($testend-$teststart,1);
			$spt=round(($testend-$teststart)/$nbtest,1);
			trace("[$bname]: $nbtest tests in $secs secs - $spt secs/test");
		}
		$t_end=microtime(true);
		$timed=$t_end-$t_start;
		$spc=round($timed/$totcfg,1);
		$tps=round($tottest/$timed,1);
		progress("RUN","Finished: $spc sec/cfg, $tps test/sec");
	}
	
	function load_cfg($cfgfile){
		trace(__METHOD__." - reading from file [$cfgfile]");
		$config=Array();
		$config["blocks"]=Array();
		$config["common"]=Array();
		$fp=fopen($cfgfile,"r");
		$nblines=0;
		$tests=Array();
		while(!feof($fp)){
			$line=fgets($fp,2000);

			$line=trim(preg_replace("/#.*$/","",$line)); //trim comments
			if(strlen($line)<1)	continue;
			if(!contains($line,":"))	continue;
			$nblines++;
			list($key,$val)=explode(":",$line,2);
			$key=trim($key);
			$val=trim($val);
			if(contains($key,"[")){
				list($key2,$block)=explode("[",$key);
				$key2=trim($key2);
				$block=trim(str_replace("]","",$block));
				$config["blocks"][$block][$key2]=$val;
				if(!in_array($block,Array("$","^","_"))){
						$tests[$block]=$block;
				}
			} else {
				$config["common"][$key]=$val;
			}
		}
		$config["stats"]["lines"]=$nblines;
		$config["stats"]["tests"]=count($tests);
		//trace($config);
		
		return $config;
	}
	
	function check_prog($name,$default=false){
		if(!$default)	$default=$name;
		$lines=cmdline("which $name");
		if($lines){
			return $lines[0];
		} else {
			return $default;
		}
	}
	

	function getexternalip(){
		$url="http://ipinfo.io/ip";
		$lines=cmdline("curl -s $url");
		if(!$lines){
			$fp=fopen($url);
			if($fp){
				$ip=fgets($fp);
			}
		} else {
			$ip=$lines[0];
		}
		return $ip;
	}
}

?>
