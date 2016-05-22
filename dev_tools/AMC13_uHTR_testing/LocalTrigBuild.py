#! /usr/bin/env python

#Script to setup and run event build from uHTR using AMC13local trigger test using uHTRTool and AMC13Tool
#assume tools have been setup and made

from optparse import OptionParser
import systemVars as sv
import os as os
import time as time
import subprocess as subprocess
import shlex as shlex

######################
# Parse Options
######################

parser = OptionParser()
parser.add_option("-r", "--reloadamc13", 
                  action="store_true", dest="reloadamc13", default=False,
                  help="If set, complete reset of AMC13 (reload firmware) at start of test. ")
(options, args) = parser.parse_args()

##############################
# Set directories and tools
##############################

script_dir = sv.SCRIPT_DIR
tmpDir = script_dir+"/tmp"
resultsDir = tmpDir

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

for i in uhtr_crate_num:
    uhtr_IPs.append(uhtr_ip_initial_bits+'.'+str(i*4))
print "uHTR IPs:"
print uhtr_IPs

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
amcPrepFile.write(" t\n");
amcPrepFile.write("localL1A b 1 150\n"); #replace trigger settings with input option
amcPrepFile.write("wv 0 1\n");
amcPrepFile.write("q\n");
amcPrepFile.close()

amc13_prep = "%s -c %s/%s -X %s/prepAMC.txt" % (amc_tool, addr_table, conn_file, script_dir)
#amc13_prep = "%s -c %s/%s -X %s/amcEnable.txt" % (amc_tool, addr_table, conn_file, script_dir)
os.system(amc13_prep)

################################
# Set up uHTR
################################

def uhtrSetup(typeStr,ipList):    

    # Check if DAQ Path is already disabled
    #logger.info("Settting up %ss..." % (typeStr))

    for i in ipList:
        # Check if DAQ Path is already DISABLED
        #logger.info("Checking DAQ path of %s at IP %s ..." % (typeStr,i))
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
                #logger.info("DAQ Path of %s at IP %s is ENABLED. Disabling..." % (typeStr,i))
                print "DAQ Path of %s at IP %s is ENABLED. Disabling..." % (typeStr,i)
                if (typeStr=="uHTR"):
                    u_daq_command = "%s -u %s -s %s/disableDAQPath.uhtr > /dev/null" % (htr_tool,i, script_dir)
                if (typeStr=="mCTR"):
                    u_daq_command = "%s/mctrDisDAQPath.sh | %s %s -t uhtr > /dev/null" % (script_dir,mctr_tool,i)
                os.system(u_daq_command)
            elif (check_list[3]=='DISABLED'):
                # Already Disabled  
                DAQ_enabled = False
                #logger.info("DAQ Path of %s at IP %s is DISABLED." % (typeStr,i))
            else:
                #logger.error("Problem while checking DAQ Path status. Quitting... ")
                print "Problem while checking DAQ Path status. Pass view tmp/daqcheck.txt."
                quit()
        else:
            #logger.error("No line with 'DAQ Path' found in DAQ Path status or line was shorter than expected. Quitting...  Please check daqcheck.txt")
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

#for i in uhtr_IPs:
#    uhtr_reload = "%s -u %s -s %s/uhtrReload.uhtr > /dev/null" % (htr_tool, i, script_dir)
#    os.system(uhtr_reload)
