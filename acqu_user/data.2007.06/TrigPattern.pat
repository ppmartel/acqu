##	Tagger Latch Pattern unit Decoder
##	JRM Annand	 22nd Jun 2004
##
##	Pattern unit data is handled by ACQU as a 16 bit "adc".
##	Pattern spectra > 16 channels can be handled by having more
##	than 1 pattern "ADC"
##	For any bits that have no meaning (or don't exit) enter -1
##	    #ADCs	#Channels
PatternSize: 3		48
## 	    ADC	The 16 bits (lsb->msb) correspond to these 16 hit channels
PatternADC: 0	000 001 002 003 004 005 006 007 024 025 026 027 028 029 030 031 
PatternADC: 1	008 009 010 011 012 013 014 015 032 033 034 035 036 037 038 039
PatternADC: 10	016 017 018 019 020 021 022 023 040 041 042 043 044 045 046 047
##
##	End of trigger pattern unit bits 0-23
