// AMC13 header class for simplified software rewrite
#ifndef AMC13_AMC13SIMPLE_HH_INCLUDED
#define AMC13_AMC13SIMPLE_HH_INCLUDED 1

#include <string>
#include <exception>
#include "uhal/uhal.hpp"
#include "amc13/Exception.hh"
#include "amc13/Features.hh"

// define maximum block read size in 32-bit words
#define MAX_BLOCK_READ_SIZE 0x400
#define MAX_BLOCK_WRITE_SIZE 0x400

// other hardware-related constants
#define AMC13SIMPLE_NAMC 12

namespace amc13{
  
  /*! \brief AMC13 Low-level support class (basic read/write operations)
   *
   * This class provides low-level support for an AMC13.  The AMC13 has
   * two boards (T1 and T2), aka "Virtex/Kintex" and "Spartan".  Each
   * has it's own IPBus instance and therefore IP address.
   *
   * The details of how the IP addresses are set are not described here,
   * but the user must provide an IPBus connection file to establish the
   * interface to the AMC13.
   */
  class AMC13Simple{
  public:
    // Enumerations used by the AMC13 class
    /*! \brief Enumeration for the AMC13 class to identify T1 and T2 */
    enum Board{
      UNKNOWN = -1, ///< board type is not (yet) known
      T2 = 0,	    ///< T2 (spartan)
      T1 = 1,	    ///< T1 (virtex/kintex)
      spartan = T2, ///< T2 (spartan)
      virtex = T1,  ///< T1 (virtex/kintex)
      kintex = T1   ///< T1 (virtex/kintex)
    };
    /*! \brief Enumeration for identifying T1 type (i.e. old board w/ Virtex and XG boards w/ Kintex) */
    enum T1Type{ 
      virtex_t1 = 0,  ///< Virtex-6 (AMC13 V1)
      kintex_t1 = 1   ///< Kintex-7 (AMC13XG)
    };
    /*! \brief Constructor using a connection file on disk
      \param ConnectionMap path to XML connection file specifying two ids:
      \param t1id ID name of T1 with address table 
      \param t2id ID name of T2 with address table 
    */
    AMC13Simple(const std::string& ConnectionMap, const std::string& t1id, const std::string& t2id);
    /*! \brief Constructor using user-provided URI and address tables
      \param p_URI1 URI for T1 board
      \param p_AddMap1 XML address table path for T1 board
      \param p_URI2 URI for T2 board
      \param p_AddMap2 XML address table path for T1 board
    */
    AMC13Simple(const std::string& p_URI1, const std::string& p_AddMap1 ,
		const std::string& p_URI2, const std::string& p_AddMap2);

    /*! \brief Constructor using user-provided HwInterface objects 
      \param t1 HwInterface for T1 board
      \param t2 HwInterface for T2 board
     */
    AMC13Simple(const uhal::HwInterface& t1, const uhal::HwInterface& t2 );
    
    /*! \brief Class destructor.  Should cleanly destroy all lower-level objects */
    ~AMC13Simple();
    
    // Getter methods
    /*! \brief Access T1 IPBus device */
    uhal::HwInterface* getT1();
    /*! \brief Access T2 IPBus device */
    uhal::HwInterface* getT2();
    /*! \brief Access IPBus device for specified board
      \param chip Board ID from enum Board (T1 or T2)
    */
    uhal::HwInterface* getChip(AMC13Simple::Board chip);
    
    /*! \brief Read one value from AMC13 register using register name
      \param chip Board ID from enum Board (T1 or T2)
      \param reg Register name from address table
    */
    uint32_t read( AMC13Simple::Board chip, const std::string& reg);
    /*! \brief Read one value from AMC13 register using numeric address
      \param chip Board ID from enum Board (T1 or T2)
      \param addr Address offset from base of board
    */
    uint32_t read( AMC13Simple::Board chip, uint32_t addr);
    /*! \brief Block read from numeric address
      \param chip Board ID from enum Board (T1 or T2)
      \param addr Address offset from base of board
      \param nWords Count in number of words
      \param buffer user-supplied buffer
      \return number of words read
    */
    size_t read(AMC13Simple::Board chip, uint32_t addr, size_t nWords, uint32_t* buffer);
    /*! \brief Block read from named register.
      \param chip Board ID from enum Board (T1 or T2)
      \param reg Register name from address table (must have XML attributes mode="incremental" and size=nnn)
      \param nWords Count in number of words
      \param buffer user-supplied buffer
      \return number of words read
    */
    size_t read(AMC13Simple::Board chip, const std::string& reg, size_t nWords, uint32_t* buffer);
    /*! \brief Single-word Write to named register
      \param chip Board ID from enum Board (T1 or T2)
      \param reg Register name from address table
      \param value Data to be written
    */
    void write( AMC13Simple::Board chip, const std::string& reg, uint32_t value);
    /*! \brief Single-word write to address offset
      \param chip Board ID from enum Board (T1 or T2)
      \param addr Address offset from base of board
      \param value Data to be written
    */
    void write( AMC13Simple::Board chip, uint32_t addr, uint32_t value);
    /*! \brief Block write to address offset
      \param chip Board ID from enum Board (T1 or T2)
      \param addr Address offset from base of board
      \param nWords Word count
      \param data pointer to data
    */
    void write( AMC13Simple::Board chip, uint32_t addr, size_t nWords, uint32_t* data);
    /*! \brief Block write to named register
      \param chip Board ID from enum Board (T1 or T2)
      \param reg Register name from address table
      \param nWords Count in number of words
      \param data user-supplied buffer
    */
    void write( AMC13Simple::Board chip, const std::string& reg, size_t nWords, uint32_t* data);
    /*! \brief Write address table mask value to register.

      Used for AMC13 "action" registers, typically to write a value with only
      a single bit set to '1' to a register to perform e.g. a reset

      \param chip Board ID from enum Board (T1 or T2)
      \param reg Register name from address table (only the mask value is used)
    */
    void writeMask( AMC13Simple::Board chip, const std::string& reg);
    
  protected:
    // HwInterface objects
    uhal::HwInterface* m_T1 ;
    uhal::HwInterface* m_T2 ;

  private:
    // Do Not use default constructor. AMC13 object should only be made using 
    // either connection file method or with a list of URIs and Address Tables    
    AMC13Simple();
    
    // Prevent copying of AMC13 objects
    AMC13Simple( const AMC13Simple& other) ; // prevents construction-copy
    AMC13Simple& operator=( const AMC13Simple&) ; // prevents copying

  };
}

#endif

