#!/bin/bash
#
AMC13Tool2.exe -c 161/c -X init.amc13
AMC13Tool2.exe -c 161/c -X trig.amc13
sleep 0.1
AMC13Tool2.exe -c 161/c -X read_status.amc13 > $0_pre_status.txt
AMC13Tool2.exe -c 161/c -X trig.amc13
sleep 0.1
AMC13Tool2.exe -c 161/c -X read_status.amc13 > $0_post_status.txt
echo "diff --width=150 -y $0_pre_status.txt $0_post_status.txt  | fgrep \|"
