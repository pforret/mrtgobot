<?php
include_once("tools.inc");

Class Mrtgobot{
	function __construct($inifolder="config",$inifile="mrtgobot.ini"){
		$this->inifolder=$inifolder;
		$this->inifile="$inifolder/$inifile";
		$this->loaded=false;
		
		$this->filecopy=Array();
		// copy to folder WorkDirRoot/ upon creation
		$this->filecopy["html_root"]=Array(
			"config/favicon.ico",
			"config/mrtg.css",
			"config/mrtgobot.png",
			);
		// copy to folder WorkDirRoot/cfgname/ upon creation
		$this->filecopy["html_work"]=Array(
			"config/favicon.ico",
			);
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
		$vars["PageIconURL"]="../logo.png";
		$vars["PageCssURL"]="../mrtg.css";
		$vars["Interval"]=5; // default is 5 minutes
		$vars["LegendI"]="I";
		$vars["LegendO"]="O";
		$vars["Legend1"]="I";
		$vars["Legend2"]="O";
		$vars["Legend3"]="PeakI";
		$vars["Legend4"]="PeakO";
		$vars["Colours"]="Orange#FF8C00,Blue#0079B2,Orange2#FFB75F,Blue2#70D1FF";
		$vars["EnableIPv6"]="no";
		progress($inifile,"configuration saved to [$this->inifile]");
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
				trace(__METHOD__."- copy file $file to $htmlroot");
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
		$lines[]="PageTop[^]: <img src='$ImgURL' style='float: right; height: 120px' />";
		$lines[]="LegendI[_]: " . $this->get_setting("LegendI","I");
		$lines[]="LegendO[_]: " . $this->get_setting("LegendO","O");
		$lines[]="Legend1[_]: " . $this->get_setting("Legend1","I");
		$lines[]="Legend2[_]: " . $this->get_setting("Legend2","O");
		$lines[]="Legend3[_]: " . $this->get_setting("Legend3","PeakI");
		$lines[]="Legend4[_]: " . $this->get_setting("Legend4","PeakO");
		$lines[]="WithPeak[_]:" . $this->get_setting("WithPeak","ym");
		$lines[]="YLegend[_]: " . $this->get_setting("YLegend","/s");
		$lines[]="ShortLegend[_]: " . $this->get_setting("ShortLegend","/s");
		$lines[]="Options[_]: " . $this->get_setting("Options","growright,nobanner,nolegend");
		$lines[]="";
		$lines[]="## --- Per graph options";
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
	
	function load_cfg($name){
	}
}

?>