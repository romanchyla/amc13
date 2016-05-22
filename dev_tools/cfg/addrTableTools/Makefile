#
# update XML from CSV sources
# (this is a hack, and should have some fancy rules)
#

t1_ver = v0x24b
t1_ldc = v0x00fb
t2_ver = v0x29

all: ../../../amc13/etc/amc13/AMC13XG_T1.xml \
 ../../../amc13/etc/amc13/AMC13XG_T1_DAQLDC.xml \
 ../../../amc13/etc/amc13/AMC13XG_T2.xml \
 ../../../amc13/etc/amc13/AMCCounters.xml \
 ../../../amc13/etc/amc13/AMCStateCounters.xml \
 ../../../amc13/etc/amc13/AMC13XG_T1.html \
 ../../../amc13/etc/amc13/AMC13XG_T2.html \
 ../../../amc13/etc/amc13/AMCLinks.xml

../../../amc13/etc/amc13/AMC13XG_T1.xml: AMC13XG_T1_$(t1_ver).csv
	perl csv_expand_repeat.pl AMC13XG_T1_$(t1_ver).csv temp.csv
	perl csv_to_xml.pl temp.csv ../../../amc13/etc/amc13/AMC13XG_T1.xml

../../../amc13/etc/amc13/AMC13XG_T1_DAQLDC.xml: AMC13XG_T1_$(t1_ldc).csv
	perl csv_expand_repeat.pl AMC13XG_T1_$(t1_ldc).csv temp.csv
	perl csv_to_xml.pl temp.csv ../../../amc13/etc/amc13/AMC13XG_T1_DAQLDC.xml

../../../amc13/etc/amc13/AMC13XG_T2.xml: AMC13XG_T2_$(t2_ver).csv
	perl csv_expand_repeat.pl AMC13XG_T2_$(t2_ver).csv temp.csv
	perl csv_to_xml.pl temp.csv ../../../amc13/etc/amc13/AMC13XG_T2.xml

../../../amc13/etc/amc13/AMCCounters.xml: AMCCounters.csv
	perl csv_expand_repeat.pl AMCCounters.csv temp.csv
	perl csv_to_xml.pl temp.csv ../../../amc13/etc/amc13/AMCCounters.xml

../../../amc13/etc/amc13/AMCStateCounters.xml: AMCStateCounters.csv
	perl csv_expand_repeat.pl AMCStateCounters.csv temp.csv
	perl csv_to_xml.pl temp.csv ../../../amc13/etc/amc13/AMCStateCounters.xml

../../../amc13/etc/amc13/AMCLinks.xml: AMCLinks.csv
	perl csv_expand_repeat.pl AMCLinks.csv temp.csv
	perl csv_to_xml.pl temp.csv ../../../amc13/etc/amc13/AMCLinks.xml

../../../amc13/etc/amc13/AMC13XG_T1.html: AMC13XG_T1_$(t1_ver).csv
	perl csv_to_html.pl AMC13XG_T1_$(t1_ver).csv ../../../amc13/etc/amc13/AMC13XG_T1.html

../../../amc13/etc/amc13/AMC13XG_T2.html: AMC13XG_T2_$(t2_ver).csv
	perl csv_to_html.pl AMC13XG_T2_$(t2_ver).csv ../../../amc13/etc/amc13/AMC13XG_T2.html
