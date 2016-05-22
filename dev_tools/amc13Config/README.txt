
**************** IPMI Control Software ****************

This set of python scripts are for carrying out IPMI actions
remotely on an AMC13, for programming the flash memory on the
MMC EEPROM, and for configuring the IP address of each AMC13 
FPGA via the MMC-FPGA SPI Interface.

Before using the software, it is important that you make sure 'systemVars.py'
contains the correct information for your system. The variables are described below:
  DEFAULT_HOST_IP --> The IP address of your commercial MCH module in the uTCA crate
  DEFAULT_AMC13_SLOT --> The desired default slot value for your AMC13
  DEFAULT_CONFIG_DIR --> The location of the PERL tools which are used for MMC actions
  NETWORK_BASE --> The network base for all IP addresses in your uTCA crate

NOTE: All options within parentases () are optional and have
default values
NOTE: Slot numbers are one-based (allowed range is 1-13)

*** ./readIPs.py (--host=<ip_addr>) (--slot=<n>)
    This script will read out the IP address bytes from the specified AMC13's
    Spartan and Virtex SPI ports.
    If '--host' and/or '--slot' is not specified, they will take on their
    default values in 'systemVars.py'

*** ./readNVmem.py (--host=<ip_addr>) (--slot=<n>)
    This script will read out 40 bytes from non-volitile memory at an address 
    specified by a NV area IPMI read (i.e. the address written to by 'storeConfig.py',
    which is documented below.
    If '--host' and/or '--slot' is not specified, they will take on their
    default values in 'systemVars.py'

*** ./applyConfig.py (--host=<ip_addr>) (--slot=<n>) --spartan=<ip_addr>
                     --virtex=<ip_addr>
    This script will apply the specified Spartan and Virtex IP address
    to each chip on the AMC13.
    Optionally, you can specify only the Spartan IP address using --ip=<ip_addr>,
    which will assign the Spartan the given address and assign the Virtex an IP
    with its LSB one integer greater than that of the spartan, i.e.
      --ip=192.168.1.100
      ** Spartan IP --> 192.168.1.100
      ** Virtex IP  --> 192.168.1.101
    As yet another option, you can specify the AMC13 serial number using --serial=<n>
    instead of the IP addresses, which will assign IP addresse to both chips based 
    on the following scheme
      --serial=SN
      ** Spartan IP --> NETWORK_BASE.(254-(2*SN))
      ** Virtex  IP --> NETWORK_BASE.(255-(2*SN))
    If the '--host' and/or '--slot' are not specified, they take on their 
    default values in 'systemVars.py'

*** ./storeConfig.py (--host=<ip_addr>) (--slot=<n>) --spartan=<ip_addr>
                     --virtex=<ip_addr>
    This script will write the provided slot number and ip addresses to the 
    MMC EEPROM and will enable the automatic configuration of the FPGAs from this
    non-volitile memory. This is your way of telling the AMC13 which IP addresses
    to take on upon power up.
    The details of the argumetns of this script are identical to those of "applyConfig.py"

*** ./backendPower.py (--host=<ip_addr>) (--slot=<n>) --enable|--disable
    This script will enable or disable the backend power to the AMC13 at the specified slot.
    If '--host' and/or '--slot' are not specified, then they will take on the default values 
    as assigned in 'systemVars.py'

*** ./coldReset.py (--host=<ip_addr>) (--slot=<n>)
    This script will issue a cold reset to the AMC13 module at the specified slot.
    If '--host- and/or '--slot' are not explicitly specified, they take on their defulat
    values as assigned in 'systemVars.py'
 
*** ./handleOverride.py (--host=<ip_addr>) (--slot=<n>) --in|--out|--release|--cycle
    This script remotely forces the handle to be pushed in (via --in), pulled out (via --out),
    released to whatever the hardware configuratoin is (via --released), or pulled out for 10
    seconds, and then pushed back in againg (via --cycle).
    If '--host' and/or '--slot' are not explicitly specified, they take on their defult values
    as assigned in 'systemVars.py'

*** ./mmcVersion.py (--host=<ip_addr>) (--slot=<n>)
    This script will read out the major and minor release numbers for the MMC
    on the specified AMC13
    If '--host' and/or '--slot' is not specified, they will take on their
    default values in 'systemVars.py'

*** ./scanCrate.pl
    This script will scan through all the slots in the crate for AMC13s 
    and report serial number,IP, MCH versions. Requires environment setup 
    so that working AMC13Tool.exe is in the path and also needs to be run
    from the 'amc13Config' directory, so that the python tools mmcVersion.py,
    etc. can be found.
