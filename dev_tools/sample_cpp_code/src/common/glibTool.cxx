#include "uhal/uhal.hpp"
#include <string.h>
#include <stdlib.h>

using namespace uhal;

#define WORDS_PER_LINE 8

int main() {

  char buff[100];

  ConnectionManager manager("file://glib_conn.xml");
  HwInterface hw = manager.getDevice("glib.0");
 
  char *pc, *p1, *p2;
  ValWord< uint32_t > reg;
  uint32_t addr;
  uint32_t valu;
  int count;

  int running = 1;
  while(running) {
    printf("> ");
    gets( buff);

    pc = strtok( buff, " ");
    if( pc) {
      switch( toupper( *pc)) {
      case 'Q':
	running = 0;
	break;
      case 'R':
	p1 = strtok( NULL, " ");
	if( !p1) {
	  printf("Need address\n");
	} else {
	  p2 = strtok( NULL, " ");
	  if( p2)
	    count = strtoul( p2, NULL, 0);
	  else
	    count = 1;

	  addr = strtoul( p1, NULL, 0);
	  for( int i=0; i<count; i++) {
	    reg = hw.getClient().read(addr);
	    hw.dispatch();
	    if( (i % WORDS_PER_LINE) == 0)
	      printf("%08x: ", addr);
	    printf(" %08x", reg.value());
	    if( ((i+1) % WORDS_PER_LINE) == 0)
	      printf("\n");
	    ++addr;
	  }
	  if( (count % WORDS_PER_LINE) != 0)
	     printf("\n");

	}
	break;

      case 'W':
	p1 = strtok( NULL, " ");
	p2 = strtok( NULL, " ");
	if( !p1 || !p2) {
	  printf("Need address and value to write\n");
	} else {
	  addr = strtoul( p1, NULL, 0);
	  valu = strtoul( p2, NULL, 0);
	  hw.getClient().write( addr, valu);
	  hw.dispatch();	  
	}
	break;

      default:
	printf("What?\n");
      }
    }

  }



}
