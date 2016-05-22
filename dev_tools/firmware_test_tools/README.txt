
Some tools to compare registers under different conditions for quick
checking of AMC13 firmware operation.  Files ending in ".amc13" are scripts
for AMC13Tool2 intended to be run as follows:

  AMC13Tool2.exe -c nnn -X file.amc13 [> output]

  dump_status_items.amc13	   tiny AMC13Tool script to list all T1 registers
			   	   with names starting with "STATUS"

  items_to_read_script.pl	   convert output from this script to a list of read
  				   commands to produce a named list of all status
			   	   register contents in an easy-to-diff format

  read_status.amc13		   script to read all registers

To make a new "read_status" script (needed when address table changes):

  $ AMC13Tool2.exe -c nnn/c -X dump_status_items.amc13 > temp.txt
  $ ./items_to_read_script.pl temp.txt > read_status.amc13

To capture AMC13 status registers:

  $ AMC13Tool2.exe -c nnn/c -X read_status.amc13 > status.txt

Use case:

  1.  Initialize AMC13 with e.g.:    AMC13Tool2.exe ... -X init.amc13
  2.  Capture initial status with:   AMC13Tool2.exe ... -X read_status.amc13 > s1.txt
  3.  Send triggers or otherwise:    AMC13Tool2.exe ... -X trig.amc13
  4.  Capture final status with:     AMC13Tool2.exe ... -X read_status.amc13 > s2.txt
  5.  Compare with e.g.:
      diff --width=150 -y pre_of_status.txt post_of_status.txt  | fgrep \|

The output is a list of all registers which have changed state.

There is a perl script which will group the changed registers by
the amount they've changed.  Run with:

  ./show_deltas.pl s1.txt s2.txt

