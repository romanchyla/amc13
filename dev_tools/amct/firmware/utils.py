#!/bin/env python

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