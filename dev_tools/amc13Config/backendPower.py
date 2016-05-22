#!/usr/bin/python

from optparse import OptionParser
import systemVars as sv
import sys as sys
import os as os
import time as time

def controlBackendPower(c_dir, host, power, ipmb):
    tool = c_dir+"/backend_power"
    COMMAND = "%s %s %s 0x%x" % (tool, host, power, ipmb)
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
parser.add_option("-e", "--enable", action="store_true", dest="power",
                  help="Enable backend power", default=None,
                  metavar="<bool>")
parser.add_option("-d", "--disable", action="store_false", dest="power",
                  help="Disable backend power", default=None,
                  metavar="<bool>")

(options, args) = parser.parse_args()

host = options.host
slot = options.slot
if(slot > 13 or slot < 1):
    print "Slot number must be 1-13"
    sys.exit(1)
elif(slot == 13):
    ipmb = 0xa4
else:
    ipmb = (0x70+(2*slot))

if(options.power == None):
    print "Must specify whether to enable or disable backend power"
    sys.exit(1)
elif(options.power == False):
    power = "off"
elif(options.power == True):
    power = "on"

print "Turning backend power %s for board in slot %d from host %s" % (power,slot,host)

controlBackendPower(CONFIG_DIR, host, power, ipmb)
time.sleep(1)
