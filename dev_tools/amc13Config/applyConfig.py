#!/usr/bin/python

from optparse import OptionParser
import systemVars as sv 
import sys as sys
import os as os
import time as time


def doIPMI(ipmi_base, cmd_base, cmd, spi, addr_lo, addr_hi, len, slot, net, ip, boot):
    IPMI_INCANTATION = "%s raw 0x%02x 0x%02x %d %d %d %d 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x" % (ipmi_base, cmd_base, cmd, spi, addr_lo, addr_hi, len, slot[0], net[0], net[1], net[2], net[3], ip[0], ip[1], ip[2], ip[3], boot[0], boot[1])
    #print IPMI_INCANTATION
    os.system(IPMI_INCANTATION)
    return

def IPMIconfig(ipmi_base, cmd_base, cmd, spi):
    arg = 0x22
    IPMI_INCANTATION = "%s raw 0x%02x 0x%02x %d 0x%02x" % (ipmi_base, cmd_base, cmd, spi, arg)
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
#Set the configuration bytes to be programmed
slot_bytes = []
net_base_bytes = []
vir_ipAddr_bytes = []
spa_ipAddr_bytes = []
boot_vector_bytes = []

#Print out details of script
print "Applying IP addresses to board in slot %d from host %s" % (slot,host)

#Set the slot number byte
slot_bytes.append(slot)
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

#Carry out the IPMITool commands
IPMI_BASE = "ipmitool -H %s -U %s -P %s -T 0x%x -b %d -t 0x%x" % (host, username, password, transit_address, transit_channel, dest_ipmb)

#Write to SPI ports
COMMAND_BASE = 0x32
COMMAND = 0x33
ADDR_LSB = 0x0
ADDR_MSB = 0x0
WRITE_LEN = 11
#Write to Port0
SPI_PORT = 0
doIPMI(IPMI_BASE, COMMAND_BASE, COMMAND, SPI_PORT, ADDR_LSB, ADDR_MSB, WRITE_LEN, slot_bytes, net_base_bytes, spa_ipAddr_bytes, boot_vector_bytes)
time.sleep(.05)
#Write to Port1
SPI_PORT = 1
doIPMI(IPMI_BASE, COMMAND_BASE, COMMAND, SPI_PORT, ADDR_LSB, ADDR_MSB, WRITE_LEN, slot_bytes, net_base_bytes, vir_ipAddr_bytes, boot_vector_bytes)
time.sleep(.05)

#Configure FPGAs
COMMAND_BASE = 0x32
COMMAND = 0x32
#Configure FPGA 0
SPI_PORT = 0
IPMIconfig(IPMI_BASE, COMMAND_BASE, COMMAND, SPI_PORT)
time.sleep(1)
#Configure FPGA 1
SPI_PORT = 1
IPMIconfig(IPMI_BASE, COMMAND_BASE, COMMAND, SPI_PORT)
time.sleep(1)
