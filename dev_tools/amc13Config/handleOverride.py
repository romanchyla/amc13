#!/usr/bin/python

from optparse import OptionParser
import systemVars as sv
import sys as sys
import os as os
import time as time

def controlHandle(c_dir, host, action, ipmb):
    tool = c_dir+"/handle_override"
    COMMAND = "%s %s %s 0x%x" % (tool, host, action, ipmb)
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
parser.add_option("-i", "--in", action="store_true", dest="push_in",
                  help="Force handle in", default=False,
                  metavar="<bool>")
parser.add_option("-u", "--out", action="store_true", dest="push_out",
                  help="Force handle out", default=False,
                  metavar="<bool>")
parser.add_option("-r", "--release", action="store_true", dest="release",
                  help="Release handle", default=False,
                  metavar="<bool>")
parser.add_option("-c", "--cycle", action="store_true", dest="cycle",
                  help="Force handle out for 10 seconds, then release", default=False,
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

if(options.push_in and not options.push_out and not options.release and not options.cycle):
    action = "in"
elif(not options.push_in and options.push_out and not options.release and not options.cycle):
    action = "out"
elif(not options.push_in and not options.push_out and options.release and not options.cycle):
    action = "release"
elif(not options.push_in and not options.push_out and not options.release and options.cycle):
    action = "cycle"
else:
    print "Cannot specify more than one action!"
    sys.exit(1)

print "Issuing handle override to board in slot %d from host %s" % (slot,host)

controlHandle(CONFIG_DIR, host, action, ipmb)
print "Wait 10 seconds while the handle cycles..."
time.sleep(10)
