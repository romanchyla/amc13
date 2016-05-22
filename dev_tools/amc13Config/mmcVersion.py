#!/usr/bin/python

from optparse import OptionParser
import systemVars as sv
import os as os
import time as time

def readVer(ipmi_base, cmd_base, cmd, addr_lo, addr_hi, len):
    IPMI_INCANTATION = "%s raw 0x%02x 0x%02x %d %d %d" % (ipmi_base, cmd_base, cmd, addr_lo, addr_hi, len)
    #print IPMI_INCANTATION
    os.system(IPMI_INCANTATION)
    return

parser = OptionParser()
parser.add_option("-o", "--host", dest="host",
                  help="MCH IP address", default=sv.DEFAULT_HOST_IP,
                  metavar="<ip_addr>", type=str)
parser.add_option("-s", "--slot", dest="slot",
                  help="uTCA slot number (1-13)", default=sv.DEFAULT_AMC13_SLOT,
                  metavar="<n>", type=int)

(options, args) = parser.parse_args()

host = options.host
username = "''"
password = "''"
transit_address = 0x82
transit_channel = 7
slot = options.slot
if(slot > 13 or slot < 1):
    print "Slot number must be 1-13"
    sys.exit(1)
elif(slot == 13):
    ipmb = 0xa4
else:
    ipmb = (0x70+(2*slot))

IPMI_BASE = "ipmitool -H %s -U %s -P %s -T 0x%x -b %d -t 0x%x" % (host, username, password, transit_address, transit_channel, ipmb)

print "Reading MMC version of board in slot %d from host %s" % (slot,host)

#Read MMC Revision
COMMAND_BASE = 0x32
COMMAND = 0x42
ADDR_LSB = 0x5
ADDR_MSB = 0x0
READ_LEN = 0x2
print "MAJOR and MINOR release numbers:"
readVer(IPMI_BASE, COMMAND_BASE, COMMAND, ADDR_LSB, ADDR_MSB, READ_LEN)
