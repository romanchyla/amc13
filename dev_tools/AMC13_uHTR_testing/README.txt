README file for preliminary LocalTrigBuild.py script
Contact: David Zou, BU Graduate Student, dzou@bu.edu
Date Created: Sept. 8, 2014
Last Updated: Sept. 8, 2014

Change Log:
2016-04-25, dzou - changed uHTR scripts to exit with -1 (uHTRtool changed)

2014-09-11, hazen - add some explanation of what actually is done!

[1] Description:

LocalTrigBuild.py is a testing script used to initialize an AMC13 and
uHTR(s). The initial build defaults to setting up the AMC13 to use
local triggers to send trigger to uHTR via the backplane DAQ and build
events.

first:  wv 3 0		Disable links
	ws 0xd 0	Disable TTC output force

	(AMC13 may or may not be in run mode at this point!)

second:	wv 0x1a 0xfff   Disable TTS from AMCs
	wv 0xd 0xfff	Force TTC output on
	wv 0x18 0xfff	Set payload size of fake events to 0xfff (WHY ??)
	i 7 t		Enable link 7 only
	localL1a b 1 1	Enable local L1A

third:	run uHTR setup

fourth:	send ECR and OCR

fifth: send trigger and dump prescaled events in batches

[2] Required Working Tools:
    AMC13Tool2.exe
    uHTRtool.exe

[3] Required Scripts/Files (Included):

	 crateTest2Local.py
	 systemVars.py

	 amcEvnOrnReset.txt
	 amcLinkDisable.txt
	 amcReload.txt
	 amcStatus.txt
	 amcTrigStop.txt
	 amcTrigStart.txt
	 checkDAQ.uhtr
	 clockLumi.sh
	 daqcheck.sh
	 disableDAQPath.uhtr
	 enabledDAQPath.uhtr

[4] Usage:

1. Make sure all required tools [2] are working, and that the hardware is set up correctly.
2. Save all required scripts [3] to a single directory (should be unpacked with this README). 
3. Open systemVars.py with editor (see comments in systemVars.py)
   3a. Change the paths to corresponding paths for your system
   3b. Change the filename for AMC13 to corresponding connection file for AMC13 to be used
   3c. Change the uHTR IP address list to a comma seperated list of the IP addresses for the uHTR that you will be initializing)
   3c. Change the uhtr slot  to a list of integers (1-based) corresponding to the AMC slots of the uHTRs that you will be using for the test
   3d. Change initial bits of uHTR IP addresses (if necessary)
4. Run script from directory with LocalTrigBuild.py. (To use options, see [5]) To run with defaults: 
   > ./crateTest2Local.py
   Running default is the same as using following options:
   > ./crateTest2Local.py -b 100 -e 200 -p 13 -t "b 1 150"

[5] Options:
   5a. -b <int>
       Set batch size to <int> (script waits until at least <int> triggers sent before pasuing triggers and dumping events and then continue receiving events)
   5b. -e <int>
       Total number of events dumped before ending program set to <int>
   5c. -p <int>
       Prescale # of 0s set to <int> (monitor buffer only saves events with <int> lowest bits equal to 0 (min 5, max 20). 0 sets mega monitoring off)
   5d. -t <string>
       trigger setting made with AMC13Tool2 command 'localL1A <string>'
   5e. -l 
       Use IPs in UHTR_LIST in systemVars.py instead of slot number defaults
   5f. -r
       Reload amc13 at start of program (not recommended). Should only be used by experts 

*** Note that the actual total events per batch (-b) or per run of program (-e) will likely be larger than the specificed amount since events may be collected and built during the time it takes to stop triggers


[6] Output files (saved to Run_<timestamp>/results directory):
       dump*.dat
       amcInitStat.txt
       amcEndStat*.txt
       (* indicates a number corresponding to the batch_no)
       htrDaqStatInit**.txt
       htrDaqStatB4Trig**.txt
       (** indicates the number correspoding to the IP address of the corresponding uHTR)

 Additional Warnings/Notes:

A number of temporary files are created during the test and stored in
the tmp directory included in the build, do not be alarmed to see
these appear


[7] Temp Scripts/Files created by CrateTest.py (saved to Run_<timestamp>/tmp directory):
     daqcheck.txt
     dumpFile.txt
     evtStatus.txt
     prepAMC.txt	 

