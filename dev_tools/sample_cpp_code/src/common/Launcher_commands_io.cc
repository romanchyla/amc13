#include "Launcher.hh"

//
// read a chip from a command (t1 or t2)
// one argument may be: address, single name or regular expression
// second argument is count to read from each address
// last argument may be "D" for doublewords (64-bits)
//
int Launcher::Read( std::vector<std::string> strArg,std::vector<uint64_t> intArg) {

  std::vector<std::string> nodes;
    
  bool numericAddr = true;
  uint32_t addr = 0;
  uint32_t val;
  uint64_t vald;
  int count = 1;
    
  std::string flags("");
    
  // sort out arguments
  switch( strArg.size()) {
  case 0:			// no arguments is an error
    printf("Need at least an address\n");
    return 0;
  case 3:			// third is flags
    flags = strArg[2];
    // fall through to others
  case 2:			// second is either count or flags
    if( isdigit( strArg[1].c_str()[0]))
      count = intArg[1];
    else
      flags = strArg[1];
    // fall through to address
  case 1:			// one must be an address
    numericAddr = isdigit( strArg[0].c_str()[0]);
    addr = uint32_t( intArg[0]);
    break;
  default:
    printf("Too many arguments after command\n");
    return 0;
  }
   
  uhal::ValWord<uint32_t> vw; //valword for transaction
  // count specified?
  // if so, read whole words only
  if( count > 1) {
      
    if( !numericAddr) {		// reg name, first make uppercase
      std::string saddr = strArg[0];
      std::transform( saddr.begin(), saddr.end(), saddr.begin(), ::toupper);
      // try to look up reg name, complain if not found
      try {
	addr = hw->getNode( saddr).getAddress();
      } catch( const uhal::exception::NoBranchFoundWithGivenUID ) {
	printf("Register not found: %s\n", saddr.c_str());
      }
    }
      
    // print multi-value, 32- or 64-bit
    bool b64 = (flags == "d" || flags == "D");
    int wpl = b64 ? 4 : 8;
    int inc = b64 ? 2 : 1;
      
    for( int i=0; i<count; i++) {
      vw = hw->getClient().read(  addr+i*inc); // start the transaction
      hw->getClient().dispatch(); // force the transaction
      vald = vw.value();
      if( b64) {
	vw = hw->getClient().read(  addr+i*inc+1);	
	hw->getClient().dispatch();
	vald = vald | ( (uint64_t)vw.value() << 32);
      }
      if( i % wpl == 0)
	printf("%08x: ",  addr+i*inc);
      printf(" %0*llx", b64?16:8, (unsigned long long)vald);
      if( i % wpl == (wpl-1))
	printf("\n");
    }
    if( count % wpl != (wpl-1))
      printf("\n");
      
  } else {
    // count not specified
    if( numericAddr) {
      vw = hw->getClient().read(addr);
      hw->getClient().dispatch();
      val = vw.value();
      printf("%08x: %08x\n", addr, val);
    } else {
//      nodes = myMatchNodes( hw->getChip(chip), strArg[0]);
//      if( nodes.size()) {
//	for( size_t i=0; i<nodes.size(); i++) {
//	  val = hw->getNode(nodes[i]);
//	  printf("%50s: 0x%08x\n", nodes[i].c_str(), val);
//	}
//    }
    }
  }
    
  return 0;
}
  
  
int Launcher::Write(std::vector<std::string> strArg,std::vector<uint64_t> intArg) {

  if (strArg.size() ==0){
    printf("Need at least an address\n");
    return 0;
  }
  std::string saddr = strArg[0];
  std::transform( saddr.begin(), saddr.end(), saddr.begin(), ::toupper);

  try {
    uint32_t addr;
    uint32_t mask;

    switch( strArg.size()) {
    case 1:			// address only means masked write
      printf("Mask write to %s\n", saddr.c_str() );
      addr = hw->getNode(saddr).getAddress();
      mask = hw->getNode(saddr).getMask();      
      hw->getClient().write(addr,mask);
      hw->dispatch();
      break;
    case 2:
      printf("Write to ");
      if( isdigit( saddr.c_str()[0])) {
	printf("address %s\n", saddr.c_str() );
	hw->getClient().write(uint32_t(intArg[0]),uint32_t(intArg[1]));
      } else {
	printf("register %s\n", saddr.c_str());
	hw->getNode(saddr).write(uint32_t(intArg[1]));
      }
      break;
    default:
      printf("Expect address and optional value only\n");
      break;
    }	

  } catch( const uhal::exception::NoBranchFoundWithGivenUID ) {
    printf("Register name not found: %s\n", saddr.c_str() );
  }

  return 0;
}
  
