./amc13config --host=192.168.1.240 [--slot=13] [--user=root] [--pass=admin] [--enable|--disable] [--store=config.txt] [--apply=config.txt] [--autoslot]
  --enable   Enables auto configuration on this card.
  --disable  Disables auto configuration on this card.
  --store    Writes the given configuration to non-volatile memory (--enable not implied)
  --apply    Writes the given configuration to the FPGA directly if REQ_CFG&!CFG_RDY (volatile)
  --autoslot *Replaces* the first byte of the configuration file dynamically with the card's slot.
             When used with --store, it will enable this functionality in the MMC.
			 When --store is used without this option, it will be disabled.

###

An example config file for our current standard format is available in ipassign.conf

The config file consists of hexadecimal bytes, whitespace separated, and ignores from # to the end of the line as comments.

Our current config format for our cards is
1-4:  netmask
5-8:  ip address
9-10: boot vector

###

./backend_power ip_addr on|off [ipmb_addr]
  Enables or disables backend power.
  Default IPMB Address = '0xa4' (AMC13 Slot)

###

./cold_reset ip_addr [fru]
  Instructs the MMC to perform a cold reset.
  Default FRU = 30 (AMC13 Slot)

###

./handle_override ip_addr in|out|cycle|release [ipmb_addr]

  Overrides the handle to the specified position.
  'in' = handle forced in
  'out' = handle forced out
  'cycle' = handle forced out for 10 seconds then released
  'release' = handle not forced.  hardware switch used

  Default IPMB Address = '0xa4' (AMC13 Slot)
