*	KELVIN LABORATORY, UNIVERSITY OF GLASGOW
*	Default configuration data for STR200 fastbus scaler
*	JRM Annand	28th Sept 1990
*	JRM Annand	18th Jan  1991	reg 11(hex) set for read & clear


##              Dummy Base	#Registers	Module type
BaseSetup:	0		7		VScaler
##		Dummy Index	#Channels	#Bits
BaseIndex:	0		32		32
##		HardwareID	
Hardware-ID:	0x6823		
##            Subaddr	Data
Register:       0x0	c0000000;
Register:	0x0	00000000;
Register: 	0x3	0
Register: 	0x7	0
Register: 	0x10	0
Register: 	0x11	ffffffff
Register: 	0x12	ffffffff

## Set Read Mode of the module
RD-Mode:



