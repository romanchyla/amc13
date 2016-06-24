#!/usr/bin/env python

from optparse import OptionParser
import systemVars as sv
import os as os
import time as time
import subprocess as subprocess
import shlex as shlex

def readIP(ipmi_base, cmd_base, cmd, spi, addr_lo, addr_hi, len):
    IPMI_INCANTATION = "%s raw 0x%02x 0x%02x %d %d %d %d" % (ipmi_base, cmd_base, cmd, spi, addr_lo, addr_hi, len)
    print IPMI_INCANTATION
    args = shlex.split(IPMI_INCANTATION)
    p = subprocess.Popen(args, stdout=subprocess.PIPE)
    out, err = p.communicate()
    hexIP = shlex.split(out)
    decIP = []
    for i in hexIP:
        decIP.append(int(i, 16))
    print(hexIP)
    print(decIP)
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

print "Reading IP addresses of board in slot %d from host %s" % (slot,host)

#Read SPI ports
COMMAND_BASE = 0x32
COMMAND = 0x34
ADDR_LSB = 0xb
ADDR_MSB = 0x0
READ_LEN = 0x4
#Read SPI Port0
SPI_PORT = 0
print "T2 IP Bytes:"
readIP(IPMI_BASE, COMMAND_BASE, COMMAND, SPI_PORT, ADDR_LSB, ADDR_MSB, READ_LEN)
#Read SPI Port1
SPI_PORT = 1
print "T1 IP Bytes:"
readIP(IPMI_BASE, COMMAND_BASE, COMMAND, SPI_PORT, ADDR_LSB, ADDR_MSB, READ_LEN)
