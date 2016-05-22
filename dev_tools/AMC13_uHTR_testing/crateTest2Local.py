#! /usr/bin/env python

#Script to setup and run event build from uHTR using AMC13local trigger test using uHTRTool and AMC13Tool
#assume tools have been setup and made

from optparse import OptionParser
import systemVars as sv
import os as os
import time as time
import subprocess as subprocess
import shlex as shlex
import logging

######################
# Parse Options
######################

parser = OptionParser()
parser.add_option("-r", "--reloadamc13", 
                  action="store_true", dest="reloadamc13", default=False,
                  help="If set, complete reset of AMC13 (reload firmware) at start of test. ")
parser.add_option("-b", "--batchsize", dest="tot_evt_dump",
                  help="Total number of events in SDRAM before dumping (# of events per batch)",
                  default=100, metavar="<n>", type=int)
parser.add_option("-e","--evtmax", dest="max_evt",
                  help="Total number of events dumped before ending program",
                  default=200, metavar="<n>", type=int)
parser.add_option("-p", "--prescale", dest="prescale",
                  help="Set prescale with mega monitoring. Save only events with <n> lower bits equal to zero (min 5, max 20). 0 for mega monitoring off",
                  default=13, metavar="<n>", type=int)
parser.add_option("-t", "--trigger", dest="trig",
                  help="Indicate the localL1A ... settings",
                  default="b 1 150", metavar="<s>", type="string")
parser.add_option("-l", "--ipList",
                  action="store_true",dest="ipList", default=False,
                  help="Use IPs in UHTR_LIST and MCTR_LIST (see systemVars.py) ")
(options, args) = parser.parse_args()

######################                                                                                                                                                            
# Set up logger                                                                                                                                                                   
######################                                                                                                                                                            
logger = logging.getLogger('CrateTest')
hdlr = logging.FileHandler('/var/tmp/CrateTest.log')
formatter = logging.Formatter('[%(asctime)s] [%(levelname)8s] --- %(message)s (%(filename)s:%(lineno)s)')
hdlr.setFormatter(formatter)
logger.addHandler(hdlr)
logger.setLevel(logging.INFO)

##############################
# Set directories and tools
##############################
logger.info("Setting paths to relevant directories and tools...")

script_dir = sv.SCRIPT_DIR
#tmpDir = script_dir+"/tmp"
#resultsDir = tmpDir

amc_tool = sv.AMC_TOOL
htr_tool = sv.HTR_TOOL
addr_table = sv.ADDR_TABLE
conn_file = sv.CONN_FILE

##### uHTR IPs ########

# Initial bits of uHTR IP's
uhtr_ip_initial_bits = sv.UHTR_IP_BITS_INIT
    
# Setup AMC slot of uHTRs in crate (1 base i.e. number seen on crate)
uhtr_crate_num = sv.UHTR_SLOT

## Setup list of uhtr IPs
# Using initial bits + slot nums
uhtr_IPs = [] #sv.UHTR_LIST

if options.ipList:
    uhtr_IPs = sv.UHTR_LIST # Use user specified list of uHTR IP addresses
else:
    for i in uhtr_crate_num: # Use slot number default IP addresses
        uhtr_IPs.append(uhtr_ip_initial_bits+'.'+str(i*4))
print "uHTR IPs:"
print uhtr_IPs

###########################################################
#logger.info("Starting Crate Test with AMC13 connection file %s and Crate ID %d." % (conn_file, uhtr_ip_initial_bits))                                                                       
logger.info("Crate Test options:")
logger.info("Total events to dump = %d" % (options.max_evt))
logger.info("Events in a single dump (batch) = %d" % (options.tot_evt_dump))
logger.info("Mega Monitoring Prescale= %d" % (options.prescale))
logger.info("Internal trigger settings = localL1A %s" % (options.trig))

# Check options
if (options.prescale < 5 or options.prescale > 20):
    logger.error("Prescale Value (%d)  is out of range. Please use a prescale value between 5 and 20. Quit..." % (options.prescale))
    print "Prescale value (%d) is out of range. Please use a prescale value between 5 and 20." % (options.prescale)
    quit()

###############################
# Create directories results and output files and temporoarly files
###############################
# Create directories for the results (output files) and temporary script files                                                                                                    
print "Capturing Timestamp..."
logger.info("Capturing Timestamp...")
date_command = "date +'%s'" % (sv.TIMESTAMP)
p_date = subprocess.Popen(date_command, shell=True, stdout=subprocess.PIPE)
timestamp = shlex.split(p_date.communicate()[0])[0]
print "Timestamp: %s" % (timestamp)
logger.info("Timestamp captured: %s" % (timestamp))

logger.info("Making Run directory...")
run_dir_com = "mkdir Run%s" % (timestamp)
os.system(run_dir_com)
logger.info("Made directory: Run%s" % (timestamp))

tmpDir = "./Run%s/tmp" % (timestamp)
resultsDir = "./Run%s/results" % (timestamp)

logger.info("Making tmp and results directories...")
print "Making tmp and results directories..."
mkdir_command = "mkdir %s %s" % (tmpDir, resultsDir)
os.system(mkdir_command)
print "%s and %s directories made" % (tmpDir, resultsDir)
logger.info( "%s and %s directories made" % (tmpDir, resultsDir))

###############################
# Disable Links in AMC13
###############################

if options.reloadamc13 :
    print "Reloading AMC13 firmware from flash..."
    amc13_reload  = "%s -c %s/%s -X %s/amcReload.txt" % (amc_tool, addr_table, conn_file, script_dir)
    os.system(amc13_reload)
    # wait 10 seconds for reload
    time.sleep(10)

amc13_disable = "%s -c %s/%s -X %s/amcLinkDisable.txt" % (amc_tool, addr_table, conn_file, script_dir)
os.system(amc13_disable)

# Make sure to stop any triggers before preparing anything
# NOT YET DONE

###############################
# Enable DAQ Links and Start Run
###############################

# create appropriate amc13 script for initialization
amcPrepFile = open("./prepAMC.txt", "w" )
amcPrepFile.write("wv 0 1\n");
amcPrepFile.write("wv 0 2\n");
amcPrepFile.write("ws 0xd 0xfff\n");
amcPrepFile.write("wv 0x18 0xfff\n");
amcPrepFile.write("i");
for j in range(len(uhtr_crate_num)):  # change to full_crate_num if you add mctr compatibility to test
    amcPrepFile.write(" %d" % (uhtr_crate_num[j]) );
    if (j < len(uhtr_IPs)-1): # change to full_crate_num if you add mctr compatibility to test
        amcPrepFile.write(",");
amcPrepFile.write(" t,b\n");
amcPrepFile.write("localL1A %s" % (options.trig)); #replace trigger settings with input option
if (options.prescale==0):
    print "Prescale Off"
    amcPrepFile.write("prescale 0 1\n")
else:
    print "Prescale set to: %d" % (options.prescale)
    amcPrepFile.write("prescale 1 %d\n" % options.prescale);  
amcPrepFile.write("wv 0 1\n");
amcPrepFile.write("q\n");
amcPrepFile.close()

amc13_prep = "%s -c %s/%s -X %s/prepAMC.txt" % (amc_tool, addr_table, conn_file, script_dir)
os.system(amc13_prep)

################################
# Set up uHTR
################################

def uhtrSetup(typeStr,ipList):    

    # Check if DAQ Path is already disabled
    logger.info("Settting up %ss..." % (typeStr))

    for i in ipList:
        # Check if DAQ Path is already DISABLED
        logger.info("Checking DAQ path of %s at IP %s ..." % (typeStr,i))
        # open uHTRtool and save status to daqcheck.txt
        DAQ_enabled = False
        if (typeStr=="uHTR"):
            print "checking daq."
            daq_check_command =  "%s -u %s -s %s/checkDAQ.uhtr > %s/daqcheck.txt" % (htr_tool,i, script_dir, tmpDir)
        if (typeStr=="mCTR"):
            daq_check_command =  "%s/daqcheck.sh | %s %s -t uhtr > %s/daqcheck.txt" % (script_dir,mctr_tool,i, tmpDir)
        os.system(daq_check_command)
          # open daqcheck.txt and check to see if DAQ Path is enabled
        grep_command = "grep 'DAQ Path' %s/daqcheck.txt " % (tmpDir)
        p_grep = subprocess.Popen(grep_command,shell=True, stdout=subprocess.PIPE)
        check_whole = p_grep.communicate()[0]
        check_list = shlex.split(check_whole)
        if (len(check_list)>2):
            if (check_list[3]=='ENABLED'):
                # Disable DAQ Path if ENABLED
                logger.info("DAQ Path of %s at IP %s is ENABLED. Disabling..." % (typeStr,i))
                print "DAQ Path of %s at IP %s is ENABLED. Disabling..." % (typeStr,i)
                if (typeStr=="uHTR"):
                    u_daq_command = "%s -u %s -s %s/disableDAQPath.uhtr > /dev/null" % (htr_tool,i, script_dir)
                if (typeStr=="mCTR"):
                    u_daq_command = "%s/mctrDisDAQPath.sh | %s %s -t uhtr > /dev/null" % (script_dir,mctr_tool,i)
                os.system(u_daq_command)
            elif (check_list[3]=='DISABLED'):
                # Already Disabled  
                DAQ_enabled = False
                logger.info("DAQ Path of %s at IP %s is DISABLED." % (typeStr,i))
            else:
                logger.error("Problem while checking DAQ Path status. Quitting... ")
                print "Problem while checking DAQ Path status. Pass view tmp/daqcheck.txt."
                quit()
        else:
            logger.error("No line with 'DAQ Path' found in DAQ Path status or line was shorter than expected. Quitting...  Please check daqcheck.txt")
            print "No line with 'DAQ Path' found in DAQ Path status or line was shorter than expected \n Please check daqcheck.txt"
            quit()

        # Setup Clock and Reset Lumis
        if(typeStr=="uHTR"):
            print "Setting up uHTR clocks and Reseting Lumis. This may take a minute..."
            #logger.info("uHTR clock setup and lumi resets...")
            uhtr_clock_lumi = "%s/clockLumi.sh | %s -u %s > /dev/null" % ( script_dir,htr_tool,i )
            #uhtr_clock_lumi = "%s/clockLumi.sh | %s -u %s" % ( script_dir,htr_tool,i )
            os.system(uhtr_clock_lumi)
    
        # Enable DAQ Path
        if (DAQ_enabled==False):
            #logger.info("Enabling  DAQ Path for %s at IP %s" % (typeStr,i))
            print "Enabling DAQ Path for %s at IP %s" % (typeStr,i)
            if (typeStr=="uHTR"):
                u_daq_command = "%s -u %s -s %s/enableDAQPath.uhtr > /dev/null" % (htr_tool,i, script_dir)
            if (typeStr=="mCTR"):
                u_daq_command = "%s/mctrEnDAQPath.sh | %s %s -t uhtr > /dev/null" % (script_dir,mctr_tool,i)
            os.system(u_daq_command)
        else:
            print "DAQ Path for %s at IP %s already enabled" % (typeStr,i)
            logger.info("DAQ Path for %s at IP %s already enabled. Skipping enabling script..." % (typeStr,i))

        # Save initial DAQ Status
        if (typeStr=="uHTR"):
            daq_check_command =  "%s -u %s -s %s/checkDAQ.uhtr > %s/htrDaqStatInit%s.txt" % (htr_tool,i, script_dir, resultsDir,i)
        if (typeStr=="mCTR"):
            daq_check_command =  "%s/daqcheck.sh | %s %s -t uhtr> %s/htrDaqStatInit%s.txt" % (script_dir,mctr_tool,i, resultsDir,i)
        os.system(daq_check_command)

uhtrSetup("uHTR", uhtr_IPs)

###################################
# AMC13 send Event and Orbit Number Resets
###################################

amc_evn_orn_reset =  "%s -c %s/%s -X %s/amcEvnOrnReset.txt" % (amc_tool, addr_table, conn_file, script_dir)
os.system(amc_evn_orn_reset)

# uHTR reload (currently not necesary)
#for i in uhtr_IPs:
#    uhtr_reload = "%s -u %s -s %s/uhtrReload.uhtr > /dev/null" % (htr_tool, i, script_dir)
#    os.system(uhtr_reload)

###############################################
# Status and Data Capture
###############################################

# Capture initial AMC13 status                                                                                                                                                    
logger.info("Capturing initial AMC13 status")
amc13_status = "%s -c %s/%s -X %s/amcStatus.txt > %s/amcInitStat.txt" % (amc_tool,addr_table, conn_file, script_dir, resultsDir)
os.system(amc13_status)

#######################################################################                                                                                                           
# Loop until large number of events (~10^6) are dump (for overnight runs)                                                                                                         
########################################################################                                                                                                          
batch_no = 1
totalEventsDumped = 0
batchSize = options.tot_evt_dump
if (options.tot_evt_dump > options.max_evt):
    batchSize = options.max_evt

# Save uHTR Daq status immediately prior to sending triggers                                                                                                                      
for i in uhtr_IPs:
    # Check status                                                                                                                                                                
    daq_check_command =  "%s -u %s -s %s/checkDAQ.uhtr > %s/htrDaqStatB4Trig%s.txt" % (htr_tool,i, script_dir, resultsDir,i)
    os.system(daq_check_command)

# Send Triggers and Loop until Total Events number dumped                                                                                                                         
logger.info("Starting trigger sending, event capturing, and dumping...")
t_trig_start = "%s -c %s/%s -X %s/amcTrigStart.txt > /dev/null" % (amc_tool,addr_table, conn_file, script_dir)
t_trig_stop = "%s -c %s/%s -X %s/amcTrigStop.txt > /dev/null" % (amc_tool,addr_table, conn_file, script_dir)
while (totalEventsDumped < options.max_evt):

    print "Batch Num: %d" % (batch_no)
    logger.info("Batch Num: %d" % (batch_no))

    # Send local triggers
    logger.info("Sending triggers: localL1A %s ..." % (options.trig))
    print "Sending triggers: localL1A %s ..." % (options.trig)
    os.system(t_trig_start)

    evt_count = 0
    logger.info("Counting Events...")
    print "Counting Events..."
    while (evt_count < batchSize):
        time.sleep(1)
        check_evt_command = "%s -c %s/%s -X %s/amcStatus.txt > %s/evtStatus.txt" % (amc_tool, addr_table, conn_file, script_dir,tmpDir)
        os.system(check_evt_command)
        # May create a seperate function with robust inputs (at the moment this is a bit too hard-coded)
        find_evt_count = "grep 'UNREAD_BLK' %s/evtStatus.txt " % (tmpDir)
        p_evt = subprocess.Popen(find_evt_count,shell=True, stdout=subprocess.PIPE)
        evt_whole = p_evt.communicate()[0]
        evt_list = shlex.split(evt_whole)
        #print "evt whole: %s" % (evt_whole) # for debugging
        #print "evt[0] : %s" % (evt_list[0]) # for debugging
        #print "evt[1] : %s" % (evt_list[1]) # for debugging
        if (len(evt_list)>1):
            evt_count = int(evt_list[1].rstrip('\|'),0) #remove ending '|' and convert hexidecimal (with leading 0x) into int
            logger.info("Events Saved: %d" % (evt_count))
            print "Events Saved: %d" % (evt_count)
        else:
            logger.error("Problem checking saved events. Please check evtStatus.txt. Quitting...")
            print "Problem checking saved events. Please check evtStatus.txt"
            os.system(t_trig_stop)
            quit()
    logger.info("Batch size reached, stopping triggers and dumping...")
    print "Minimum number of events saved, stopping triggers and dumping..."
    os.system(t_trig_stop)

    # Dump events to file (this or start continuous dump early and stop it here)
    dumpScript = open("%s/dumpFile.txt" % (tmpDir), "w")
    dumpScript.write("st\n");
    dumpScript.write("df %s/dump%d.dat\n" % (resultsDir,batch_no));
    dumpScript.write("q\n");
    dumpScript.close()

    dump_evt_command = "%s -c %s/%s -X %s/dumpFile.txt > %s/amcEndStat%d.txt" % (amc_tool,addr_table, conn_file, tmpDir, resultsDir, batch_no)
    os.system(dump_evt_command)

    # Report dump
    # This should being implemented in a separate function, so as not to have multiple places of hard-coding
    find_evt_final_count = "grep 'UNREAD_BLK' %s/amcEndStat%d.txt " % (resultsDir,batch_no)
    pf_evt = subprocess.Popen(find_evt_final_count,shell=True, stdout=subprocess.PIPE)
    evt_f_whole = pf_evt.communicate()[0]
    evt_f_list = shlex.split(evt_f_whole)
    if (len(evt_f_list) > 1):
        evt_f_count = int(evt_f_list[1].rstrip('\|'),0)
        logger.info("%d events dumped to dump%d.dat with status saved to amcEndStat%d.txt" % (evt_f_count,batch_no,batch_no))
        print "%d events dumped to dump%d.dat with status saved to amcEndStat%d.txt" % (evt_f_count,batch_no,batch_no)
    else:
        logger.error("Problem with Saved events in SDRAM before dumping. Please view tmp/amcEndStat%d.txt. Quitting..." % (batch_no))
        print "Problem with Saved events in SDRAM before dumping. Please view tmp/amcEndStat%d.txt." % (batch_no)
        quit()

    # Increment counters                                                                                                                                                         
    batch_no += 1
    totalEventsDumped += evt_f_count
    logger.info("Total events dumped so far: %d" %(totalEventsDumped) )
    print "TotEvt: %d" %(totalEventsDumped)

print "Total Evts Reached. Disabling uHTR DAQ Paths..."
# Disable uHTR DAQ Path                                                                                                                                                          
for i in uhtr_IPs:
    u_daq_command = "%s -u %s -s %s/disableDAQPath.uhtr > /dev/null" % (htr_tool,i, script_dir)
    os.system(u_daq_command)

print "Run Complete"
logger.info("Run Complete.")
