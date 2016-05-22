
#include "Launcher.hh"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
// for usleep() in local trigger gen
#include <unistd.h>

// split a string on a delimiter
//
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

  int Launcher::Connect(std::vector<std::string> strArg,
			     std::vector<uint64_t> intArg)
  {
    //Check for a filename
    if(strArg.size() ==  0) {
      printf("Connect: Missing connection file.\n");
      return 0;
    }
    
    //create AMC13 module
    if( hw != NULL){
      printf("Deleting old HwInterface");
      delete hw;
      hw = NULL;
    }
    uhal::ConnectionManager manager(strArg[0]);
    hw = new uhal::HwInterface(manager.getDevice("glib.0"));
    return 0;
  }
  
