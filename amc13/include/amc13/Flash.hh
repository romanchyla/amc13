#ifndef AMC13_AMC13_FLASH_HH_INCLUDED
#define AMC13_AMC13_FLASH_HH_INCLUDED 1

#include <vector>
#include <stdint.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <dirent.h> // accessing files in current directory

#include "amc13/AMC13Simple.hh"
#include "amc13/Exception.hh"
#include "uhal/log/exception.hpp"

namespace amc13{
  
  /*! \brief A class which handles the flash programming and firmware configuration for the AMC13
   *
   * This class provides programming, readback and verification of the SPI flash memory
   * on the T2 board which stores FPGA configuration data.  The flash is layed out (as of this writing)
   * as follows:
   *
   * - Offset 0x000000 - flash header
   * - Offset 0x080000 - Backup firmware for Spartan LX45T chips
   * - Offset 0x100000 - Backup firmware for Spartan LX25T chips
   * - Offset 0x200000 - Spartan primary firmware image
   * - Offset 0x400000 - Virtex or Kintex firmware image
   */
  class Flash {
  public:
    //Contructor and Destructor
    /// AMC13_flash class constructor which takes as arguments a pointer to an AMC13 object
    Flash(uhal::HwInterface* T2);
    ~Flash();

  //Needed to add in the more complex read/write commands from AMC13Simple
    size_t read( uint32_t addr, size_t nWords, uint32_t* buffer); 
    uint32_t read(const std::string& reg);
    void write( uint32_t addr, size_t nWords, uint32_t* data);


    //flash read
    /*! \brief Read one page (256 bytes) from flash memory and return a vector of 32-bit word read values
     * \param addr starting byte address in flash
     * \return 64-element vector of 32-bit data items, packed low-byte first into vector items
     */
    std::vector<uint32_t> readFlashPage(uint32_t addr) ;
    /*! \brief Reads multiple pages (256 bytes per page) from flash memory
     * \param add starting byte address in flash
     * \param pages number of 256-byte pages to read
     * \return 64*pages element vector of 32-bit data items, packed low-byte first into vector items
     */
    std::vector<uint32_t> firmwareFromFlash(uint32_t add, int pages) ;
    /*! \brief Program one page (256 bytes) to flash memory
     * \param data 64-element array of values.  Low byte programmed into first location
     */
    void writeFlashPage(uint32_t, std::vector<uint32_t> data) ;
    /*! \brief Erases one flash sector (256K bytes) in flash
     * \param add sector address to erase
     */
    void eraseFlashSector(uint32_t add) ;
    /*! \brief Programs AMC13 firmware file in Intel hex (MCS) format into flash memory
     *
     * Parse file name per AMC13 naming convention AMC13Txv0xzzzz_ccccc.mcs where:
     * - x is 1 for T1 or 2 for T2
     * - zzzz is hex version number
     * - ccccc is chip type, e.g. 6slx45t, 7k325t
     * Use information from file name to determine flash offset
     *
     * throws exceptions from amc13::Exception class on errors
     *
     * \param file path to Intel hex file written by Xilinix ISE
     */
    void programFlash(const std::string& file);
    /*! \brief Programs Intel hex (MCS) file into flash memory
     *
     * Program any Intel hex file to specified address.  AMC13 naming
     * convention not required.
     *
     * \param file Intel hex (MCS) file name
     * \param addr starting address offset (added to address in hex file)
     */
    void programFlash(const std::string& file, uint32_t addr);
    /*! Verify flash against AMC13 hex file
     *
     * Parse file name per AMC13 naming convention AMC13Txv0xzzzz_ccccc.mcs where:
     * - x is 1 for T1 or 2 for T2
     * - zzzz is hex version number
     * - ccccc is chip type, e.g. 6slx45t, 7k325t
     * Use information from file name to determine flash offset
     *
     * compare data against flash contents.  <b>Currently only displays results, does not return anything useful!</b>
     * \param file file path
     */
    void verifyFlash(const std::string& file) ;
    /*! \brief Compare flash contents against MCS file
     *
     * Program any Intel hex file to specified address.  AMC13 naming
     * convention not required.
     *
     * \param file Intel hex (MCS) file name
     * \param addr starting address offset (added to address in hex file)
     */
    void verifyFlash(const std::string& file, uint32_t addr) ;
    /*! \brief Trigger reconfiguration of T1 FPGA
     *
     * Reconfigure Kintex/Virtex FPGA from flash memory.  Function returns immediately but
     * the actual reconfiguration takes several seconds.
     */
    void loadFlashT1() ;
    /*! \brief Trigger reconfiguration of T1 FPGA
     *
     * Reconfigure both T1 and T2 FPGA from flash memory.  Function returns immediately but
     * the actual reconfiguration takes several seconds.
     */
    void loadFlash() ;
    /*! \brief Parse AMC13-format firmware file name
     *
     * Parse file name per AMC13 naming convention AMC13Txv0xzzzz_ccccc.mcs where:
     * - x is 1 for T1 or 2 for T2
     * - zzzz is hex version number
     * - ccccc is chip type, e.g. 6slx45t, 7k325t
     *
     * Set some private member variables so the chip type and flash offset
     * can be used later.
     */
    void parseMcsFile( std::string p_name ) ;
    /*! \brief Display names of AMC13 firmware files in current directory and let user choose.
     *
     * Interact with user using stdout/stdin to select file
     *
     * \param chipNo is T1 or T2
     * \param ver is version (not used?)
     * \param chipType is Xilinx part number
     * \return selected file name
     */
    std::string selectMcsFile( int chipNo, std::string ver, std::string chipType) ;
    
    /// Calculate flash offset from chip number, version and type
    uint32_t offset( int chipNo, std::string ver, std::string chipType) ;

    /// Determine FPGA type from serial number
    std::string chipTypeFromSN( int chipNo, int sn) ;

    /// Reset MCS file name member variables and throw an exception
    void clearThrow( std::string appendstr) ;

    /// Reset MCS file name member variables
    void clear() ;
    
  private:
    //Other class objects used by AMC13_flash
    uhal::HwInterface * FlashT2;
    
    //flash control
    /// Issues a flash command parameter int (arg 0)
    void flashDoCommand(int) ;
    /// Enables flash writing
    void enableFlashWrite() ;
    /// Returns when a flash write is completed
    void waitForWriteDone() ;
    
    //MCS file handling
    /// Reads the entirety of an MCS file into a vector 32-bit words which is returned when the read is finished
    std::vector<uint32_t> firmwareFromMcs(const std::string&);

    // Helper functions for string parsing/converting    
    bool parseChipType(std::string chip_type) ;
    uint32_t intFromString(const std::string& s, unsigned int pos, unsigned int n) ;
 
    // Variables to hold parsed information
    std::string file_name;
    std::string lead_const;
    int chip_no;
    std::string version;
    std::string firm_ver_str;
    int firm_ver_int;
    std::string revision_str;
    int revision_int;
    std::string firm_type;
    std::string chip_type;
    std::string trail_const;
    std::string series;
    std::string family;
    std::string type;
    std::string size;
    std::string type_suffix;
    bool valid;
    std::string error;    
    uint32_t flash_address;

    Flash();//do not implement this
  };
  
}
#endif //HCAL_AMC13_AMC13_FLASH_HH_INCLUDED
