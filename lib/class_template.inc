<?php
include_once("tools.inc");

Class Template {
	var $folder;
	var $active;
	
	function __construct($folder){
		if(file_exists($folder)){
			$this->folder=$folder;
			$this->active=true;
			trace("Template:: active in folder [$folder]");
		} else {
			$this->active=false;
		}
	}
	
	function render($input,$keys=false){
		if(!$this->active)	return false;
		$ifile="$this->folder/$input";
		if(!file_exists($ifile)){
			return false;
		}
		$html=file_get_contents($ifile);
		trace("Template::render [$input]: " . strlen($html) ." chars");
		$html=$this->replace_keys($html,$keys);
		$html=$this->cleanup($html);
		trace("Template::render now " . strlen($html) ." chars");
		return $html;
	}
	
	function replace_keys($html,$keys){
		if(!$this->active)	return false;
		foreach($keys as $key => $val){
			if(is_array($val)){
				// look for [$key[ ... ]$key]
				$blocktpl=preg_find("/(\[$key\].*\[\/$key\])/s",$html);
				if($blocktpl){
					$blockhtml="";
					foreach($val as $item){
						$blockhtml.=$this->replace_keys($blocktpl,$item);
					}
					$html=str_replace($blocktpl,$blockhtml,$html);
				}
			} else {
				$html=str_replace("[$key]",$val,$html);
				//trace("Template::replace_keys [$key] => [$val]");
			}
		}
		return $html;
	}
	
	function cleanup($html){
		if(!$this->active)	return false;
		$html=preg_replace("|(\[\w+\])|","",$html);
		$html=preg_replace("|(\[\/\w+\])|","",$html);
		return $html;
	}
}