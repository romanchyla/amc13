#!/bin/env python

import uhal
import amc13
import sys

if __name__ == '__main__':
    
    usage = '''
    Usage:
        %s <connection.xml>
    '''
    if len(sys.argv) != 2:
        print usage % sys.argv[0]
        sys.exit(-1)

    uhal.setLogLevelTo(uhal.LogLevel.ERROR)
    device = amc13.AMC13(sys.argv[1])


    print 'T1 firmware version', hex(device.read(device.Board.T1, 'STATUS.FIRMWARE_VERS'))
    print 'T2 firmware version', hex(device.read(device.Board.T2, 'STATUS.FIRMWARE_VERS'))