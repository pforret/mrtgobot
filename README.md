# MRTGOBOT

Robot script for a full MRTG installation: takes care of index, updates and rsync

## mrtgobot setup

* helps set up a minimal MRTG .cfg config file
* will ask for target folder, html output folder, rsync settings, ...
* will set up a list of localhost metrics for MRTG (cpu/mem/disk/netw)

## mrtgobot run [target]

* run MRTG for target (typically every 5 minutes)
* target can be 1 `.cfg` file, or a folder of `.cfg` files

## mrtgobot index [target]

* create HTML index files for target 
* target can be 1 `.cfg` file, or a folder of `.cfg` files

## mrtgobot rsync [target]

* rsync the HTML output of the target to a different server
* typical setup: a LAN server 
* target can be 1 `.cfg` file, or a folder of `.cfg` files

## mrtgobot crontab [target]

gives back a good suggestion for insertion into your cron jobs

* every 5 minutes for `mrtgobot run`
* 1/hour for `mrtgobot rsync`
* 1/day for `mrtgobot index`

## mrtgobot probe [metric]

* allows for measuring server metrics and returnign result in MRTG compatible format
* metrics: cpu,mem,disk,network
* MRTG format =
    
        [I value]
    	[O value]
    	[uptime]
    	[description]
    	[...] (is not used by MRTG, but can contain useful extra information for testing)
		[...]
