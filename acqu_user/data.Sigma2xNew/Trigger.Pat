##	Trigger Latch Pattern unit Decoder
##	JRM Annand	 22nd Jun 2004
##
##	Pattern unit data is handled by ACQU as a 16 bit "adc".
##	Pattern spectra > 16 channels can be handled by having more
##	than 1 pattern "ADC"
##	For any bits that have no meaning (or don't exit) enter -1
##
Number-Patterns: 2
#Number-Patterns: 1
##
##	     Name	#ADCs		#Channels
PatternSize: Trigger	2		32
## 	    ADC	The 16 bits (lsb->msb) correspond to these 16 hit channels
PatternADC: 0	000 001 002 003 004 005 006 007  -1  -1  -1  -1  -1  -1  -1  -1
PatternADC: 1	008 009 010 011 012 013 014 015  -1  -1  -1  -1  -1  -1  -1  -1
##
##
PatternSize: TaggHelicity	2	32
PatternADC: 5	000 001 002 003 004 005 006 007  -1  -1  -1  -1  -1  -1  -1  -1
PatternADC: 6	008 009 010 011 012 013 014 015  -1  -1  -1  -1  -1  -1  -1  -1
##
#PatternSize: BeamPol	1		16
#PatternADC: 10	000 001 002 003 004 005 006 007  -1  -1  -1  -1  -1  -1  -1  -1
##
##	End
