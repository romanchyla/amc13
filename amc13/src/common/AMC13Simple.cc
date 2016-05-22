//AMC13 class for simplified software rewrite

#include "amc13/AMC13Simple.hh"

namespace amc13{

  // AMC13 Class Constructors

  // Constructor w/ connection file, specifying T1 and T2 id
  AMC13Simple::AMC13Simple(const std::string& ConnectionMap, const std::string& t1id, const std::string& t2id):m_T1(NULL),m_T2(NULL) {
    
    // If this throws, we haven't done anything yet, so we just let it fall to the caller
    uhal::ConnectionManager ConnectXML("file://"+ConnectionMap);
    
    //Create the uhal HwInterface object for the T1 board
    try {
      m_T1 = new uhal::HwInterface(ConnectXML.getDevice( t1id ));
    } catch (uhal::exception::exception & e) {
      //If this throws, append that this was during the T1 allocation in AMC13Simple and re-throw
      e.append("\n In AMC13Simple(std::string) constructor allocating T1.");
      throw;
    }
    
    //Create a uhal HwInterface object for the T2 board
    try {
      m_T2 = new uhal::HwInterface(ConnectXML.getDevice( t2id ));
    } catch(uhal::exception::exception& e) {
      //Clean up T1 if T2 throws
      if (m_T1!=NULL){
	delete m_T1;
	m_T1=NULL;
      }    
      //re-throw the exception with some notes appended.
      e.append("\n In AMC13Simple(std::string) constructor allocating T2.");
      throw;
    }
  }

  // Constructor specifying URIs and address tables to use
  AMC13Simple::AMC13Simple(const std::string& p_URI1, const std::string& p_AddMap1,
			   const std::string& p_URI2, const std::string& p_AddMap2):m_T1(NULL),m_T2(NULL) {
    std::string URI1 = p_URI1;
    std::string URI2 = p_URI2;
    std::string ADD1 = "file://"+p_AddMap1;
    std::string ADD2 = "file://"+p_AddMap2;
    
    
    //Create the uhal HwInterface object for the T1 board
    try {
      m_T1 = new uhal::HwInterface(uhal::ConnectionManager::getDevice("T1", URI1, ADD1));
    } catch (uhal::exception::exception & e) {
      //If this throws, append that this was during the T1 allocation in AMC13Simple and re-throw
      e.append("\n In AMC13Simple(std::string,std::string,std::string,std::string) constructor allocating T1.");
      throw;
    }
    
    //Create a uhal HwInterface object for the T2 board
    try {
      m_T2 = new uhal::HwInterface(uhal::ConnectionManager::getDevice("T2", URI2, ADD2));
    } catch(uhal::exception::exception& e) {
      //Clean up T1 if T2 throws
      if (m_T1!=NULL){
	delete m_T1;
	m_T1=NULL;
      }    
      //re-throw the exception with some notes appended.
      e.append("\n In AMC13Simple(std::string,std::string,std::string,std::string) constructor allocating T2.");
      throw;
    }
  }

  AMC13Simple::AMC13Simple( const uhal::HwInterface& t1, const uhal::HwInterface& t2 ) {
    try {
      m_T1 = new uhal::HwInterface(t1);
    } catch(uhal::exception::exception& e) {
       //If this throws, append that this was during the T1 allocation in AMC13Simple and re-throw
      e.append("\n In AMC13Simple( uhal::HwInterface, uhal::HwInterface ) constructor allocating T1.");
      throw;
    }
    try{
      m_T2 = new uhal::HwInterface(t2);
    } catch(uhal::exception::exception& e) {
      //Clean up T1 if T2 throws
      if (m_T1!=NULL){
	delete m_T1;
	m_T1=NULL;
      }     
      //re-throw the exception with some notes appended.
      e.append("\n In AMC13Simple( uhal::HwInterface, uhal::HwInterface ) constructor allocating T2.");
      throw;
    }
  }

  /*! \brief Class destructor.  Should cleanly destroy all lower-level objects */
  AMC13Simple::~AMC13Simple(){
    if (m_T1 != NULL){
      delete m_T1;
    }
    if(m_T2 != NULL){
      delete m_T2;
    }
  }

  // Getter methods
  /*! \brief Access T1 IPBus device */
  uhal::HwInterface* AMC13Simple::getT1() {
    return m_T1;
  }
  /*! \brief Access T2 IPBus device */
  uhal::HwInterface* AMC13Simple::getT2() {
    return m_T2;
  }


  uhal::HwInterface* AMC13Simple::getChip(AMC13Simple::Board chip) {
    uhal::HwInterface * ret = NULL;
    switch (chip) {
      
    case T2:       //0 T2 Spartan
      ret = m_T2;
      break;
   
    case T1:       //1 T1 Virtex/Kintex
      ret = m_T1;
      break;
      
    default:
      //Throw "Bad AMC13 Chip"
      throw amc13::Exception::BadChip();
      break; //This isn't really needed
    }
    return ret ;  
  }

  
  
  // Read Methods
  
  
  // Register Read
  uint32_t AMC13Simple::read(AMC13Simple::Board chip, const std::string& reg) {
    uhal::ValWord<uint32_t> ret;
    
    switch (chip) {
      
    case T2:       //0 T2 Spartan
      ret = m_T2 -> getNode( reg ).read() ;
      m_T2 -> dispatch() ;
      break;
      
    case T1:       //1 T1 Virtex/Kintex
      ret = m_T1 -> getNode( reg ).read() ;
      m_T1 -> dispatch() ;
      break;
      
    default:
      //Throw "Bad AMC13 Chip"
      throw amc13::Exception::BadChip();
      break; //This isn't really needed
    }
    return ret ;  
  }
  
  
  // Address Read
  uint32_t AMC13Simple::read( AMC13Simple::Board chip, uint32_t addr) {
    uhal::ValWord<uint32_t> ret;
    
    switch (chip) {
      
    case T2:       //0 T2 Spartan
      ret = m_T2 -> getClient().read( addr ) ;
      m_T2 -> getClient().dispatch() ;
      break;
      
    case T1:       //1 T1 Virtex/Kintex
      ret = m_T1 -> getClient().read( addr ) ;
      m_T1 -> getClient().dispatch() ;
      break;
      
    default:
      //Throw "Bad AMC13 Chip"
      throw amc13::Exception::BadChip();
      break; //This isn't really needed
    }
    
    return ret ;  
  }
  
  // Block Read from Address
  size_t AMC13Simple::read(AMC13Simple::Board chip, uint32_t addr, size_t nWords, uint32_t* buffer) {
    
    //Make sure we have a valid buffer
    if ( buffer==NULL ) {
      throw amc13::Exception::NULLPointer() ;
    }
    
    //choose the board we are using
    uhal::HwInterface * brd;
    
    switch (chip) {
    case T2:       //0 T2 Spartan
      brd = m_T2;
      break;
    case T1:       //1 T1 Virtex/Kintex
      brd = m_T1;
      break;
    default:
      //Throw "Bad AMC13 Chip"
      throw amc13::Exception::BadChip();
      break; //This isn't really needed
    }
    
    uhal::ValVector<uint32_t> retVec ;
    std::vector<uint32_t> stdVec ;
    uint32_t offset = 0 ;
    int remainingWords = nWords ;
    
    //Do the read, pass on uhal exceptions
    while( remainingWords ) {
      uint32_t wordsToRead = remainingWords > MAX_BLOCK_READ_SIZE ? MAX_BLOCK_READ_SIZE : remainingWords ; 
      retVec = brd -> getClient(). readBlock( ( addr+offset ), wordsToRead, uhal::defs::INCREMENTAL );
      brd -> getClient(). dispatch() ;
      std::copy( retVec.begin(), retVec.end(), std::back_inserter( stdVec ) ) ;
      remainingWords -= wordsToRead ;
      offset += wordsToRead ;
    }
    
    // Make sure stdVec is equal or smaller than the number of requested words
    if ( stdVec.size() > nWords) {
      throw amc13::Exception::UnexpectedRange();
    }
    
    // Copy Block read (stdVec) to buffer array
    for ( size_t i=0; i <  stdVec.size(); i++) {
      buffer[i] = stdVec[i] ;
    }
    return stdVec.size() ;
  }
  
  // Block Read from Register Name
  // Note that a register node needs mode="incremental" and size to block read from a register
  size_t AMC13Simple::read(AMC13Simple::Board chip,  const std::string& reg, size_t nWords, uint32_t* buffer) {
    
    //Make sure we have a valid buffer
    if ( buffer==NULL ) {
      throw amc13::Exception::NULLPointer();
    }
    
    //choose the board we are using
    uhal::HwInterface * brd;
    
    switch (chip) {
    case T2:       //0 T2 Spartan
      brd = m_T2;
      break;
    case T1:       //1 T1 Virtex/Kintex
      brd = m_T1;
      break;
    default:
      //Throw "Bad AMC13 Chip"
      throw amc13::Exception::BadChip();
      break; //This isn't really needed
    }
    
    uhal::ValVector<uint32_t> retVec ;
    std::vector<uint32_t> stdVec ;
    uint32_t offset = 0 ;
    int remainingWords = nWords ;
    
    // Check if register has incremental mode
    if ( ( brd -> getNode( reg ). getMode() == uhal::defs::INCREMENTAL ) && ( nWords < MAX_BLOCK_READ_SIZE ) ) {
      // Using register block read
      retVec = brd -> getNode( reg ). readBlock( nWords );
      brd -> dispatch() ;
    } else {
      // Using address block read (starting address taken from reg)
      uint32_t nodeAddr = brd -> getNode( reg ). getAddress();
      while ( remainingWords ) {
	uint32_t wordsToRead = remainingWords > MAX_BLOCK_READ_SIZE ? MAX_BLOCK_READ_SIZE : remainingWords ; 
	retVec = brd -> getClient(). readBlock( ( nodeAddr + offset ), wordsToRead, uhal::defs::INCREMENTAL );
	brd -> getClient(). dispatch() ;    
	std::copy( retVec.begin(), retVec.end(), std::back_inserter( stdVec ) ) ;
	remainingWords -= wordsToRead ;
	offset += wordsToRead ;
      }
    }
    
    // Make sure stdVec is equal or smaller than the number of requested words
    if ( stdVec.size() > nWords) {
      throw amc13::Exception::UnexpectedRange();
    }
    
    // Copy Block read (stdVec) to buffer array
    for ( size_t i=0; i < stdVec.size(); i++) {
      buffer[i] = stdVec[i] ;
    }
    
    return stdVec.size() ;
  }
  
  // Write Methods
  // Register Write
  void AMC13Simple::write(AMC13Simple::Board chip, const std::string& reg, uint32_t value) {
    
    switch (chip) {
      
    case T2:       //0 T2 Spartan
      m_T2 -> getNode( reg ).write( value );
      m_T2 -> dispatch() ;
      break;      
    case T1:       //1 T1 Virtex/Kintex
      m_T1 -> getNode( reg).write( value);
      m_T1 -> dispatch() ;
      break;
      
    default:
      //Throw "Bad AMC13 Chip"
      throw amc13::Exception::BadChip();
      break; //This isn't really needed
    }
  }
  
  // Address Write
  void AMC13Simple::write(AMC13Simple::Board chip, uint32_t addr, uint32_t value) {
    switch (chip) {
      
    case T2:       //0 T2 Spartan
      m_T2 -> getClient().write( addr, value);
      m_T2 -> getClient().dispatch() ;
      break;
      
    case T1:       //1 T1 Virtex/Kintex
      m_T1 -> getClient().write( addr, value);
      m_T1 -> getClient().dispatch() ;
      break;
      
    default:
      //Throw "Bad AMC13 Chip"
      throw amc13::Exception::BadChip();
      break; //This isn't really needed
    }
    
  }
  
  // Block Write to Address
  void AMC13Simple::write(AMC13Simple::Board chip, uint32_t addr, size_t nWords, uint32_t* data) {
    
    //Make sure we have a valid buffer
    if ( data==NULL ) {
      throw amc13::Exception::NULLPointer();
    }
    
    //choose the board we are using
    uhal::HwInterface * brd;
    
    switch (chip) {
    case T2:       //0 T2 Spartan
      brd = m_T2;
      break;
    case T1:       //1 T1 Virtex/Kintex
      brd = m_T1;
      break;
    default:
      //Throw "Bad AMC13 Chip"
      throw amc13::Exception::BadChip();
      break; //This isn't really needed
    }
    
    //Create writeVector and load it with our data
    std::vector<uint32_t> writeVec;
    writeVec.resize(nWords);
    memcpy( &(writeVec[0]), data, nWords*sizeof(uint32_t) );
    brd -> getClient().writeBlock( addr, writeVec, uhal::defs::INCREMENTAL );
    brd -> dispatch();  
  }
  // Block Write to Register
  void AMC13Simple::write(AMC13Simple::Board chip, const std::string& reg, size_t nWords, uint32_t* data) {
    //Make sure we have a valid buffer
    if ( data==NULL ) {
      throw amc13::Exception::NULLPointer();
    }
    
    //choose the board we are using
    uhal::HwInterface * brd;
    
    switch (chip) {
    case T2:       //0 T2 Spartan
      brd = m_T2;
      break;
    case T1:       //1 T1 Virtex/Kintex
      brd = m_T1;
      break;
    default:
      //Throw "Bad AMC13 Chip"
      throw amc13::Exception::BadChip();
      break; //This isn't really needed
    }
    
    //Create writeVector and load it with our data
    std::vector<uint32_t> writeVec;
    writeVec.resize(nWords);
    memcpy(&(writeVec[0]),data,nWords*sizeof(uint32_t));
    
    brd -> getNode( reg ).writeBlock( writeVec );
    brd -> dispatch();  
  }


  // Write the mask value
  void AMC13Simple::writeMask(AMC13Simple::Board chip, const std::string& reg) {

    //choose the board we are using
    uhal::HwInterface* brd = NULL;

    switch (chip) {

    case T2:       //0 T2 Spartan
      brd = m_T2;
      break;

    case T1:       //1 T1 Virtex/Kintex
      brd = m_T1;
      break;

    default:
      //Throw "Bad AMC13 Chip"
      throw amc13::Exception::BadChip();
      break; //This isn't really needed
    }

    uint32_t addr = brd->getNode( reg).getAddress();
    uint32_t mask = brd->getNode( reg).getMask();
    brd->getClient().write(addr, mask);
    brd->dispatch();

  }

}

