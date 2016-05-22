#!/usr/bin/perl
#
# generate local trigger input sample 0x1022-0x103f
#
for( my $n=0; $n<30; $n++) {
    $addr = 0x1022 + $n;
    printf qq{0x%08x,0,11,"r","STATUS.LTRIG.SAMPLE%02d.BCN","HCAL trigger test rx",,,9,,"HCAL_Trigger_Input_test","Sample_%02d","BCN"\n}, $addr, $n, $n;
    printf qq{0x%08x,12,14,"r","STATUS.LTRIG.SAMPLE%02d.VER","HCAL trigger test rx",,,9,,"HCAL_Trigger_Input_test","Sample_%02d","Ver"\n}, $addr, $n, $n;
    printf qq{0x%08x,15,15,"r","STATUS.LTRIG.SAMPLE%02d.BC0","HCAL trigger test rx",,,9,,"HCAL_Trigger_Input_test","Sample_%02d","BC0"\n}, $addr, $n, $n;
    printf qq{0x%08x,16,23,"r","STATUS.LTRIG.SAMPLE%02d.DATA","HCAL trigger test rx",,,9,,"HCAL_Trigger_Input_test","Sample_%02d","Data"\n}, $addr, $n, $n;
    printf qq{0x%08x,31,31,"r","STATUS.LTRIG.SAMPLE%02d.ERR","HCAL trigger test rx",,,9,,"HCAL_Trigger_Input_test","Sample_%02d","Err"\n}, $addr, $n, $n;    
}
