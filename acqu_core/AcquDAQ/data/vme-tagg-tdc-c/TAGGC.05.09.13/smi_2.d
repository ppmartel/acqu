************************************************************************
*			  Kelvin Laboratory
*			University of Glasgow
*			*******smi_2.d*******
*
*	1821 SMI code for ECL bus readout of FASTBUS developed by
*	B. Oussena
*	B.Oussena	24th Nov 1992	Final touches before return
*					to Algiers
*	JRM Annand	12th May 1992	Redo format, extend space for
*					adc's and scalers
*	JRM Annand	17th May 1992	Do error stuff properly	
*
***********************************************************************
*
*	SMI code file conventions:
*	smi_0.d: almost standard LeCroy supplied code
*	smi_1.d: fast readout loop, random software read
*	smi_2.d: fast readout loop, block software read
*	smi_3.d: fast readout loop, random hardware read
*	smi_4.d: fast readout loop, block hardware read
*	smi_5.d: basic JRMA mods of LeCroy..expts till Apr 1993
*
*	All non-comment lines should have 9 fields
*	0: line number..must increase monotonically
*	1: instruction code
*	2: condition code
*	3: bus control code
*	4: hs bus data
*	5: strobe code
*	6: data flow control
*	7: fastbus protocol 1
*	8: fastbus protocol 2
*
***********************************************************************
*
*	Sequencer Idle Loop
*
* 0  1  2  3  4  5  6  7  8	Field alignment
  0  8 42  4 00 00  0  2 00
  1  a 42  4 01 00  0  2 00
  2  0 00  4 00 00  0  0 00
*
*	Real time loop, check if host demands attention, 
*	or if frontpanel input 1 signals event readout
*
 03  8 00  0 00 00  0  7 08	Set RDOC....for ECL bus control
 04  8 00  0 00 00  0  3 08	Reset RDOC..for ECL bus control
 05  8 00  0 00 00  0  0 00	Delay 32ns
 06  8 8d  4 00 00  0  7 20 Test HOST (bit 5 0f Reg 0) & set GK.. grap bus
 07  a f0  4 0a 00  0  0 00 br to 0a if HOST, test ^LFPCC1
 08  a 8d  4 06 00  0  0 00 Loop back if ^LFPCC1, test HOST bit 
 09  c 00  4 af 00  0  0 42 clear LFPCC1 ... jump to readout routine at af
 0a  c 00  4 00 00  0  0 00 Return to idle loop and reset ATTN
*
*	Fast front panel read...call internal block read loop
*	from Host instead of real time loop
*	Call 0b for scaler read
*	Call 0d for ADC read
*
 0b  8 00  4 10 00 03 05 80	load 10->NREG for scaler readout
 0c  c 00  4 b0  0  0  0  0	jump to address loop
 0d  c 00  4 af  0  0  0  0	jump to address loop for ADC readout
*
*	The list of scaler geographical addresses starts here
*	Line 10 sets up the return to this list
*	Line 4f contains the jump statement out of the list
*	Max 30 scaler modules addressed by 1 SMI
*
 10  4 00  4 00 00  0  0 00     NRET..fetch Adr in RAR, load NSAR into RAR
*
*	The list of ADC geographical addresses starts here
*	Line 50 sets up the return to this list
*	Line ab max contains the jump statement out of the list
*	Max 43 ADC modules addressed by 1 SMI
*
 50  4 00  4 00 00  0  0 00     NRET..fetch Adr in RAR, load NSAR into RAR
*
*	ECL bus readout...toggle RDOC to transfer bus control to
*	next SMI, and jump back to real time loop					
*
 ac  8 00  4 00 00  0  7 08 	Set RDOC, PDREG=00 good return
 ad  8 00  0 00 00  0  0 00 	delay
 ae  c 00  4 06 00  0  3 08	Reset RDOC flip-flopx, jmp to 06
*
*	Start line for getting ADC addresses = 50
*
 af  8 00  4 50 00  3  5 80	N-REG = 50
*
*	Module address and block read loop
*	Primary address, Secondary address then block read
*
 b0  d 00  4 fb 00  0  0 00	call fb, clear bus
 b1  8 00  4 00 10  3  5 80	incr NREG for next item on list
*
 b2  d 00  c 00 40  3  5 80	get primary adr from list
 b3  d 00  4 d1 00  0  0 00	call d1 : exec primary address
*
* b4  8 00  4 00 00  0  0 00	Pause
 b4  8 00  4 00 10  3  5 80	incr NREG for next item on list
*
 b5  d 00  c 00 40  3  5 80	get secondary address from list
 b6  8 00  4 00 00  0  2 1f	NOP...was set PDREG=22
 b7  d 00  4 d9 00  0  6 02	call d9 : Fb write procedure
*
 b8  d 00  4 bc 00  0  2 10	call block read bc
 b9  c 00  4 b0 00  0  0 00	loop round for next module
*
*	Subroutine to execute a "block" read
*	This one is software BLOCK read
*
 bc  8 19  4 00 00  0  6 09  set up RD; *PDREG=28**deleted
 bd  a 19  4 bd 00  0  2 06  make sure DK is down, & MS1=MS2=0
 be  8 19  4 00 00  0  6 11  tst DK=1, DS=1, MS0=1, *PDREG=29*, reset timer
 bf  a 47  4 c2 00  0  2 00  br if DK=1, tst .not. timer
 c0  a 19  4 bf 00  0  2 00  br if .not. timer, tst DK=1
 c1  c 00  4 e2 00  0  0 00  timeout error, jmp to e2 : flag error
 c2  8 21  4 00 01  8  2 00  tst SS=1, *PDREG=2a*, latch 32 Bit in word mode
 c3  a 60  4 c6 00  0  2 00  br if SS=1, tst SS.ne.0, skip to $A if busy
 c4  a 22  4 ce 00  0  2 00  br if SS.ne.0, tst SS=2
 c5  8 00  4 00 00  4  0 00  pipe data
 c6  8 59  4 00 04  0  2 10  tst DK.ne.1, *PDREG=2b*, DS=0, reset timer
 c7  a 47  4 ca 00  0  2 10  br if DK.ne.1, tst .not. timer, DS=0
 c8  a 59  4 c7 00  0  2 10  br if .not. timer, tst Dk.ne.1, DS=0
 c9  c 00  4 e2 00  0  0 00  timeout error; jmp to e2 : flag error
 ca  8 21  4 00 01  8  2 00  tst SS=1, *PDREG=2c*, latch 32 B in word mode
 cb  a 60  4 cd 00  0  2 00  br on SS=1, tst SS.ne.0
 cc  a 22  4 ce 00  0  2 00  br SS.ne.0, tst SS=2
 cd  c 00  4 be 00  4  2 00  exec loop again, pipe data
 ce  a 00  4 d0 00  0  0 00  good return br on SS=2
 cf  c 00  4 e2 00  0  0 00  otherwise jmp e2 : flag error 
 d0  4 00  4 00 00  0  2 15  return; *PDREG=0*
*
*	Subroutine to primary address cycle
*	Address in TCNT, MS codes already set
*
 d1  8 5b  4 00 00  0  2 00  tst AS=AK=0, *SEt err code--PDREG=21*
 d2  8 5b  4 d2 04  0  3 01  Wait AS=AK=0, reset timer, reset EG
 d3  8 07  7 00 00  0  e 20  Tst timer, enable adr. (8-bit TCNT) on bus, AS=1
 d4  a 58  7 d8 00  0  f 01  Tst .NOT. AK, adr. on bus (8-bit TCNT), set EG
 d5  a 07  7 d4 00  0  a 00  Tst timer, adr. on bus (8-bit TCNT), Br AK.NE.1
 d6  8 00  4 00 00  0  0 00  Good return, return PDREG=0
 d7  4 58  4 00 00  0  3 01  Tst .NOT.AK, reset EG, return
 d8  c 00  4 e2 00  0  0 00  Error. JMP to e2: flag error
*
*	Subroutine to perform FASTBUS Writes for secondary addressing
*	Put in P.Jennewein bug fix line db...set FBOUT = 1 ans DS = 1
*	in same instruction
*
 d9  8 19  4 00 00  0  2 00  Tst DK=1
 da  a 19  4 da 04  0  6 00  Reset timer, tst DK=1
 db  8 07  4 00 00  0  e 10  Tst timer, DS=1, enable FBOUT
 dc  a 59  5 df 00  0  a 00  Tst DK.NE.1, Br on timer, enable FBOUT
 dd  a 07  5 dc 00  0  a 00  Tst timer, Br DK.NE.1
 de  8 00  5 00 00  0  a 00  Good write
 df  8 60  5 00 00  0  a 00  Tst SS.NE.0
 e0  a 00  4 e2 00  0  0 00  If SS.NE.0, JMP e2: flag error
 e1  4 00  4 02 00  0  2 15  Good return, clr protocol bits
* 
*	Error identify sequence
*	1st decode SS response
*
 e2  8 21  4 01 08  0  2 00 	TCNT=1, test SS=1
 e3  a 00  4 e9 00  0  2 00 	Br if SS=1
 e4  8 26  4 06 08  0  2 00	TCNT=6, test SS=6
 e5  a 00  4 e9 00  0  2 00 	Br if SS=6
 e6  8 27  4 07 08  0  2 00 	TCNT=7, test SS=7
 e7  a 00  4 e9 00  0  2 00 	Br if SS=7
 e8  8 00  4 ff 08  0  2 00 	TCNT=ff undefined SS code
*
*	Write code 0xefefefef into memory buffer...fast read error header
*	NB the last two pipes make the correct size of error block in data
*
 e9  8 00  4 ef 01  0  0 00	32-bit REG(0) = 0xef
 ea  8 00  4 ef 01  1  0 00	32-bit REG(1) = 0xef
 eb  8 00  4 ef 01  2  0 00	32-bit REG(2) = 0xef
 ec  8 00  4 ef 05  3  0 00	32-bit REG(3) = 0xef, reset timer
 ed  8 45  4 00 00  4  0 00	PIPE data to memory. Test not timer
 ee  a 45  4 ee 00  0  0 00	Wait for pipe
 ef  8 00  c 00 05  0  0 00	32-bit REG(0) = TCNT, ie SS code, reset timer
 f0  8 00  c 00 41  1  0 00	32-bit REG(1) = NREG, address line at failure
 f1  8 00  4 30 01  2  0 00	32-bit REG(2) = 30 = PDREG
 f2  8 45  4 00 00  4  0 00	PIPE data to memory, test timer
 f3  a 45  4 f3 00  0  0 00	Wait for timer, ie allow previous pipe
 f4  8 00  4 00 04  4  0 00	PIPE data to memory, reset timer
 f5  8 45  4 00 00  0  0 00	test timer
 f6  a 45  4 f6 00  0  0 00	Wait for timer, ie allow previous pipe
 f7  8 00  4 30 02  4  0 00	PIPE data to memory, set PDREG=30 fast error
 f8  c 00  4 b0 00  0  0 00	jmp read loop attempt next adc or scaler
*
*	Subroutine zero 32-BIT REG, TCNT, PROTOCOL Bits before next module
*
 fb  8 00  4 00 00  0  2 7f  
 fc  8 00  4 00 09  0  0 3f  
 fd  8 00  4 00 01  1  1 3f  
 fe  8 00  4 00 01  2  2 bf  
