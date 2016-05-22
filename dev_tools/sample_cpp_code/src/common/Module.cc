#include "amc13/Module.hh"

#include <boost/regex.hpp>

//For networking constants and structs
#include <sys/types.h>  
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h> //for inet_ntoa


namespace amc13{
#define T1 (AMC13Simple::T1)
#define T2 (AMC13Simple::T2)
  
  Module::Module() {
    amc13 = NULL;
    connectionFile = "??";
    serialNo = revT1 = revT2 = 0;
    stream = NULL;
    fileName = "";
  }
  
  
  Module::~Module() {
    if( amc13 != NULL) {
      delete amc13;
    }
    if (stream != NULL) {
      stream->close();
      delete stream;
    }
  }
  
  
  // connect to module
  // Expects either a connection file name, or an IP address
  // IP address for now must be numeric, and may have suffix "/c"
  // to force use of controlhub
  // IP address is that of T2 board; T1 must be next higher address
  //
  void Module::Connect( const std::string file, const std::string addressTablePath, const std::string prefix) {
    connectionFile = file;

    // check for numeric IP address using Boost
    // separate last octet so we can increment it
    static const boost::regex re("(\\d{1,3}.\\d{1,3}.\\d{1,3}.)(\\d{1,3})(/[cC])?");
    static const boost::regex sn("(\\d{1,3})(/[cC])?");
    static const boost::regex fileXml("\\S*\\.[xX][mM][lL]");
    static boost::cmatch what;
    static char t1_uri[256];
    static char t2_uri[256];

    if( boost::regex_match( file.c_str(), what, re)) {
      std::string ip_addr(what[1].first, what[1].second); // extract 1st 3 octets
      std::string ip_last(what[2].first, what[2].second); // extract last octet
      uint8_t oct_last = atoi( ip_last.c_str());
      bool use_ch = what[3].matched; // check for /c suffix
      if( use_ch) {
	printf("use_ch true\n");
      }
      else
	printf("use_ch false\n");

      // specify protocol prefix
      std::string proto = use_ch ? "chtcp-2.0://localhost:10203?target=" : "ipbusudp-2.0://";

      snprintf( t2_uri, 255, "%s%s%d:50001", proto.c_str(), ip_addr.c_str(), oct_last);
      snprintf( t1_uri, 255, "%s%s%d:50001", proto.c_str(), ip_addr.c_str(), oct_last+1);      
      
      printf("Created URI from IP address:\n  T2: %s\n  T1: %s\n", t2_uri, t1_uri);

      amc13 = new AMC13( t1_uri, addressTablePath + "/AMC13XG_T1.xml", t2_uri, addressTablePath + "/AMC13XG_T2.xml");
    } 
    else if( boost::regex_match( file.c_str(), what, fileXml) ) { // connection file ends with ".xml" 
      printf("Using .xml connection file...\n");
      if( prefix==""){ // use default "T1" and "T2" xml IDs
	amc13 = new AMC13( file);
      }
      else{ // specified xml ID prefix
	const std::string t1id = prefix + ".T1";
	const std::string t2id = prefix + ".T2";
	amc13 = new AMC13( file, t1id, t2id);
      }
    }
    else if(boost::regex_match( file.c_str(), what, sn)){
      int serialnum = atoi(what[1].first);
      uint8_t oct_last = 254-(2*serialnum)%256;
      int x = serialnum/128+1;
  
      bool use_ch = what[2].matched; // check for /c suffix
      if( use_ch) {
	printf("use_ch true\n");
      }
      else
	printf("use_ch false\n");

      // specify protocol prefix
      std::string proto = use_ch ? "chtcp-2.0://localhost:10203?target=" : "ipbusudp-2.0://";

      snprintf( t2_uri, 255, "%s192.168.%i.%d:50001", proto.c_str(), x, oct_last);
      snprintf( t1_uri, 255, "%s192.168.%i.%d:50001", proto.c_str(), x, oct_last+1);      
      
      printf("Created URI from IP address:\n  T2: %s\n  T1: %s\n", t2_uri, t1_uri);

      amc13 = new AMC13( t1_uri, addressTablePath + "/AMC13XG_T1.xml", t2_uri, addressTablePath + "/AMC13XG_T2.xml");
    }
    else { // hostname less '_t2' and '_t1'
      printf("does NOT match\n");
      // define necessary variables
      char ip_t2[16], ip_t1[16];
      std::string name_t2, name_t1;
      name_t2 = file+"_t2";
      name_t1 = file+"_t1";

      // convert hostname to ips
      printf("converting host name...\n");
      hostnameToIp( name_t2.c_str(), ip_t2);
      hostnameToIp( name_t1.c_str(), ip_t1);
      printf("making proto...\n");

      // specify protocol prefix
      std::string proto = "ipbusudp-2.0://"; // ATTENTION: may need to add check for chtcp protocol

      printf("copying uris");
      // copy ip address to appropriate uris
      snprintf( t2_uri, 255, "%s%s:50001", proto.c_str(), ip_t2 );
      snprintf( t1_uri, 255, "%s%s:50001", proto.c_str(), ip_t1 );
      printf("Created URI from IP address:\n  T2: %s\n  T1: %s\n", t2_uri, t1_uri);
      // Create amc13 object
      amc13 = new AMC13( t1_uri, addressTablePath + "/AMC13XG_T1.xml", t2_uri, addressTablePath + "/AMC13XG_T2.xml");      
    }
    // try to read some stuff
    serialNo = amc13->read( T2, "STATUS.SERIAL_NO");
    revT1 = amc13->read( T1, "STATUS.FIRMWARE_VERS");
    revT2 = amc13->read( T2, "STATUS.FIRMWARE_VERS");
  }
  
  std::string Module::Show() {
    char buff[80];
    snprintf( buff, 80, "SN: %3d T1v: %04x T2v: %04x cf: %s", serialNo, revT1, revT2, connectionFile.c_str());
    std::string str(buff);
    return str;
  }

  //If there is no file open, return std::cout
  std::ofstream& Module::getStream() {
    return *stream;
  }

  //If there is a file currently open, it closes it
  void Module::setStream(const char* file) {
    if (stream != NULL) {
      stream->close();
      delete stream;
      stream = NULL;
    }
    stream = new std::ofstream;
    stream->open(file);
    fileName = file;
  }

  //Closes the stream
  void Module::closeStream() {
    if (stream != NULL) {
      stream->close();
      delete stream;
      stream = NULL;
      fileName = "";
    }
  }

  // get ip from domain name
  // ATTENTION should probably be void with throws instead of return 1 for failure, 0 for success
  void Module::hostnameToIp( const char *hostname, char *ip) { //change hostname to const str
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_in *h;
    int rv;
    
    printf("memset hints\n");
    memset(&hints, 0, sizeof hints);
    printf("memset hints complete\n");
    hints.ai_family = AF_INET; // use AF_INET assuming IPv4 or AF_UNSPEC if also accept IPv6 
    hints.ai_socktype = SOCK_STREAM;
    
    if ( (rv = getaddrinfo( hostname , NULL , &hints , &servinfo) ) != 0) {
      // ATTENTION: should throw exception and append gai_strerror(rv), print and return 1 placeholder for now
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); 
      return;
    }
    printf("start loop\n");
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
      h = (struct sockaddr_in *) p->ai_addr;
      printf("strcpy\n");
      //ATTENTION: Should check that ip is large enough to copy given IP address
      strcpy(ip , inet_ntoa( h->sin_addr ) );
    }
    printf("free\n");
    freeaddrinfo(servinfo); // all done with this structure
  }

}
