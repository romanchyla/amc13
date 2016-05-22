#!/usr/bin/python

from optparse import OptionParser
import systemVars as sv
import sys as sys
import os as os
import time as time


def storeConfig(c_dir, host, slot, fname):
    tool = c_dir+"/amc13config"
    COMMAND = "%s --host=%s --slot=%d --enable --store=%s --autoslot" % (tool, host, slot, fname)
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
parser.add_option("-p", "--spartan", dest="s_IP",
                  help="spartan IP address to be assigned",
                  default="", metavar="<ip_addr>",
                  type=str)
parser.add_option("-v", "--virtex", dest="v_IP",
                  help="virtex IP address to be assigned",
                  default="", metavar="<ip_addr>",
                  type=str)
parser.add_option("-i", "--ip", dest="IP",
                  help="IP address to be assigned",
                  default="", metavar="<ip_addr>",
                  type=str)
parser.add_option("-n", "--serial", dest="sn",
                  help="AMC13 Serial Number",
                  default=0, metavar="<n>", type=int)

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
    dest_ipmb = 0xa4
else:
    dest_ipmb = (0x70+(2*slot))

print "Storing IP addresses to board in slot %d from host %s" % (slot,host)

#Set the configuration bytes to be programmed
slot_bytes = []
net_base_bytes = []
vir_ipAddr_bytes = []
spa_ipAddr_bytes = []
boot_vector_bytes = []

#Set the slot number byte
slot_bytes.append(0x00)
#Set the net base bytes
net_base_bytes.append(0xff)
net_base_bytes.append(0xff)
net_base_bytes.append(0xff)
net_base_bytes.append(0x00)
#Set the IP address bytes
if( options.sn != 0 and (options.s_IP != "" or options.v_IP != "" or options.IP != "") ):
    print "Cannot set the IP address with _both_ SN and with explicit IP request"
    sys.exit(1)
elif( options.sn != 0 and options.s_IP == "" and options.v_IP == "" and options.IP == ""):
    print "Setting IP addresses using SN %d" % options.sn
    spa_ipAddr_bytes = sv.NETWORK_BASE.split(".")
    spa_ipAddr_bytes.append(254-(2*options.sn))
    vir_ipAddr_bytes = sv.NETWORK_BASE.split(".")
    vir_ipAddr_bytes.append(255-(2*options.sn))
    for i in range(len(spa_ipAddr_bytes)):
        try:
            spa_ipAddr_bytes[i] = int(spa_ipAddr_bytes[i])
            vir_ipAddr_bytes[i] = int(vir_ipAddr_bytes[i])
        except ValueError:
            print "Trouble converting IP address bytes to integers"
            sys.exit(1)
elif( options.IP == "" and options.s_IP == "" and options.v_IP == "" ):
    print "Must specify ip address to be assigned"
    sys.exit(1)
elif( (options.IP != "" and (options.s_IP != "" or options.v_IP != "")) ):
    print "Cannot specify IP address using -i as well as -si and/or -vi"
    sys.exit(1)
elif( (options.s_IP != "" and options.v_IP == "") or (options.s_IP == "" and options.v_IP != "") ):
    print "Must specify both T1 and T2 IP addresses using -vi and -si"
    sys.exit(1)
elif( options.IP != "" and options.s_IP == "" and options.v_IP == "" ):
    spa_ipAddr_bytes = options.IP.split(".")
    for i in range(len(spa_ipAddr_bytes)):
        try:
            spa_ipAddr_bytes[i] = int(spa_ipAddr_bytes[i])
        except ValueError:
            print "Provided IP address must be decimal numbers delimited by periods"
            sys.exit(1)
        if(i < 3):
            vir_ipAddr_bytes.append(spa_ipAddr_bytes[i])
        else:
            vir_ipAddr_bytes.append(spa_ipAddr_bytes[i]+1)
elif( options.IP == "" and options.s_IP != '' and options.v_IP != "" ):
    spa_ipAddr_bytes = options.s_IP.split(".")
    vir_ipAddr_bytes = options.v_IP.split(".")
    for i in range(len(spa_ipAddr_bytes)):
        try:
            spa_ipAddr_bytes[i] = int(spa_ipAddr_bytes[i])
            vir_ipAddr_bytes[i] = int(vir_ipAddr_bytes[i])
        except ValueError:
            print "Provided IP addresses must be decimal numbers delimited by periods"
            sys.exit(1)
#Set the boot vector bytes
boot_vector_bytes.append(0x00)
boot_vector_bytes.append(0x00)

if(len(slot_bytes) != 1):
    print "Only allowed 1 slot specification"
    sys.exit(1)
if(len(net_base_bytes) != 4):
    print "Only allowed 4 bytes for the net base"
    sys.exit(1)
if(len(vir_ipAddr_bytes) != 4):
    print "Only allowed 4 bytes in T1 IP address"
    sys.exit(1)
if(len(spa_ipAddr_bytes) != 4):
    print "Only allowed 4 bytes in T2 IP address"
    sys.exit(1)
if(len(boot_vector_bytes) != 2):
    print "Only allowed 2 bytes in boot vector"
    sys.exit(1)

#Write the configuration bytes to a file for the config_tool to handle
FILE_NAME = "temp_ipassn.conf"
f = open(FILE_NAME, 'w')
#Write the bytes for Port0
f.write("Port0\n")
for i in slot_bytes:
    f.write("%02x " % (i) )
f.write("\n")
for i in net_base_bytes:
    f.write("%02x " % (i) )
f.write("\n")
for i in spa_ipAddr_bytes:
    f.write("%02x " % (i) )
f.write("\n")
for i in boot_vector_bytes:
    f.write("%02x " % (i) )
f.write("\n\n")
#Write the bytes for Port1
f.write("Port1\n")
for i in slot_bytes:
    f.write("%02x " % (i) )
f.write("\n")
for i in net_base_bytes:
    f.write("%02x " % (i) )
f.write("\n")
for i in vir_ipAddr_bytes:
    f.write("%02x " % (i) )
f.write("\n")
for i in boot_vector_bytes:
    f.write("%02x " % (i) )
f.write("\n\n")
#Close the file
f.close()

#Store the configuration
print "Config is in $FILE_NAME\n";
# storeConfig(CONFIG_DIR, host, slot, FILE_NAME)

#Get rid of the temp file
# os.system("rm "+FILE_NAME)
