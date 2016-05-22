#ifndef AMC13_FEATURES_HH_INCLUDED
#define AMC13_FEATURES_HH_INCLUDED 1

// flags used by getVersionDependentFeatures()

// unknown version number, features not determined
#define AMC13_FEATURE_UNKNOWN (1<<31)

// 5Gb/s S-Link express outputs
#define AMC13_FEATURE_SLINK_EXPRESS_5G 1
// 10Gb/s S-link Express outputs
#define AMC13_FEATURE_SLINK_EXPRESS_10G 2
// 10Gb/s TCP/IP G-2 format outputs
#define AMC13_FEATURE_G2_TCP_10G 4
// HCAL local trigger
#define AMC13_FEATURE_HCAL_TRIGGER 8
// 2-bit field for number of DAQ links implemented (nom. 2 or 3)
#define AMC13_FEATURE_DAQ_LINK_COUNT_MASK 0x30
// This is AMC test firmware
#define AMC13_FEATURE_AMC_TEST 0x40

// Random rate is 7X the programmed value
#define AMC13_FEATURE_RANDOM_TIMES7 0x80

#define AMC13_FEATURE_DAQ_LINK_COUNT_MASK_BIT (0x30&(-0x30))

#endif
