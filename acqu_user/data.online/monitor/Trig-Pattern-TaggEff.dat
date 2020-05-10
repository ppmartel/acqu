##	Trigger Latch Pattern unit Decoder
##	JRM Annand	 22nd Jun 2004
##
##	Pattern unit data is handled by ACQU as a 16 bit "adc".
##	Pattern spectra > 16 channels can be handled by having more
##	than 1 pattern "ADC"
##	For any bits that have no meaning (or don't exit) enter -1
##
# number of PatternSize: statements below
Number-Patterns: 4
#
##	     Name	#ADCs		#Channels
PatternSize: Trigger	3		48
## 	    ADC	The 16 bits (lsb->msb) correspond to these 16 hit channels
PatternADC: 0	000 001 002 003 004 005 006 007 008 009 010 011 012 013 014 015
PatternADC: 1	016 017 018 019 020 021 022 023 024 025 026 027 028 029 030 031
PatternADC: 8	032 033 034 035 036 037 038 039 -1  -1  -1  -1  -1  -1  -1  -1 
##
PatternSize: Multiplicity	4	64
## 	    ADC	The 16 bits (lsb->msb) correspond to these 16 hit channels
PatternADC: 2	000 001 002 003 004 005 006 007 008 009 010 011 012 013 014 015
PatternADC: 3	016 017 018 019 020 021 022 023 024 025 026 027 028 029 030 031
PatternADC: 4	032 033 034 035 036 037 038 039 040 041 042 043 044 045 046 047
PatternADC: 5	048 049 050 051 052 053 054 055 056 057 058 059 060 061 062 063
##
# ADC no 6 is calculated multiplicity
##
PatternSize: MAMIHelicity 1 16
PatternADC: 7 000 001 002 003 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 004  
##
PatternSize: TaggHelicity	2	16
PatternADC: 10	000 001 002 003 004 005 006 007  -1  -1  -1  -1  -1  -1  -1  -1
PatternADC: 11	008 009 010 011 012 013 014 015  -1  -1  -1  -1  -1  -1  -1  -1
##
