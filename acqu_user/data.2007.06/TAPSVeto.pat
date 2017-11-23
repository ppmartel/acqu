##	TAPS Veto Pattern unit Decoder
##	JRM Annand	 22nd Jun 2004
##
##	Pattern unit data is handled by ACQU as a 16 bit "adc".
##	Pattern spectra > 16 channels can be handled by having more
##	than 1 pattern "ADC"
##	For any bits that have no meaning (or don't exit) enter -1
##	    #ADCs	#Channels
PatternSize: 32		512
## 	    ADC		The 16 bits (lsb->msb) correspond to these 16 hit channels
PatternADCAuto: 300	32	0
##
##	End of tagger pattern unit bits 0-351
