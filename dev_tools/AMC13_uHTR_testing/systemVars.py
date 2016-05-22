#system variables for local triggers initialization of AMC13 w/ uHTR

#### Change for different boards ####

# Connection file name that has configuration for AMC13 to be used in test
#CONN_FILE = "connectionSN86.xml" 
CONN_FILE = "connectionSN86.xml" 

# List of uHTR IP addresses
UHTR_LIST = ["192.168.115.28", "192.168.115.32"]

# Slot numbers for uHTRs to be used in test (always necessary)
# (1 based, i.e. AMC slot number visible on crate)
UHTR_SLOT = [7,8]

# uHTR initial bits
# The whole IP address should be the below value followed by a number calculated by the slot number of the uHTR being used: [slot number]*4 
UHTR_IP_BITS_INIT = "192.168.115"

#### One time directory settings ####

# Path to directory with all required scripts (including LocalTrigBuild.py)
# LocalTrigBuild.py should be run from this directory
SCRIPT_DIR = "/home/dzou/work/amc13/dev_tools/AMC13_uHTR_testing"

# Path to AMC13Tool.exe
AMC_TOOL=  "/opt/cactus/bin/amc13/AMC13Tool2.exe"

# Path to uHTRtool.exe
HTR_TOOL= "uHTRtool.exe"

# Path to connection files directory
ADDR_TABLE = "/home/dzou/work/amc13/amc13/etc/amc13"

### Things to add ###
TIMESTAMP = "_%Y_%m_%d__%p_%I_%M"



