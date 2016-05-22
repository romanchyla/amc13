// AMC13 class inheriting AMC13Simple class

#include "amc13/AMC13Simple.hh"
#include "amc13/AMC13.hh"
#include "amc13/Exception.hh"
#include "amc13/Features.hh"
#include "uhal/uhal.hpp"

// use PRIu64 etc format specifiers in printf()
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <vector>

static char *AMC13_fmt_bits( int v, int siz)
{
  static char s[64];
  if( siz < 1 || siz > 64) {
    printf("Invalid size %d in fmt_bits\n", siz);
    exit(1);
  }
  for( int i=0; i<siz; i++)
    s[i] = (v >> (siz-i-1)) & 1 ? '1' : '0';
  s[siz] = '\0';
  return s;
}

namespace amc13 {

  /*! \brief Constructor using a connection file on disk
    \param ConnectionMap path to XML connection file specifying two ids "T1" and "T2" with address tables
  */
  AMC13::AMC13( const std::string& ConnectionMap )
    : AMC13Simple( ConnectionMap, "T1", "T2" ){
    initAMC13();
  }

  /*! \brief Constructor using a connection file on disk
    \param ConnectionMap path to XML connection file specifying two ids t1id and t2id with address tables
  */
  AMC13::AMC13( const std::string& ConnectionMap, const std::string& t1id, const std::string& t2id )
    : AMC13Simple( ConnectionMap, t1id, t2id ){
    initAMC13();
  }  

  /*! \brief Constructor using user-provided URI and address tables
    \param p_URI1 URI for T1 board
    \param p_AddMap1 XML address table path for T1 board
    \param p_URI2 URI for T2 board
    \param p_AddMap2 XML address table path for T1 board
  */
  AMC13::AMC13(const std::string& p_URI1, const std::string& p_AddMap1 ,
	const std::string& p_URI2, const std::string& p_AddMap2)
    : AMC13Simple( p_URI1 , p_AddMap1, p_URI2, p_AddMap2 ) {
    initAMC13(); 
  }

  /*! \brief Constructor using user-provided HwInterface objects 
    \param t1 HwInterface for T1 board
    \param t2 HwInterface for T2 board
  */
  AMC13::AMC13(const uhal::HwInterface& t1, const uhal::HwInterface& t2)
    : AMC13Simple( t1, t2 ) {
    initAMC13(); 
  }

  /// Destructor
  AMC13::~AMC13() {
    //delete the flash if allocated
    if(flash != NULL){
      delete flash ;
    }
    //delete the status if allocated
    if(status != NULL){
      delete status;
    }
  }

  // Initialize AMC13
  void AMC13::initAMC13() {
    uint32_t t1r1, t2r1;

    fprintf(stderr,"Using AMC13 software ver:%d\n",Version);
    flash = new Flash( m_T2 ) ;
    status = new Status( this ,Version);
    // additional constructor stuff for AMC13 (if needed)

    // now determine the firmware type and features
    try {
      revT1 = read( T1, "STATUS.FIRMWARE_VERS");
      revT2 = read( T2, "STATUS.FIRMWARE_VERS");
      // read register 1 for sanity  check
      t1r1 = read( T1, 1);
      t2r1 = read( T2, 1);
    } catch (uhal::exception::exception & e) {
      amc13::Exception::BadAMC13 e;
      e.Append( "AMC13::initAMC13() - Error reading firmware version from T1 or T2");
      throw e;
    }

    printf("Read firmware versions 0x%x 0x%x\n", revT1, revT2);

    // check for swapped IP or other issues
    if( (t2r1 & 0xffff0000) != 0 ||
	(t1r1 & 0xffff0000) == 0) {
      char tmp[80];
      amc13::Exception::BadAMC13 e;
      e.Append( "AMC13::initAMC13() - possibly incorrect IP addresses?");
      snprintf( tmp, 80, "  T1 R1=%08x  T2 R1=%08x\n", t1r1, t2r1);
      e.Append( tmp);
      throw e;
    }

    features = 0;

    // sanity check the version
    if( revT1 > 0x8800 || 	// new-style version number, not yet supported
	revT1 < 0x200) {	// very old firmware
      features |= AMC13_FEATURE_UNKNOWN;
      return;
    }

    flavor = 0;

    // figure out flavors
    if( revT1 >= 0x200 && revT1 < 0x3ff)
      flavor = 1;
    else if( revT1 >= 0x2200 && revT1 < 0x23ff)
      flavor = 2;
    else if( revT1 >= 0x4000 && revT1 < 0x41ff)
      flavor = 3;
    else if( revT1 >= 0x6000 && revT1 < 0x61ff)
      flavor = 4;
    else if( revT1 >= 0x8000 && revT1 < 0x81ff)
      flavor = 5;
    else if( revT1 >= 0x1000 && revT1 < 0x11ff)
      flavor = 6;

    // now map features
    switch( flavor) {
    case 1:
      features |= AMC13_FEATURE_SLINK_EXPRESS_5G;
      features |= 3 * AMC13_FEATURE_DAQ_LINK_COUNT_MASK_BIT;
      if( revT1 >= 0x24b)
	features |= AMC13_FEATURE_RANDOM_TIMES7;
      break;
    case 2:
      features |= AMC13_FEATURE_SLINK_EXPRESS_10G;
      features |= 3 * AMC13_FEATURE_DAQ_LINK_COUNT_MASK_BIT;
      if( revT1 >= 0x224b)
	features |= AMC13_FEATURE_RANDOM_TIMES7;
      break;
    case 3:
      features |= AMC13_FEATURE_SLINK_EXPRESS_5G;
      features |= 3 * AMC13_FEATURE_DAQ_LINK_COUNT_MASK_BIT;
      features |= AMC13_FEATURE_HCAL_TRIGGER;
      if( revT1 >= 0x4045)
	features |= AMC13_FEATURE_RANDOM_TIMES7;
      break;
    case 4:
      features |= AMC13_FEATURE_SLINK_EXPRESS_10G;
      features |= 3 * AMC13_FEATURE_DAQ_LINK_COUNT_MASK_BIT;
      features |= AMC13_FEATURE_HCAL_TRIGGER;
      if( revT1 >= 0x6045)
	features |= AMC13_FEATURE_RANDOM_TIMES7;
      break;
    case 5:
      features |= AMC13_FEATURE_G2_TCP_10G;
      features |= 3 * AMC13_FEATURE_DAQ_LINK_COUNT_MASK_BIT;
      if( revT1 >= 0x8130)
	features |= AMC13_FEATURE_RANDOM_TIMES7;
      break;
    case 6:
      features |= AMC13_FEATURE_AMC_TEST;
      break;;
    default:
      break;
    }

    printf("flavor = %d  features = 0x%08x\n", flavor, features);

  }

  /// Accessor for Flash interface
  Flash* AMC13::getFlash() {
    return flash ;
  }

  /// Accessor for Flash interface
  Status* AMC13::getStatus() {
    return status ;
  }


  // AMC13 Class Constructor Inherits from AMC13Simple class (see AMC13.hh)
  
  // ***************** control functions *********************
  void AMC13::reset(AMC13Simple::Board chip) {
    writeMask( chip, "ACTION.RESETS.GENERAL");
  }

  void AMC13::resetCounters() {
    writeMask( T1, "ACTION.RESETS.COUNTER");
  }

  void AMC13::resetDAQ() {
    writeMask( T1, "ACTION.RESETS.DAQ");
  }
  
  void AMC13::AMCInputEnable( uint32_t mask ) {
    write(T1, "CONF.AMC.ENABLE_MASK",  ( mask & 0xFFF )); 
    AMC13::m_enabledAMCMask = mask & 0xFFF;
  }
  
  // Function used in old AMCInputEnable to convert list into bit mask
  // (probably should move elsewhere)
  // now handles command-separated values or ranges e.g. "1,2,5-8,12"
  uint32_t AMC13::parseInputEnableList(std::string list,bool slotbased) {
    typedef std::string::size_type string_size;
    string_size i = 0;
    uint32_t n = 0;
    bool in_range = false;
    uint32_t num0 = 0;
    uint32_t num1 = 0;
    unsigned slot_bias = (slotbased)?(1):(0);
    amc13::Exception::UnexpectedRange e;

    while (i != list.size()) {
      // first scan past leading cruft
      while (i != list.size() && !isdigit(list[i]))
	++i;
      string_size j = i;
      // scan to end of number, if any
      while (j != list.size() && isdigit(list[j]))
	++j;
      if (i != j) {		// was there a number?
	num1 = strtoul(list.substr(i, j-i).c_str(), NULL, 0);
	if( !in_range)
	  num0 = num1;
	for( unsigned k=num0; k<=num1; k++) {
	  if( (k-slot_bias) > 11)
	    throw e;
	  n |= (1<<(k-slot_bias));
	}
	in_range = false;
      }
      i = j;
      num0 = num1;
      // check for range indicated by '-'
      if( list[i] == '-') {
	in_range = true;
	++i;
      }
    }
    return n;
  }

  void AMC13::enableLocalL1A( bool ena) {
    write(T1, "CONF.TTC.ENABLE_INTERNAL_L1A", ena);
  }

  void AMC13::configureLocalL1A( bool ena, int mode, uint32_t burst, uint32_t rate, int rules)
  {
    write(T1, "CONF.TTC.ENABLE_INTERNAL_L1A", ena);

    if (mode == 0)
      write(T1, "CONF.LOCAL_TRIG.TYPE", 0x0); // per orbit
    else if (mode == 1)
      write(T1, "CONF.LOCAL_TRIG.TYPE", 0x2); // per bunch crossing
    else if (mode == 2)
      write(T1, "CONF.LOCAL_TRIG.TYPE", 0x3); // random
    else {
      char tmp[80];
      amc13::Exception::UnexpectedRange e;
      snprintf( tmp, 80, "AMC13::configureLocalL1A() - type must be 0-2 (was %d)", mode);
      e.Append( tmp);
      throw e;
    }

    if( burst < 1 || burst > 0x1000) {
      amc13::Exception::UnexpectedRange e;
      e.Append("AMC13::configureLocalL1A() - burst must be in the range 1..4096");
      throw e;
    }
    if( mode == 2) { // ignore burst input and write set to 0 if in random mode
      if (burst != 1) {
	printf( "WARNING: AMC13 random trigger mode, setting burst to 1 L1A per burst\n");
      }
      write( T1, "CONF.LOCAL_TRIG.NUM_TRIG", 0);
    }
    else {
      write( T1, "CONF.LOCAL_TRIG.NUM_TRIG", (burst-1));
    }

    if( mode == 2) { // random mode needs to account for rate factor
      // random mode depends on firmware version
      uint32_t rate_mul = 2;
      uint32_t rate_set;
      if( features & AMC13_FEATURE_RANDOM_TIMES7)  // new mode, rate is times 16
	rate_mul = 16;
      rate_set = rate / rate_mul;
      if( rate_set < 1)
	rate_set = 1;
      if( rate_set > 0xffff)
	rate_set = 0xffff;
      printf("rate_mul = %d  rate_set = %d\n", rate_mul, rate_set);
      if( rate_mul == 2 && rate_set > 0)
	--rate_set;
      write(T1, "CONF.LOCAL_TRIG.RATE", rate_set );
    }

    else {
      if( rate < 1 || rate > 0x10000) {
	amc13::Exception::UnexpectedRange e;
	e.Append("AMC13::configureLocalL1A() - period must be in the range 1..65536");
	throw e;
      }
      write(T1, "CONF.LOCAL_TRIG.RATE", (rate-1));
    }

    if( rules < 0 || rules > 3) {
      amc13::Exception::UnexpectedRange e;
      e.Append("AMC13::configureLocalL1A() - rules must b in the range 0..3");
      throw e;
    }
    write(T1, "CONF.LOCAL_TRIG.RULES", rules);
  }


  void AMC13::sendL1ABurst() {
    writeMask(T1, "ACTION.LOCAL_TRIG.SEND_BURST");
  }
  
  void AMC13::startContinuousL1A() {
    writeMask(T1, "ACTION.LOCAL_TRIG.CONTINUOUS");
  }

  void AMC13::stopContinuousL1A() {
    // check if continuous mode is already on
    uint32_t n = read( T1, "STATUS.LOCAL_TRIG.CONTINUOUS_ON");
    // if so, turn it off (side-effect of SEND_BURST)
    if( n)
      writeMask( T1, "ACTION.LOCAL_TRIG.SEND_BURST");
    // if it isn't on, do nothing
  }
  
  void AMC13::sendLocalEvnOrnReset(uint32_t a, uint32_t b) {
    if (a && !b)
      writeMask(T1, "ACTION.RESETS.EVN"); // Reset EvN
    if (b && !a)
      writeMask(T1, "ACTION.RESETS.ORN"); // Reset OrN
    if (a && b){
      writeMask(T1, "ACTION.RESETS.EVN"); // Reset both
      writeMask(T1, "ACTION.RESETS.ORN"); 
    }
  }

  void AMC13::setOcrCommand( uint32_t cmd, uint32_t mask) {
    if( cmd > 255 || ((cmd & 1) != 0)) {
      char tmp[80];
      amc13::Exception::UnexpectedRange e;
      snprintf( tmp, 80, "AMC13::setOcrCommand() - OCR command must be < 0xff and low bit = 0 (0x%x)\n", cmd);
      e.Append( tmp);
      throw e;
    }
    if( mask > 255 || ((mask & 1) != 0)) {
      char tmp[80];
      amc13::Exception::UnexpectedRange e;
      snprintf( tmp, 80, "AMC13::setOcrCommand() - OCR mask must be < 0xff and low bit = 0 (0x%x)\n", mask);
      e.Append( tmp);
      throw e;
    }
    write( T1, "CONF.TTC.OCR_COMMAND", cmd);
    write( T1, "CONF.TTC.OCR_MASK", mask);
  }

  void AMC13::setOcrCommand( uint32_t cmd) {
    setOcrCommand( cmd, 0);
  }


  void AMC13::setResyncCommand( uint32_t cmd, uint32_t mask) {
    if( cmd > 255 || ((cmd & 1) != 0)) {
      char tmp[80];
      amc13::Exception::UnexpectedRange e;
      snprintf( tmp, 80, "AMC13::setResyncCommand() - resync command must be < 0xff and low bit = 0 (0x%x)\n", cmd);
      e.Append( tmp);
      throw e;
    }
    if( mask > 255 || ((mask & 1) != 0)) {
      char tmp[80];
      amc13::Exception::UnexpectedRange e;
      snprintf( tmp, 80, "AMC13::setResyncCommand() - resync mask must be < 0xff and low bit = 0 (0x%x)\n", mask);
      e.Append( tmp);
      throw e;
    }
    write( T1, "CONF.TTC.RESYNC.COMMAND", cmd);
    write( T1, "CONF.TTC.RESYNC.MASK", mask);
  }

  void AMC13::setResyncCommand( uint32_t cmd) {
    setResyncCommand( cmd, 0);
  }



  void AMC13::setOrbitGap( uint32_t begin, uint32_t end) {
    if( begin > 0xdeb || end > 0xdeb) {
      char tmp[80];
      amc13::Exception::UnexpectedRange e;
      snprintf( tmp, 80, "AMC13::setOrbitGap() - Invalid range (0x%x - 0x%x)\n", begin, end);
      e.Append( tmp);
      throw e;
    }
    write( T1, "CONF.LOCAL_TRIG.GAP_BEGIN", begin);
    write( T1, "CONF.LOCAL_TRIG.GAP_END", end);
  }

  void AMC13::enableAllTTC() {
    write(T2, "CONF.TTC.OVERRIDE_MASK", 0xfff);
  }
  
  void AMC13::daqLinkEnable(bool b) {
    write(T1, "CONF.EVB.ENABLE_DAQLSC", b);    
  }
  
  void AMC13::fakeDataEnable(bool b) {
    write(T1, "CONF.LOCAL_TRIG.FAKE_DATA_ENABLE", b);
  }
  
  void AMC13::localTtcSignalEnable(bool b) {
    write(T1, "CONF.DIAG.FAKE_TTC_ENABLE", b);
    write(T1, "CONF.TTC.ENABLE_INTERNAL_L1A", b);
  }
  
  void AMC13::monBufBackPressEnable(bool b) {
    write(T1, "CONF.EVB.MON_FULL_STOP_EVB", b); 
  } 
  
  void AMC13::configurePrescale( int mode, uint32_t n) {
    amc13::Exception::UnexpectedRange e;
    if( mode < 0 || mode > 1) {
      e.Append( "in configurePrescale, mode must be 0 or 1");
      throw e;
    }
    if( mode) {			// zero-match mode
      if( n < 5 || n > 20) {
	e.Append("in configurePrescale, n must be 5...20 when mode is 1");
	throw e;
      }
      write(T1, "CONF.EVB.SELECT_MASKED_EVN", 20-n);
      write(T1, "CONF.EVB.ENABLE_MASKED_EVN", 1);
    } else {
      if( n > 0x10000) {
	e.Append("in configurePrescale, n must be 1..0x10000 when mode is 0");
	throw e;
      }
      write(T1, "CONF.EVB.ENABLE_MASKED_EVN", 0);
      write(T1, "CONF.EVB.SET_MON_PRESCALE",((n>0)?(n-1):(0)));
    }
  }

  void AMC13::configureBGOShort( int chan, uint8_t cmd, uint16_t bx, uint16_t prescale, bool repeat)
  {
    char tmp[32];
    
    if( chan < 0 || chan > 3) {
      amc13::Exception::UnexpectedRange e;
      e.Append("AMC13::configureBGOShort() - channel must be in range 0 to 3");
      throw e;
    }      

    if( bx > 3563) {
      amc13::Exception::UnexpectedRange e;
      e.Append("AMC13::configureBGOShort() - bx must be in range 0 to 3563");
      throw e;
    }

    snprintf( tmp, sizeof(tmp), "CONF.TTC.BGO%d.%s", chan, "COMMAND");
    write( T1, tmp, cmd);

    snprintf( tmp, sizeof(tmp), "CONF.TTC.BGO%d.%s", chan, "LONG_CMD");
    write( T1, tmp, 0);

    snprintf( tmp, sizeof(tmp), "CONF.TTC.BGO%d.%s", chan, "BX");
    write( T1, tmp, bx);

    snprintf( tmp, sizeof(tmp), "CONF.TTC.BGO%d.%s", chan, "ORBIT_PRESCALE");
    write( T1, tmp, prescale);
    
    snprintf( tmp, sizeof(tmp), "CONF.TTC.BGO%d.%s", chan, "ENABLE_SINGLE");
    if( !repeat)
      write( T1, tmp, 1);
    else
      write( T1, tmp, 0);
  }

  std::vector<uint32_t> AMC13::getBGOConfig( int chan) {
    /*
     *   [0] - repeat enabled (1=yes, 0=no)
     *   [1] - command length (1=long, 0=short)
     *   [2] - command value
     *   [3] - bunch crossing number
     *   [4] - orbit prescale value
     *   [5] - single command (1=single command enabled 0=disabled)
     */
    std::vector<uint32_t> res;
    char tmp[32];
    
    if( chan < 0 || chan > 3) {
      amc13::Exception::UnexpectedRange e;
      e.Append("AMC13::getBGOConfig() - channel must be in range 0 to 3");
      throw e;
    }      

    snprintf( tmp, sizeof(tmp), "CONF.TTC.BGO%d.%s", chan, "ENABLE");
    res.push_back( read( T1, tmp));

    snprintf( tmp, sizeof(tmp), "CONF.TTC.BGO%d.%s", chan, "LONG_CMD");
    res.push_back( read( T1, tmp));

    snprintf( tmp, sizeof(tmp), "CONF.TTC.BGO%d.%s", chan, "COMMAND");
    res.push_back( read( T1, tmp));

    snprintf( tmp, sizeof(tmp), "CONF.TTC.BGO%d.%s", chan, "BX");
    res.push_back( read( T1, tmp));

    snprintf( tmp, sizeof(tmp), "CONF.TTC.BGO%d.%s", chan, "ORBIT_PRESCALE");
    res.push_back( read( T1, tmp));

    snprintf( tmp, sizeof(tmp), "CONF.TTC.BGO%d.%s", chan, "ENABLE_SINGLE");
    res.push_back( read( T1, tmp));
    
    return res;
  }

  void AMC13::configureBGOLong( int chan, uint32_t cmd, uint16_t bx, uint16_t prescale, bool repeat)
  {
    char tmp[32];
    
    if( chan < 0 || chan > 3) {
      amc13::Exception::UnexpectedRange e;
      e.Append("AMC13::configureBGOShort() - channel must be in range 0 to 3");
      throw e;
    }      

    if( bx > 3563) {
      amc13::Exception::UnexpectedRange e;
      e.Append("AMC13::configureBGOShort() - bx must be in range 0 to 3563");
      throw e;
    }

    snprintf( tmp, sizeof(tmp), "CONF.TTC.BGO%d.%s", chan, "COMMAND");
    write( T1, tmp, cmd);

    snprintf( tmp, sizeof(tmp), "CONF.TTC.BGO%d.%s", chan, "LONG_CMD");
    write( T1, tmp, 1);

    snprintf( tmp, sizeof(tmp), "CONF.TTC.BGO%d.%s", chan, "BX");
    write( T1, tmp, bx);

    snprintf( tmp, sizeof(tmp), "CONF.TTC.BGO%d.%s", chan, "ORBIT_PRESCALE");
    write( T1, tmp, prescale);
    
    snprintf( tmp, sizeof(tmp), "CONF.TTC.BGO%d.%s", chan, "ENABLE_SINGLE");
    if( !repeat)
      write( T1, tmp, 1);
    else
      write( T1, tmp, 0);
  }

  void AMC13::enableBGO( int chan) {
    char tmp[32];
    
    if( chan < 0 || chan > 3) {
      amc13::Exception::UnexpectedRange e;
      e.Append("AMC13::enableBGO() - channel must be in range 0 to 3");
      throw e;
    }      

    snprintf( tmp, sizeof(tmp), "CONF.TTC.BGO%d.%s", chan, "ENABLE");
    write( T1, tmp, 1);
  }

  void AMC13::disableBGO( int chan) {
    char tmp[32];
    
    if( chan < 0 || chan > 3) {
      amc13::Exception::UnexpectedRange e;
      e.Append("AMC13::disableBGO() - channel must be in range 0 to 3");
      throw e;
    }      

    snprintf( tmp, sizeof(tmp), "CONF.TTC.BGO%d.%s", chan, "ENABLE");
    write( T1, tmp, 0);
  }

  void AMC13::sendBGO() {
    writeMask(T1, "ACTION.TTC.SINGLE_COMMAND");
  }

  void AMC13::setFEDid(uint32_t id) { // 12-bit mask
    if( id > 0xfff) {
      amc13::Exception::UnexpectedRange e;
      e.Append("AMC13::setFEDid() - FED ID is 12 bits so must be in range 0...4095");
      throw e;
    }      
    write(T1, "CONF.ID.SOURCE_ID", ( id & 0xFFF ) );
  }
  
  void AMC13::setFEDid(int DAQLink, uint32_t id) { // 12-bit mask
    //DAQ link bounds check
    if ( (DAQLink > 2) || (DAQLink < 0) ) {
      amc13::Exception::UnexpectedRange e;
      std::stringstream str;
      str << "AMC13::setFEDid() - DAQLink number "
	  << DAQLink
	  << " is outside of the valid range 0,1,2.";
      e.Append(str.str().c_str());
      throw e;
    }
    //FED ID bounds check
    if( id > 0xfff) {
      amc13::Exception::UnexpectedRange e;
      std::stringstream str;
      str << "AMC13::setFEDid() - FED ID is 12 bits so must be in range 0...4095 (passed 0x"
	  << std::hex << id << std::dec << ").";
      e.Append(str.str().c_str());
      throw e;
    }
    
    switch (DAQLink) {
    case 1:
      write(T1, "CONF.ID.SFP1.SOURCE_ID", ( id & 0xFFF ) );
      break;
    case 2:
      write(T1, "CONF.ID.SFP2.SOURCE_ID", ( id & 0xFFF ) );
      break;
    case 0:
    default:
      write(T1, "CONF.ID.SFP0.SOURCE_ID", ( id & 0xFFF ) );
      break;
    }  
  }  
  
  void AMC13::setSlinkID( uint32_t id) { // 16-bit link id
    if( id > 0xffff || ( (id & 3) != 0)) {
      amc13::Exception::UnexpectedRange e;
      e.Append("AMC13::setSlinkID(): id must be in range 0..0xffff and low 2 bits must be zero");
      throw e;
    }
    write(T1, "CONF.ID.FED_ID", id);
  }

  void AMC13::setBcnOffset( uint32_t offset) { // 12-bit offset
    if( offset > 0xfff) {
      amc13::Exception::UnexpectedRange e;
      e.Append("AMC13::setBcnOffset(): BcN offset must be in range 0..0xfff");
      throw e;
    }
    write(T1, "CONF.BCN_OFFSET", ( offset & 0xFFF) );
  }

  void AMC13::ttsDisableMask( uint32_t mask) { // 12-bit mask
    if( mask > 0xfff) {
      amc13::Exception::UnexpectedRange e;
      e.Append("AMC13::ttsDisableMask(): mask must be in range 0..0xfff");
      throw e;
    }
    write(T1, "CONF.AMC.TTS_DISABLE_MASK", ( mask & 0xFFF) );
  }

  void AMC13::sfpOutputEnable( uint32_t mask) { // 3-bit mask
    write(T1, "CONF.SFP.ENABLE_MASK", ( mask & 0x7 ) );
  }
  
  void AMC13::startRun() {
    // Enable run bit
    write(T1, "CONF.RUN", 1);
    usleep(2000);
    // Reset Virtex Chip
    writeMask(T1, "ACTION.RESETS.GENERAL");
  }
  
  void AMC13::endRun() {
    // Disable run bit
    write(T1, "CONF.RUN", 0);
  }


  
//
// read one (possibly segmented) event from AMC13
// return pointer to malloc'd data
// set nwords to length in 64-bit words
// 
// set rc as follows:
//   0 - all OK
//   1 - buffer empty (0xd == 0)
//   2 - invalid size (< 0x10 or > 0x20000)
//   3 - malformed event header
//   4 - malloc failed (nwords set to attempted size)
//
// if rc != 0 then NULL is returned
//
  uint64_t* AMC13::readEvent( size_t& nwords, int& rc) {

    uint32_t enabledDAQs = read( T1, "CONF.SFP.ENABLE_MASK" );

    if ( enabledDAQs > 1 ) {
      amc13::Exception::UnexpectedRange e;
      char tmp[60];
      snprintf( tmp, 60, "AMC13::readEvent(size_t,int) - unexpected number of DAQs enabled - %u, consider readEventMultiFED()\n", enabledDAQs );
      e.Append( tmp );
      throw e;
    }

    int debug = 0;

    uint64_t *p = NULL;			 // NULL if data not valid
    rc = 0;
    uint32_t BUFF;
    
    if( debug) {
      printf("readEvent()\n");
    }

    BUFF = getT1()->getNode("MONITOR_BUFFER_RAM").getAddress();
    
    // check size
    int mb_siz = read( T1, "STATUS.MONITOR_BUFFER.WORDS_SFP0");
    if( debug) printf("readEvent() initial size=0x%x\n", mb_siz);
    
    if( !mb_siz) {
      rc = 1;
      return p;
    }
    
    if( mb_siz < 0x10 || mb_siz > 0x20000) {
      rc = 2;
      return p;
    }
    
    // read the event header and unpack
    uint64_t h[16];
    read( T1, BUFF, 4, (uint32_t*)h);
    int namc = ( h[1]>>52) & 0xf;
    int evn = ( h[0]>>32) & 0xffffff;
    int bcn = ( h[0]>>20) & 0xfff;
    int fov = ( h[1]>>60) & 0xf;
    uint32_t orn = (h[1] >> 4) & 0xffffffff;
    // check format
    if( ((h[0] >> 60) & 0xf) != 5 || fov != 1)
      printf("Header looks funny but trying to proceed: %016"PRIx64"\n", h[0]);
    
    // print header info for debug
    if( debug)
      printf( "EvN: %06x BcN: %03x OrN: %08x  namc: %d\n", evn, bcn, orn, namc);
    if( namc > AMC13SIMPLE_NAMC || namc == 0) {
      printf("AMC count bad\n");
      rc = 3;
      return p;
    }
    
    // read AMC headers
    read( T1, BUFF+4, namc*2, (uint32_t *)(&h[2]));
    int amc_siz[AMC13SIMPLE_NAMC];
    int nblock[AMC13SIMPLE_NAMC];
    int tsiz = 0;
    int tblk = 0;
    int nblock_max = 0;
    
    if( debug)
      printf( "nn: -LMSEPVC -Size- Blk Ident\n");
    for( int i=0; i<namc; i++) {
      uint64_t ah = h[2+i];
      int lmsepvc = (ah>>56) & 0xff;
      amc_siz[i] = (ah>>32) & 0xffffff;
      tsiz += amc_siz[i];
      int no = (ah>>16) & 0xf;
      int blk = (ah>>20) & 0xff;
      // calculate block size
      if( amc_siz[i] <= 0x13fe)
	nblock[i] = 1;
      else
	nblock[i] = (amc_siz[i]-1023)/4096+1; // Wu's formula, fixed
      
      if( debug)
	printf("Calculated block count %d from size %d using (siz-5118)/4096+1\n",
	       nblock[i], amc_siz[i]);
      tblk += nblock[i];
      if( nblock[i] > nblock_max)	// keep track of AMC with the most blocks
	nblock_max = nblock[i];
      int bid = ah & 0xffff;
      if( debug)
	printf( "%2d: %s %06x %02x  %04x\n", no, AMC13_fmt_bits( lmsepvc, 8), amc_siz[i], blk, bid);
    }
    
    // calculate total event size Wu's way
    nwords = tsiz + tblk + nblock_max*2 + 2;
    
    if( debug) {
      printf("Calculated size 0x%llx by Wu formula:\n", (unsigned long long)nwords);
      printf("Nwords = sum(size) + sum(blocks) + NblockMax*2 + 2\n");
      printf("            %5d          %5d     %5d\n", tsiz, tblk, nblock_max);
      printf("Montor buffer has N(32)=0x%x N(64)=0x%x words\n", mb_siz, mb_siz/2);
    }
    
    if( (p = (uint64_t*)calloc( nwords, sizeof(uint64_t))) == NULL) {
      rc = 4;
      return p;
    }
    
    int words_left = nwords;
    uint64_t* tp = p;
    
    while( words_left) {
      
      if( debug) printf("0x%x words left reading to offset 0x%lx\n", words_left, tp-p);
      
      if( mb_siz % 2) {
	if( debug) printf("ERROR: mb_siz=0x%x is odd\n", mb_siz);
	rc = 3;
	return p;
      }
      
      if( (mb_siz/2) > words_left || mb_siz == 0) { // check buffer size
	if( debug) printf("ERROR: mb_siz = 0x%x  words_left = 0x%x\n", mb_siz, words_left);
	rc = 3;
	return p;
      }
      
      // have to break up big reads
      read( T1, BUFF, mb_siz, (uint32_t *)tp);
      
      if( debug) {
	for( int i=0; i<8; i++)
	  printf("  %08lx: %016"PRIx64"\n", (tp-p+i), tp[i]);
	printf( "  ...\n");
	for( int i=(mb_siz/2)-8; i<(mb_siz/2); i++)
	  printf("  %08lx: %016"PRIx64"\n", (tp-p+i), tp[i]);	
      }
      words_left -= mb_siz/2;
      tp += mb_siz/2;
      write( T1, "ACTION.MONITOR_BUFFER.NEXT_PAGE", 0); // advance to next page
      mb_siz = read( T1, "STATUS.MONITOR_BUFFER.WORDS_SFP0");
    }
    
    rc = 0;
    return p;

  }


  const uint32_t AMC13::MONITOR_BUFFER_RAM_SFP[4][3] = { {0,0,0}, {0x20000,0,0}, {0x20000,0x30000,0}, {0x20000,0x2a000,0x34000} }; // accessed with MONITOR_BUFFER_RAM_SFP[numDAQs][iDAQ]
  const std::string AMC13::STATUS_MONITOR_BUFFER_WORDS_SFP[3] = {"STATUS.MONITOR_BUFFER.WORDS_SFP0","STATUS.MONITOR_BUFFER.WORDS_SFP1","STATUS.MONITOR_BUFFER.WORDS_SFP2"};

  const std::vector< std::vector<uint64_t> > & AMC13::MonitorBufferEventData() {
    return MonitorBufferData;
  }
  
  uint32_t AMC13::EventSizer( size_t iDAQ, size_t numDAQs, uint32_t& maxBlocks ) {
    // Calculates the number of words in an event

    int debug = 0;

    uint64_t* headers = new uint64_t[2+12/numDAQs];
    read( T1, MONITOR_BUFFER_RAM_SFP[numDAQs][iDAQ], 4+24/numDAQs, (uint32_t*)headers ); // reads all potential header words, combining the 32-bit words read into 64-bit words
                                                                                         // may read more than necessary if namc is <12/numDAQs, but then the extra words are ignored

    uint32_t nAMC  = ( headers[1]>>52 ) & 0xf; // number of AMCs involved in event
    uint32_t fov   = ( headers[1]>>60 ) & 0xf; // should be 1 if everything is ok
    uint32_t mNumb = ( headers[0]>>60 ) & 0xf; // should be 5 is everything is ok
    // uint32_t bcn   = ( headers[0]>>20 ) & 0xfff;
    // uint32_t orn = ( headers[1] >> 4 ) & 0xffffffff;

    if ( debug ) {
      printf( "First two 64-bit words: %016lx, %016lx\n", headers[0], headers[1] );
      printf( "nAMC = %u\n", nAMC );
    }

    if ( mNumb != 5 || fov != 1 ) {
      char tmp[80];
      amc13::Exception::UnexpectedRange e;
      snprintf( tmp, 80, "AMC13::EventSizer() Header looks funny: %016"PRIx64"\n", headers[0] );
      e.Append( tmp );
      throw e;
    }

    if( nAMC > AMC13SIMPLE_NAMC || nAMC == 0 ) {
      char tmp[80];
      amc13::Exception::UnexpectedRange e;
      snprintf( tmp, 80, "AMC13::EventSizer() AMC count bad: %u (should be 1..12)\n", nAMC );
      e.Append( tmp );
      throw e;
    }
    
    uint32_t amcSize;
    uint32_t nBlocks;
    uint32_t tSize = 0;
    uint32_t tBlocks = 0;
    uint32_t nBlock_max = 0;

    //int lmsepvc, amc_no, blk_no, brd_id; //unnecessary for calculating size of event

    for ( uint32_t iAMC = 0; iAMC < nAMC; ++iAMC ) {
    
      amcSize = ( headers[2+iAMC] >> 32 ) & 0xffffff;
      tSize += amcSize;

      // lmsepvc = ( headers[2+iAMC] >> 56 ) & 0xff;
      // amc_no = ( headers[2+iAMC] >> 16 ) & 0xf;
      // blk_no = ( headers[2+iAMC] >> 20 ) & 0xff;
      // brd_id = headers[2+iAMC] & 0xffff;
    
      if ( amcSize <= 0x13fb ) {
        nBlocks = 1;
      }
      else {
        nBlocks = ( amcSize - 1023 )/4096 + 1; // Wu's formula
      }
      tBlocks += nBlocks;
      if ( nBlocks > nBlock_max ) { nBlock_max = nBlocks; }

      if ( debug ) {
	printf( "AMC%u: word=%016lx, amc_size = %u, nBlocks = %u\n", iAMC+1, headers[2+iAMC], amcSize, nBlocks );
      }

    }

    if ( nBlock_max > maxBlocks ) { maxBlocks = nBlock_max; }
    
    if ( debug ) { 
      printf( "tSize=%u, tBlocks=%u, nBlock_max=%u\n", tSize, tBlocks, nBlock_max );
      printf( "tSize     + tBlocks   + nBlock_max*2 + 2 = %u\n", tSize+tBlocks+(nBlock_max*2)+2 );
    }
    
    delete[] headers;
    return tSize + tBlocks + (nBlock_max*2) + 2;

  }

  uint32_t AMC13::BlockSizer( size_t iDAQ, size_t numDAQs, uint32_t block_num ) {
    // Returns number of 64-bit words in current block of event

    if ( !read(T1, STATUS_MONITOR_BUFFER_WORDS_SFP[iDAQ]) ) {
      // check to see if block is empty, i.e. other channels take up
      // more blocks than this channel.
      return 0;
    }

    uint64_t* blockHeads = new uint64_t[2+12/numDAQs];
    read( T1, MONITOR_BUFFER_RAM_SFP[numDAQs][iDAQ], 2*(2+12/numDAQs), (uint32_t*)blockHeads );

    uint32_t blockSize = 0;

    uint32_t namc;
    uint32_t MS;
    if ( block_num == 0 ) {
      namc = (blockHeads[1] >> 52) & 0xf;
      blockSize += 1;
    }
    else {
      namc = (blockHeads[0] >> 52) & 0xf;
    }

    blockSize += 2 + namc;

    uint32_t netM = 0;
    
    for (uint32_t iAMC = 0; iAMC < namc; ++iAMC ) {
      
      MS = ( blockHeads[ ( (block_num == 0) ? 2 : 1 ) + iAMC ] >> 60 ) & 0x2;

      netM += (MS>>1) & 0x1;

      switch (MS) {
      case 0:
      case 1:
      case 3:
	blockSize += ( blockHeads[ ( (block_num == 0 ) ? 2 : 1 ) + iAMC ] >> 32 ) & 0xffffff;
	break;
      case 2:
	blockSize += 0x1000;
	break;
      default:
	amc13::Exception::UnexpectedRange e;
	e.Append("Bad MS in BlockSizer\n");
	throw e;
      }
    
    }

    if ( netM == 0 ) {
      blockSize += 1;
    }

    delete[] blockHeads;

    return blockSize;

  }

  void AMC13::MultiFEDBlockReader( uint32_t* nWords, uint32_t& maxBlocks, size_t numDAQs ) {

    int debug = 0;

    // uint32_t mbSize;

    uint32_t wordsRead[3];
    uint32_t payloadWords;

    uint32_t blocksRead = 0;
    
    for ( size_t iDAQ = 0; iDAQ < numDAQs; ++iDAQ ) {
      wordsRead[iDAQ] = 0;
    }

    uhal::ValVector<uint32_t> tempVec;

    size_t readCheck = 0;

    if ( debug ) { printf( "Starting read... \n" ); }

    for ( uint32_t iBlock = 0; iBlock < maxBlocks; ++iBlock ) {

      for ( size_t iDAQ = 0; iDAQ < numDAQs; ++iDAQ ) {


	payloadWords = BlockSizer( iDAQ, numDAQs, iBlock );

	// mbSize = read( T1, STATUS_MONITOR_BUFFER_WORDS_SFP[iDAQ] );
	// printf("SFP%lu     mbSize: %u   BlockSizer: %u\n", iDAQ, mbSize, 2*payloadWords);

	// if event size is too large, throw exception
	if ( payloadWords > 0x20000 )  {
	  char tmp[80];
	  amc13::Exception::UnexpectedRange e;
	  snprintf( tmp, 80, "AMC13::MultiFEDBlockReader() - unexpected event size 0x%x\n", payloadWords );
	  e.Append( tmp );
	  throw e;
	}

	// if number of 32-bit words is odd, throw exception
	// if ( mbSize % 2 ) {
	//   char tmp[90];
	//   amc13::Exception::UnexpectedRange e;
	//   snprintf( tmp, 90, "AMC13::MultiFEDBlockReader() 32-bit word count in buffer is odd: 0x%x\n", mbSize );
	//   e.Append( tmp );
	//   throw e;
	// }
	
	tempVec = getT1()->getClient().readBlock( MONITOR_BUFFER_RAM_SFP[numDAQs][iDAQ], 2*payloadWords, uhal::defs::INCREMENTAL );
	getT1()->getClient().dispatch();

	for( uint32_t wordCnt = 0; wordCnt < payloadWords; ++wordCnt ) {
	  MonitorBufferData[iDAQ].push_back( tempVec[wordCnt*2] | ((uint64_t)tempVec[wordCnt*2+1] << 32) );
	}

	wordsRead[iDAQ] += payloadWords;
	readCheck += 1;
		
      }

      if ( readCheck == numDAQs ) {
	readCheck = 0;
	write( T1, "ACTION.MONITOR_BUFFER.NEXT_PAGE", 0 );
	++blocksRead;
	if ( debug ) { printf( "Next page in memory...\n" ); }
      }
      else {
	// somehow missed an SFP channel. will probably never occur.
	if ( debug ) { printf( "Error reading one of the DAQ channels. Exiting read.\n" ); }
	char tmp[120];
	amc13::Exception::UnexpectedRange e;
	snprintf( tmp, 120, "AMC13::MultiFEDBlockReader() - an SFP channel was unread\n" );
	e.Append( tmp );
	throw e;
      }

      if ( debug ) { printf("%u blocks remain to be read\n", maxBlocks); }

    }

    /// Done reading, now check that it read correctly ///
    for ( size_t jDAQ = 0; jDAQ < numDAQs; ++jDAQ ) {
      if ( debug ) {
	printf("***SFP%lu: %u read, %u exp\n", jDAQ, wordsRead[jDAQ], nWords[jDAQ]);
	printf("  *Last word: %016lx\n", MonitorBufferData[jDAQ].back() );
      }
      if ( wordsRead[jDAQ] != nWords[jDAQ] ) {
      	char tmp[120];
      	amc13::Exception::UnexpectedRange e;
      	snprintf( tmp, 120, "AMC13::MultiFEDBlockReader() - did not read number of words expected\n" \
      		  "in SFP%lu  -  expected: %u,  read: %u\n", jDAQ, nWords[jDAQ], wordsRead[jDAQ]);
      	e.Append( tmp );
      	throw e; 
      }
      if ( ((MonitorBufferData[jDAQ].back()>>32)&0xffffff) != wordsRead[jDAQ] ) {
	char tmp[120];
	amc13::Exception::UnexpectedRange e;
	snprintf( tmp, 120, "AMC13::MultiFEDBlockReader() - words read doesn't match trailer - read: %u, trailer: %lu\n", \
		  wordsRead[jDAQ], (MonitorBufferData[jDAQ].back()>>32)&0xffffff );
	e.Append( tmp );
	throw e;
      }
    }

    return;
    
  }

  void AMC13::ReadEventMultiFED() {
    // This method is built to read one event from potentially multiple DAQ channels in its entirety.
    // It acts on a private AMC13 vector< vector<uint64_t> >
    // Having the container be a class member should result in fewer memory reallocations / copies.
    // Upon every read, the old data is cleared, but the capacity remains, so once a few
    // large events have been read, there should always be enough reserved space and no reallocations.

    int debug = 0;

    // CONF.SFP.ENABLE_MASK = 0b000=0 for 0 DAQs, 0b001=1 for 1 DAQ, 0b011=3 for 2 DAQs, 0b111=7 for 3 DAQs
    size_t numDAQs;
    switch ( read(T1, "CONF.SFP.ENABLE_MASK") ) {
    case 0: // 0 DAQ case treated like 1 DAQ case
    case 1:
      numDAQs = 1;
      break;
    case 3:
      numDAQs = 2;
      break;
    case 7:
      numDAQs = 3;
      break;
    default:
      char tmp[120];
      amc13::Exception::UnexpectedRange e;
      snprintf( tmp, 120, "AMC13::readeventMultFED() - CONF.SFP.ENABLE_MASK (%d) has value other than 0,1,3,7\n", read(T1, "CONF.SFP.ENABLE_MASK") );
      e.Append( tmp );
      throw e;
      break;
    }

    // Ensure the vector of vectors is the right size and that it is empty of data
    if ( MonitorBufferData.size() != numDAQs ) {
      // this block will only execute on the first read after the number of DAQs has been changed
      for ( unsigned int subVec = 0; subVec < MonitorBufferData.size(); ++subVec ) {
	MonitorBufferData.at(subVec).resize(0); // erases subvector content and frees used memory
      }
      MonitorBufferData.resize(numDAQs); // resize to appropriate number of DAQs
    }
    else {
      for ( unsigned int subVec = 0; subVec < MonitorBufferData.size(); ++subVec ) {
	MonitorBufferData.at(subVec).clear(); // get rid of old content, but leave capacity untouched -> fewer reallocations
      }
    }

    if ( debug ) { 
      printf( "numDAQs = %lu\n", numDAQs ); 
    }
    
    bool emptyCheck = 1;
    for ( size_t iDAQ = 0; iDAQ < numDAQs; ++iDAQ ) {
      if ( read( T1, STATUS_MONITOR_BUFFER_WORDS_SFP[iDAQ] ) ) {
	emptyCheck = 0; // any non-empty channel will cause empty check to fail
      }
    }

    if ( emptyCheck ) {
      if ( debug ) { printf( "Nothing to read from buffer. Exiting read.\n" ); }
      return;
    } 

    uint32_t nWordsEventFragment[3];
    uint32_t maxBlocks = 0;
    for ( size_t iDAQ = 0; iDAQ < numDAQs; ++iDAQ ) {
      if ( debug ) { printf( "Getting event size from channel SFP%lu\n", iDAQ ); }
      nWordsEventFragment[iDAQ] = EventSizer( iDAQ, numDAQs, maxBlocks );
      MonitorBufferData.at(iDAQ).reserve( nWordsEventFragment[iDAQ] ); // will reallocate if nWordsEvFrag[iDAQ] is greater than current capacity, otherwise do nothing
    }

    MultiFEDBlockReader( nWordsEventFragment, maxBlocks, numDAQs );
    
    return;

  }
  
  //
  // read one (possibly segmented) event from AMC13
  // return a vector of 64-bit words
  //
  std::vector<uint64_t> AMC13::readEvent() {

    uint32_t enabledDAQs = read( T1, "CONF.SFP.ENABLE_MASK" );

    if ( enabledDAQs > 1 ) {
      amc13::Exception::UnexpectedRange e;
      char tmp[60];
      snprintf( tmp, 60, "AMC13::readEvent() - unexpected number of DAQs enabled - %u\n", enabledDAQs );
      e.Append( tmp );
      throw e;
    }

    int debug = 0;

    std::vector<uint64_t> datVec;

    size_t nwords;

    uint32_t BUFF;
    uint32_t mBUFF;

    if( debug) {
      printf("readEvent()\n");
    }

    BUFF = getT1()->getNode("MONITOR_BUFFER_RAM").getAddress();
    
    if( debug) {
      printf("readEvent()\n");
      printf(" BUFF=0x%08x  mBUFF=0x%08x\n", BUFF, mBUFF);
    }
    
    // check size
    int mb_siz = read( T1, "STATUS.MONITOR_BUFFER.WORDS_SFP0");
    if( debug) printf("readEvent() initial size=0x%x\n", mb_siz);
    
    if( !mb_siz)		// no data?
      return datVec;		// return empty vector
    
    if( mb_siz < 0x10 || mb_siz > 0x20000) {
      char tmp[80];
      amc13::Exception::UnexpectedRange e;
      snprintf( tmp, 80, "AMC13::readEvent() - unexpected event size 0x%x\n", mb_siz);
      e.Append( tmp);
      throw e;
    }
    
    // read the event header and unpack
    uint64_t h[16];
    read( T1, BUFF, 4, (uint32_t*)h);
    int namc = ( h[1]>>52) & 0xf;
    int evn = ( h[0]>>32) & 0xffffff;
    int bcn = ( h[0]>>20) & 0xfff;
    int fov = ( h[1]>>60) & 0xf;
    uint32_t orn = (h[1] >> 4) & 0xffffffff;
    // check format
    if( ((h[0] >> 60) & 0xf) != 5 || fov != 1) {
      char tmp[80];
      amc13::Exception::UnexpectedRange e;
      snprintf( tmp, 80, "AMC13::readEvent() Header looks funny: %016"PRIx64"\n", h[0]);
      e.Append( tmp);
      throw e;
    }
    
    // print header info for debug
    if( debug)
      printf( "EvN: %06x BcN: %03x OrN: %08x  namc: %d\n", evn, bcn, orn, namc);
    if( namc > AMC13SIMPLE_NAMC || namc == 0) {
      char tmp[80];
      amc13::Exception::UnexpectedRange e;
      snprintf( tmp, 80, "AMC13::readEvent() AMC count bad: %d (should be 1..12)\n", namc);
      e.Append( tmp);
      throw e;
    }
    
    // read AMC headers
    read( T1, BUFF+4, namc*2, (uint32_t *)(&h[2]));
    int amc_siz[AMC13SIMPLE_NAMC];
    int nblock[AMC13SIMPLE_NAMC];
    int tsiz = 0;
    int tblk = 0;
    int nblock_max = 0;
    
    if( debug)
      printf( "nn: -LMSEPVC -Size- Blk Ident\n");
    for( int i=0; i<namc; i++) {
      uint64_t ah = h[2+i];
      int lmsepvc = (ah>>56) & 0xff;
      amc_siz[i] = (ah>>32) & 0xffffff;
      tsiz += amc_siz[i];
      int no = (ah>>16) & 0xf;
      int blk = (ah>>20) & 0xff;
      // calculate block size
      if( amc_siz[i] <= 0x13fe)
	nblock[i] = 1;
      else
	nblock[i] = (amc_siz[i]-1023)/4096+1; // Wu's formula, fixed
      
      if( debug)
	printf("Calculated block count %d from size %d using (siz-5118)/4096+1\n",
	       nblock[i], amc_siz[i]);
      tblk += nblock[i];
      if( nblock[i] > nblock_max)	// keep track of AMC with the most blocks
	nblock_max = nblock[i];
      int bid = ah & 0xffff;
      if( debug)
	printf( "%2d: %s %06x %02x  %04x\n", no, AMC13_fmt_bits( lmsepvc, 8), amc_siz[i], blk, bid);
    }
    
    // calculate total event size Wu's way
    nwords = tsiz + tblk + nblock_max*2 + 2;
    
    if( debug) {
      printf("Calculated size N(64)=0x%llx by Wu formula:\n", (unsigned long long)nwords);
      printf("Nwords = sum(size) + sum(blocks) + NblockMax*2 + 2\n");
      printf("            %5d          %5d     %5d\n", tsiz, tblk, nblock_max);
      printf("Montor buffer has N(32)=0x%x N(64)=0x%x words\n", mb_siz, mb_siz/2);
    }
    
    int words_left = nwords;
    uint32_t offset = 0 ;

    while( words_left) {
      
      if( mb_siz % 2) {
	char tmp[80];
	amc13::Exception::UnexpectedRange e;
	snprintf( tmp, 80, "AMC13::readEvent() word count in buffer is odd: 0x%x\n", mb_siz);
	e.Append( tmp);
	throw e;
      }
      
      if( (mb_siz/2) > words_left || mb_siz == 0) { // check buffer size
	char tmp[80];
	amc13::Exception::UnexpectedRange e;
	snprintf( tmp, 80, "AMC13::readEvent() word count in buffer (0x%x) > calculated words left (0x%x)\n",
		  mb_siz, words_left);
	e.Append( tmp);
	throw e;
      }
      
      uint32_t addr = BUFF;
      uhal::ValVector<uint32_t> retVec;

      if( debug)
	printf("****  Reading N(32)=0x%x words...\n", mb_siz);

      retVec = getT1() -> getClient(). readBlock( ( addr+offset ), mb_siz, uhal::defs::INCREMENTAL );
      getT1()-> getClient(). dispatch() ;
      // std::copy( retVec.begin(), retVec.end(), std::back_inserter( datVec ) ) ;
      for( int k=0; k<mb_siz/2; k++)
	datVec.push_back( retVec[k*2] | ((uint64_t)retVec[k*2+1] << 32));

      if( debug) {
	// print first 8 and last 8 words for debug
	for( int i=0; i<8; i++)
	  printf("  %08x: %016"PRIx32"\n", i, retVec[i]);
	printf( "  ...\n");
	for( int i=(mb_siz)-8; i<(mb_siz); i++)
	  printf("  %08x: %016"PRIx32"\n", i, retVec[i]);
      }
      words_left -= mb_siz/2;
      if( debug)
	printf("**** Finished reading N(32)=0x%x words left\n", words_left);

      write( T1, "ACTION.MONITOR_BUFFER.NEXT_PAGE", 0); // advance to next page
      mb_siz = read( T1, "STATUS.MONITOR_BUFFER.WORDS_SFP0");
    }
    
    return datVec;

  }

  uint16_t AMC13::GetEnabledAMCMask( bool readFromBoard ) {
    if (readFromBoard){
      uint16_t mask = read( T1, "CONF.AMC.ENABLE_MASK" );
      m_enabledAMCMask = mask ;
    }	
    return m_enabledAMCMask;
  }

  void AMC13::setTTCHistoryEna( bool enaHist) {
    write( T2, "CONF.TTC_HISTORY.ENABLE", enaHist);
  }

  void AMC13::setTTCFilterEna( bool ena) {
    write( T2, "CONF.TTC_HISTORY.FILTER", ena);
  }

  void AMC13::setTTCHistoryFilter( int n, uint32_t filterVal) {
    if( n < 0 || n > 15) {
      amc13::Exception::UnexpectedRange e;
      e.Append( "TTC history filter number must be in range 0-15");
      throw e;
    }
    uint32_t adr = getT2()->getNode("CONF.TTC_HISTORY.FILTER_LIST").getAddress() + n;
    write( T2, adr, filterVal);
  }

  uint32_t AMC13::getTTCHistoryFilter( int n) {
    if( n < 0 || n > 15) {
      amc13::Exception::UnexpectedRange e;
      e.Append( "TTC history filter number must be in range 0-15");
      throw e;
    }
    uint32_t adr = getT2()->getNode("CONF.TTC_HISTORY.FILTER_LIST").getAddress() + n;
    return( read( T2, adr));
  }

  void AMC13::clearTTCHistoryFilter() {
    writeMask( T2, "ACTION.RESETS.TTC_FILTER_LIST");
  }

  void AMC13::clearTTCHistory() {
    writeMask( T2, "ACTION.RESETS.TTC_COMMAND_HISTORY");
  }

  int AMC13::getTTCHistoryCount() {
    if( read( T2, "STATUS.TTC_HISTORY.FULL"))
      return( 512);
    return( read( T2, "STATUS.TTC_HISTORY.COUNT"));
  }

  // calculate address for specified history item, where
  // 0 is the most recent, -1 is the previous, etc
  uint32_t AMC13::getTTCHistoryItemAddress( int item) {
    if( item > -1 || item < -512) {
      amc13::Exception::UnexpectedRange e;
      e.Append( "TTC history item offset out of range");
      throw e;
    }
    uint32_t base = getT2()->getNode("STATUS.TTC_HISTORY.BUFFER.BASE").getAddress();
    uint32_t wp = read( T2, "STATUS.TTC_HISTORY.COUNT");
    uint32_t a = base + 4*(wp+item);		// write pointer adjusted
    if( a < base)
      a += 0x800;
    return a;
  }


  void AMC13::getTTCHistory( uint32_t* buffer, int nreq) {
    write( T2, "CONF.TTC_HISTORY.ENABLE", 0); // disable history capture
    uint32_t base = getT2()->getNode("STATUS.TTC_HISTORY.BUFFER.BASE").getAddress();
    int nhist = getTTCHistoryCount();	      // get current count
    if( nreq < 0 || nreq > nhist) {	      // check range
      amc13::Exception::UnexpectedRange e;
      e.Append( "TTC history filter request count out of range");
      throw e;
    }
    uint32_t adr = getTTCHistoryItemAddress( -nreq);
    uint32_t* p = buffer;
    for( int i=0; i<nreq; i++) {
      for( int k=0; k<4; k++)
	p[k] = read( T2, adr+k);
      adr = base + ((adr + 4) % 0x800);
      p += 4;
    }
  }

  // vector version for the pointer-challenged
  std::vector<uint32_t> AMC13::getTTCHistory( int nreq) {
    std::vector<uint32_t> hist;
    write( T2, "CONF.TTC_HISTORY.ENABLE", 0); // disable history capture
    uint32_t base = getT2()->getNode("STATUS.TTC_HISTORY.BUFFER.BASE").getAddress();
    int nhist = getTTCHistoryCount();	      // get current count
    if( nreq < 0 || nreq > nhist) {	      // check range
      amc13::Exception::UnexpectedRange e;
      e.Append( "TTC history filter request count out of range");
      throw e;
    }
    uint32_t adr = getTTCHistoryItemAddress( -nreq);
    for( int i=0; i<nreq; i++) {
      for( int k=0; k<4; k++)
	hist.push_back( read( T2, adr+k));
      adr = base + ((adr + 4) % 0x800);
    }
    return hist;
  }

  std::vector<uint32_t> AMC13::getL1AHistory( int nhist)
  {
    int ngot = 0;
    uint32_t l1a[4];
    std::vector<uint32_t> hist;
    uint32_t base = getT1()->getNode("STATUS.TTC.L1A_HISTORY").getAddress();
    while( ngot < nhist) {
      for( int i=0; i<128; i++) {
	for( int k=0; k<4; k++)
	  l1a[k] = read( T1, base+k);
	base += 4;
	if( l1a[2] != 0) {
	  for( int k=0; k<4; k++)
	    hist.push_back( l1a[k]);
	  ++ngot;
	}
      }
    }
    return hist;
  }
    


  //=======================================
  //===    Calibration Trigger Methods  ===
  //=======================================
  void AMC13::calTrigEnable(bool en) {
    int i;
    if (en) {
      i = 1;
    } else {
      i = 0;
    }
    write(T1,"CONF.CAL_ENABLE", i);
  }

  void AMC13::setCalTrigWindow(uint16_t lo, uint16_t hi) {
    amc13::Exception::UnexpectedRange e;
    bool shouldThrow = true;
    if (lo > 0xdbf || lo < 0xd80) {
      e.Append("Lower Calibration Window limit must be between 3456 (0xd80) and 3519 (0xdbf)");
    } else if (hi > 0xdbf || hi < 0xd80) {
      e.Append("Upper Calibration Window limit must be between 3456 (0xd80) and 3519 (0xdbf)");
    } else if ( hi <= lo) {
      e.Append("Upper Calibration Window limit must be greater than Lower Calibration Limit");
    } else {
      shouldThrow = false;
    }
    if (shouldThrow) {
      throw e;
    } else {
      uint8_t hi_prog = hi & 0b111111;
      uint8_t lo_prog = lo & 0b111111;
      write(T1,"CONF.CAL_WINDOW_UPPER_PROG",hi_prog);
      write(T1,"CONF.CAL_WINDOW_LOWER_PROG",lo_prog);
    }
  }
    
  bool AMC13::getCalTrigEnable() {
    return read(T1,"CONF.CAL_ENABLE") == 1;
  }

  uint16_t AMC13::getCalTrigWindowHigh() {
    uint16_t val = read(T1,"CONF.CAL_WINDOW_UPPER");
    return val;
  }

  uint16_t AMC13::getCalTrigWindowLow() {
    uint16_t val = read(T1,"CONF.CAL_WINDOW_LOWER");
    return val;
  }


}
