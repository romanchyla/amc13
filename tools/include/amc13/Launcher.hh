#ifndef HCAL_AMC13_COMMANDS
#define HCAL_AMC13_COMMANDS


#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include "amc13/AMC13.hh"
#include "amc13/Module.hh"

#include <stdint.h>

namespace amc13{
  
  //-- non-member functions
  void ReplaceStringInPlace(std::string& subject, const std::string& search,
			    const std::string& replace);
  std::vector<std::string> myMatchNodes( uhal::HwInterface* hw, const std::string regex);
  
  
  class Launcher {
  public:
    Launcher();
    ~Launcher();
    
    int EvaluateCommand(std::vector<std::string> command);

    //For CLI to auto complete commands
    std::string AutoCompleteCommand(std::string const & line,int state);  
    std::string AutoCompleteSubCommand(std::vector<std::string> const & line,
				     std::string const & currentToken,int state);

    // set/get address table path
    void SetAddressTablePath( std::string p) { addressTablePath = p; }
    std::string GetAddressTablePath() { return addressTablePath; }

  private:
    //A mapping between command string and function to call
    //The called function returns an int and takes an AMC13 class, a vector of argument strings and a vector of uint64_t conversions of those strings
    void AddCommand(std::string name, 
		    int (Launcher::*)(std::vector<std::string>,std::vector<uint64_t>),
		    std::string help, 
		    std::string (Launcher::*)(std::vector<std::string> const &,std::string const &,int)=NULL);
    void AddCommandAlias( std::string alias, std::string existingCommand );
    int FindCommand(std::string command);
    /*! \brief read a chip from a command (t1 or t2)          
     
     */
    int ReadChip(AMC13Simple::Board chip, std::vector<std::string> strArg,
		  std::vector<uint64_t> intArg);
    int WriteChip(AMC13Simple::Board chip, std::vector<std::string> strArg,
		  std::vector<uint64_t> intArg);

    typedef int (Launcher::*CommandFcnPtr)(std::vector<std::string>,std::vector<uint64_t>);
    std::vector<std::string> commandName;
    std::vector<CommandFcnPtr> commandPtr;
    std::vector<std::string> commandHelp;
    std::vector<std::string (Launcher::*)(std::vector<std::string> const &,std::string const &,int)> commandAutoComplete;
    
    // address table path (a bit ugly, but has to go somewhere!)
    std::string addressTablePath;

    //====================================================
    // Keep track of connected AMC13s
    //====================================================
    std::vector<Module*> AMCModule;
    size_t defaultAMC13no;		// current module number
    Module* defaultModule();
    AMC13* defaultAMC13();	// accessor function for the device
    
    //====================================================
    //Add your commands here
    //====================================================
    
    //Here is where you update the map between string and function
    void LoadCommandList();
    
    //Add new command functions here
    int InvalidCommandInclude(std::vector<std::string>,std::vector<uint64_t>);	   
    int Help(std::vector<std::string>,std::vector<uint64_t>);	   
    int Quit(std::vector<std::string>,std::vector<uint64_t>);	   
    int Echo(std::vector<std::string>,std::vector<uint64_t>);	   
    int Sleep(std::vector<std::string>,std::vector<uint64_t>);	   
    int AMC13Connect(std::vector<std::string>,std::vector<uint64_t>);	   
    int AMC13List(std::vector<std::string>,std::vector<uint64_t>);      
    int AMC13Select(std::vector<std::string>,std::vector<uint64_t>);      
    /*! \brief read T1 (virtex/kintex) chip
     */
    int AMC13ReadT1(std::vector<std::string>,std::vector<uint64_t>);      
    /*! \brief read T2 (sparta) chip                                                                                                                                                                                                                                               */
    int AMC13ReadT2(std::vector<std::string>,std::vector<uint64_t>);      
    /*! \brief write T1 (virtex/kintex) chip                                                                                                                                                                                                                                       */
    int AMC13WriteT1(std::vector<std::string>,std::vector<uint64_t>);      
    /*! \brief write T2 (spartan) chip                                                                                                                                                                                                                                             */
    int AMC13WriteT2(std::vector<std::string>,std::vector<uint64_t>);      
    int AMC13Initialize(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13SetOcrCommand(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13SetResyncCommand(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13SetOrbitGap(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13Prescale(std::vector<std::string>,std::vector<uint64_t>);

    int AMC13ConfigL1A(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13ConfigDAQ(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13LocalTrig(std::vector<std::string>,std::vector<uint64_t>);

    int AMC13Start(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13Stop(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13ResetGeneral(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13ResetCounters(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13ResetDAQ(std::vector<std::string>,std::vector<uint64_t>);
    int ListNodes(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13ShowStatus(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13DumpEvent(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13DumpMultiFEDEvent(std::vector<std::string>,std::vector<uint64_t>);

    int AMC13ReadEvent(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13ReadEventVector(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13ReadEventMultiFED(std::vector<std::string>,std::vector<uint64_t>);

    int AMC13ConfigBGO(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13TTCHistory(std::vector<std::string>,std::vector<uint64_t>);

    int AMC13L1AHistory(std::vector<std::string>,std::vector<uint64_t>);

    int AMC13PrintFlash(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13VerifyFlash(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13ProgramFlash(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13ReloadFPGAs(std::vector<std::string>,std::vector<uint64_t>);
 
    int AMC13VerifyFlashFile(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13ProgramFlashFile(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13SelectFileTest(std::vector<std::string>,std::vector<uint64_t>);

    int AMC13VerifyFlashHeader(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13VerifyFlashGolden(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13VerifyFlashT1(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13VerifyFlashT2(std::vector<std::string>,std::vector<uint64_t>);

    int AMC13ProgramFlashHeader(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13ProgramFlashGolden(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13ProgramFlashT1(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13ProgramFlashT2(std::vector<std::string>,std::vector<uint64_t>);

    int AMC13SetID(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13FedID(std::vector<std::string>,std::vector<uint64_t>);
    int AMC13SlinkID(std::vector<std::string>,std::vector<uint64_t>);

    int AMC13CalTriggerWindow(std::vector<std::string>, std::vector<uint64_t>);

    int Status(std::vector<std::string>,std::vector<uint64_t>);
    int StatusHTML(std::vector<std::string>,std::vector<uint64_t>);
    int OpenFile(std::vector<std::string>,std::vector<uint64_t>);
    int CloseFile(std::vector<std::string>,std::vector<uint64_t>);
    int MrWuRegisterDump(std::vector<std::string>,std::vector<uint64_t>);
    void MrWuRegDumpHelper(std::ofstream&, uint32_t);
    int SFPDump(std::vector<std::string>,std::vector<uint64_t>);

    //Add new command (sub command) auto-complete files here
    std::string autoComplete_Help(std::vector<std::string> const &,std::string const &,int);
    std::string autoComplete_T1AddressTable(std::vector<std::string> const &,std::string const &,int);
    std::string autoComplete_T2AddressTable(std::vector<std::string> const &,std::string const &,int);
    std::string autoComplete_StatusTable(std::vector<std::string> const &,std::string const &,int);
  };
}
#endif
  
