// Parses the address table and prints out LaTeX tables for documentations purposes

#include "amc13/Status.hh"
#include "amc13/Module.hh"

#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {
  amc13::Module pMod;
  char *atp = getenv("AMC13_ADDRESS_TABLE_PATH");

  //===================
  //Command line args
  //===================
  if (argc < 2) {
    std::cout << "Usage:  LaTeXprint.exe <connection_file>|<ip-address>" << std::endl;
    return 0;
  }

  //Disable the logging
  uhal::disableLogging();

  if( atp == NULL) {
    printf("Please set AMC13_ADDRESS_TABLE_PATH to location of address table files\n");
    exit( 1);
  }

  pMod.Connect( argv[1], atp);
  amc13::AMC13* ptr = pMod.amc13;

  amc13::Status sts(ptr, 0);
  // sts.SetLaTeX();
  // sts.SetHTML();
  std::ofstream file("table.tex"); 
  sts.Report(99,file);
  file.close();
  std::cout << "Wrote to table.tex\n"; 
}
