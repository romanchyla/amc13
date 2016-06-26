#!/bin/env python
import os as os
import time as time
import subprocess as subprocess
import shlex as shlex

try:
    import uhal
    import amc13
except:
    uhal = None
    amc13 = None
    

def fail_fast():
    """Function to raise warnings if uhal/amc13 cannot be found."""
    if not uhal or not amc13:
        raise Exception('Python modules for uhal/amc13 cannot be loaded!')

def get_amc_version(xml_config):
    uhal.setLogLevelTo(uhal.LogLevel.ERROR) #TODO: this is setting log level globally, which is not very nice...
    device = amc13.AMC13(xml_config)
    
    return (hex(device.read(device.Board.T1, 'STATUS.FIRMWARE_VERS')),
            hex(device.read(device.Board.T2, 'STATUS.FIRMWARE_VERS')))

def get_ip_from_slot(crate_ip, slot, cmd_base=0x32, cmd=0x34, spi=0, addr_lo=0xb, addr_hi=0x0, addr_len=0x4):
    """ readIP function will transfer the slot numbers into the ip. """
    if(slot > 13 or slot < 1):
        raise Exception("Slot number must be 1-13")
    elif(slot == 13):
        ipmb = 0xa4
    else:
        ipmb = (0x70+(2*slot))
<<<<<<< HEAD
    ipmi_base = "ipmitool -H '%s' -U '' -P '' -T 0x%x -b %d -t 0x%x" % (crate_ip, 0x82, 7, ipmb)
=======
<<<<<<< HEAD
    ipmi_base = "ipmitool -H '%s' -U '' -P '' -T 0x%x -b %d -t 0x%x" % (crate_ip, 0x82, 7, ipmb)
=======
    ipmi_base = "ipmitool -H '' -U '' -P '' -T 0x%x -b %d -t 0x%x" % (0x82, 7, ipmb)
>>>>>>> 658a836b63c05ee62af57addbaac22d79f34baac
>>>>>>> d34c39d0d5c031a00c9e399207067b96ccde2b5c
    IPMI_INCANTATION = "%s raw 0x%02x 0x%02x %d %d %d %d" % (ipmi_base, cmd_base, cmd, spi, addr_lo, addr_hi, addr_len)
    print IPMI_INCANTATION
    args = shlex.split(IPMI_INCANTATION)
    p = subprocess.Popen(args, stdout=subprocess.PIPE)
    out, err = p.communicate()
    hexIP = shlex.split(out)
    decIP = []
    for i in hexIP:
        decIP.append(str(int(i, 16)))
    return hexIP, ".".join(decIP)
