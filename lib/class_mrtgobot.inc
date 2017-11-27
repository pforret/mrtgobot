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
		$lines[]="## Title: $title";
		$lines[]="WorkDir: $workdir";
		$lines[]="EnableIPv6: no";
		$lines[]="";
		$lines[]="PageTop[$]: <div class='navbar'><a href='./'>Overview</a></div>";
		$ImgURL=$this->get_setting("PageIconURL");
		$lines[]="PageTop[^]: <img src='$ImgURL' style='float: right; height: 120px' title='MRTGoBot'/>";
		$lines[]="AddHead[_]: <link rel='stylesheet' href='../mrtgobot.css' />";
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
		/*
		### Global Config Options

		WorkDir: /volume1/web/mrtg/bfnp_stor02
		EnableIPv6: no
		PageTop[^]: <img src="https://core.api.myqnapcloud.com/device_model/52d4ff96f01af85df64bc163" style="float: right; height: 120px" />
		PageTop[$]: <div class='navbar'><a href='./'>Overview</a></div>
		Options[_]: growright,nobanner
		AddHead[_]: <link rel='stylesheet' href='http://whale.cinemapub.be/css/mrtg.css' /><link rel='stylesheet' href='mrtg.css' />
		WithPeak[_]:ym
		ShortLegend[_]: B/s
		YLegend[_]: B/s
		Legend1[_]: Download
		Legend2[_]: Upload
		Legend3[_]: Peak Download
		Legend4[_]: Peak Upload
		LegendI[_]: Download
		LegendO[_]: Upload
		PNGTitle[_]: BFNP-STOR02
		#Title: BFNP_STOR02 QNAP BACKUP


		Target[bruxsstor02.cpu]: `curl -s "http://10.11.114.15/mrtg_remote_sensor/index.php?key=cpu"`
		Title[bruxsstor02.cpu]: bfnp-stor02: CPU load (5/15 min)
		PageTop[bruxsstor02.cpu]: <h1>bfnp-stor02: CPU load (5/15 min)</h1>
		LegendI[bruxsstor02.cpu]: Avg load over 5 min
		LegendO[bruxsstor02.cpu]: Avg load over 15 min
		YLegend[bruxsstor02.cpu]: load
		PNGTitle[bruxsstor02.cpu]: bruxsstor02.cpu
		ShortLegend[bruxsstor02.cpu]: load
		Options[bruxsstor02.cpu]: growright,nobanner,gauge
		MaxBytes[bruxsstor02.cpu]: 1080
		kMG[bruxsstor02.cpu]: ,k,M,G,T,P
		*/
	}
	
	function indexmaker($profile){
		$this->load_ini();
		if(is_dir ($profile)){
			$cfgs=listfiles($profile,".cfg");
		} else {
			$cfgs=Array(realpath($profile));
		}
		foreach($cfgs as $cfgfile){
			$cfgname=basename($cfgfile,".cfg");
			$cfg=$this->load_cfg($cfgfile);
			$idxprog=$this->get_setting("prog_index");
			$folder=$cfg["common"]["WorkDir"];
			$parent=dirname($folder);
			$idxfile="$folder/index.html";
			$idx2file="$folder/$cfgname.html";
			$root[$parent][]=$idx2file;
			$hostname=gethostname();
			$hostip=$this->getexternalip();
			$hostdesc=php_uname();
			$params=Array();
			$params[]="\"$idxprog\"";
			$params[]="--nolegend";
			$params[]="--show=day";
			$params[]="--width=350"; // images on 50%
			$params[]="--columns=3";	// all screens can show 1000px
			if(isset($cfg["blocks"]["_"]["AddHead"])){
				$params[]="--addhead=\"" . $cfg["blocks"]["_"]["AddHead"] . "\"";
			}
			if(isset($cfg["blocks"]["^"]["PageTop"])){
				$params[]="--pagetop=\"" . $cfg["blocks"]["^"]["PageTop"] . "\"";
			}
			$time=date("c");
			$params[]="--pageend=\"<footer>Generated by <a href='https://github.com/pforret/mrtgobot'>MRTGoBot</a><code><br />$time<br />$hostdesc</code></footer>\"";
			if(isset($cfg["blocks"]["_"]["Title"])){
				$params[]="--title=\"" . $cfg["blocks"]["_"]["Title"] . "\"";
			} else {
				$params[]="--title=MRTGoBot";
			}
			$params[]="--subtitle=\"<div class='subtitle'>@ $hostname - $hostip</div>\"";
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
		foreach($root as $parent => $kids){
			$pname=basename($parent);
			$pfile="$parent/index.html";
			$html="<html>\n";
			$html.="<head><title>$pname overview - MRTGoBot</title></head>\n";
			$html.="<link rel='stylesheet' href='mrtgobot.css' />\n";
			$html.="<body><div style='text-align: center; background: #EEE'><strong>MRTGoBot</strong>: \n";
			foreach($kids as $indexfile){
				$dname=basename(dirname($indexfile));
				$iname=basename($indexfile);
				$html.="[<a target='theframe' href='$dname/$iname'>$dname</a>] \n";
			}
			$html.="</div>\n";
			$html.="<iframe name='theframe' style='width:100%; height:99%; border:0px' src='$dname/$iname'></iframe>\n";
			$html.="</body>\n";
			$html.="</html>\n";
			progress("INDEX","Create parent index for [$pname]");
			file_put_contents($pfile,$html);
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