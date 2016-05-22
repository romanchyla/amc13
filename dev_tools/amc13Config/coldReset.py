#!/usr/bin/python

from optparse import OptionParser
import systemVars as sv
import os as os
import time as time

def issueColdReset(c_dir, host, fru):
    tool = c_dir+"/cold_reset"
    COMMAND = "%s %s %d" % (tool, host, fru)
    #print COMMAND
    os.system(COMMAND)
    return

CONFIG_DIR = sv.DEFAULT_CONFIG_DIR

parser = OptionParser()
parser.add_option("-o", "--host", dest="host",
                  help="MCH IP address", default=sv.DEFAULT_HOST_IP,
                  metavar="<ip_addr>", type=str)
parser.add_option("-s", "--slot", dest="slot",
                  help="uTCA slot number (1-13)", default=sv.DEFAULT_AMC13_SLOT,
                  metavar="<n>", type=int)

(options, args) = parser.parse_args()

host = options.host
slot = options.slot
if(slot > 13 or slot < 1):
    print "Slot number must be 1-13"
    sys.exit(1)
elif(slot == 13):
    fru = 30
else:
    fru  = (4+slot)

print "Issuing cold reset to board in slot %d from host %s" % (slot,host)

issueColdReset(CONFIG_DIR, host, fru)
time.sleep(1)
