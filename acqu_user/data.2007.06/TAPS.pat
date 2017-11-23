##	Tagger Latch Pattern unit Decoder
##	JRM Annand	 22nd Jun 2004
##
##	Pattern unit data is handled by ACQU as a 16 bit "adc".
##	Pattern spectra > 16 channels can be handled by having more
##	than 1 pattern "ADC"
##	For any bits that have no meaning (or don't exit) enter -1
Number-Patterns: 1
##	     Name	#ADCs	#Channels
#PatternSize: TAPSDiscr	512	8192
## 	    	ADC	The 16 bits (lsb->msb) correspond to these 16 hit channels
#PatternADCAuto: 20501	64 0
#PatternADCAuto: 21501	64 1024
#PatternADCAuto: 22501	64 2048
#PatternADCAuto: 23501	64 3072
#PatternADCAuto: 24501	64 4096
#PatternADCAuto: 25501	64 5120
#PatternADCAuto: 26501	64 6144
#PatternADCAuto: 27501	64 7168
##
##	     Name	#ADCs		#Channels
PatternSize: TAPSVeto	32		512
## 	    ADC		The 16 bits (lsb->msb) correspond to these 16 hit channels
PatternADCAuto: 300	32	0
##
##	End of tagger pattern unit bits 0-351
