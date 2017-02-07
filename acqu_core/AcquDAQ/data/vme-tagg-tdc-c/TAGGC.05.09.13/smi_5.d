************************************************************************
*			  Kelvin Laboratory
*			University of Glasgow
*			*******smi_5.d*******
*
* 	JRM Annand 	14th Apr 1992
* 	JRM Annand 	19th Jun 1992	Update error codes
*	JRM Annand	 3rd Jun 1993	Change to new file format, install
*					P.Jennewein bug fix and error handler
*
*	"Slow" Fastbus Readout code as used in PIP/TOF/TAGGER experiments
*	until Apr 1993
* 	This code has hardware block read at line bc
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
 33  c 00  4 85 00  0  0 00		Call block read at 85
* 
*	Error identify sequence
*	1st decode SS response
*
 50  8 21  4 01 08  0  2 00 	TCNT=1, test SS=1
 51  a 00  4 5f 00  0  2 00 	Br if SS=1
 52  8 22  4 02 08  0  2 00 	TCNT=2, test SS=2
 53  a 00  4 5f 00  0  2 00 	Br if SS=2
 54  8 23  4 03 08  0  2 00 	TCNT=3, test SS=3
 55  a 00  4 5f 00  0  2 00 	Br if SS=3
 56  8 24  4 04 08  0  2 00 	TCNT=4, test SS=4
 57  a 00  4 5f 00  0  2 00 	Br if SS=4
 58  8 25  4 05 08  0  2 00 	TCNT=5, test SS=5
 59  a 00  4 5f 00  0  2 00 	Br if SS=5
 5a  8 26  4 06 08  0  2 00	TCNT=6, test SS=6
 5b  a 00  4 5f 00  0  2 00 	Br if SS=6
 5c  8 27  4 07 08  0  2 00 	TCNT=7, test SS=7
 5d  a 00  4 5f 00  0  2 00 	Br if SS=7
 5e  8 00  4 ff 08  0  2 00 	TCNT=ff undefined SS code
*
*	Write code 0xefefefef into memory buffer...read error header
*	NB the last two pipes make the correct size of error block in data
*
 5f  8 00  4 ef 01  0  0 00	32-bit REG(0) = 0xef
 60  8 00  4 ef 01  1  0 00	32-bit REG(1) = 0xef
 61  8 00  4 ef 01  2  0 00	32-bit REG(2) = 0xef
 62  8 00  4 ef 05  3  0 00	32-bit REG(3) = 0xef, reset timer
 63  8 45  4 00 00  4  0 00	PIPE data to memory. Test not timer
 64  a 45  4 64 00  0  0 00	Wait for pipe
 65  8 00  c 00 05  0  0 00	32-bit REG(0) = TCNT, ie SS code, reset timer
 66  8 00  4 ff 01  1  0 00	32-bit REG(1) = ff Not used for slow readout
 67  8 00  c 00 03  2  0 00	32-bit REG(2) = PDREG code denotes error pt.
 68  8 45  4 00 00  4  0 00	PIPE data to memory, test timer
 69  a 45  4 69 00  0  0 00	Wait for timer, ie allow previous pipe
 6a  8 00  4 00 04  4  0 00	PIPE data to memory, reset timer
 6b  8 45  4 00 00  0  0 00	test timer
 6c  a 45  4 6c 00  0  0 00	Wait for timer, ie allow previous pipe
 6d  8 00  4 40 02  4  0 00	PIPE data to memory, set PDREG=40 slow error
 6e  c 00  4 00 00  0  0 00	jump back to idle loop
*
* 	start of block read sequence
*
 85  d 00  4 fb 00  0  0 00		call fb, clear bus
 86  8 00  0 00 08  1  0 00
 87  8 00  4 00 00  0  2 3f
 88  d 00  4 eb 00  0  0 00		call eb primary addr
 89  8 00  4 00 08  0  0 00
 8a  8 00  0 00 01  0  0 00
 8b  8 00  4 22 02  0  2 1f		load PDREG 22, sec addr wrt diag
 8c  d 00  4 d9 00  0  6 02		call d9 secondary addr wrt
 8d  d 00  4 bc 00  0  2 1f		call bc hardware block read
 8e  c 00  4 00 00  0  0 00		return to idle loop
* 	end of block read sequence
*
*
* 	start of hardware block read
*
 bc  8 19  4 23 02  8  6 09		PDREG = 23 start of hard block
 bd  a 19  4 bd 00  8  3 04
 be  8 00  4 00 00  8  7 04
 bf  8 6f  4 00 00  8  2 00
 c0  a 6f  4 c0 00  8  2 00
* c1  8 00  4 00 00  0  5 80
 c1  8 20  4 00 00  0  2 00		tst SS=0
 c2  a 22  4 c6 00  0  2 00		tst SS=2 jmp c6 if SS=0
 c3  a 61  4 c7 00  0  2 00		tst SS!=1 jmp c7 if SS=2 (end of read)
 c4  a 00  4 c6 00  0  2 00		jmp c6 if SS!=1
 c5  c 00  4 bc 00  0  3 04		loop back to start
*
 c6  c 00  4 50 00  0  2 07		Error, go to error diagnostic
 c7  8 00  4 00 02  0  2 07		PDREG=0 good return
 c8  4 00  5 00 00  0  3 04
*
* 	general fastbus write, P.Jennewein bug fix line db
*
 d9  8 19  4 00 00  0  2 00
 da  a 19  4 da 04  0  6 00
 db  8 07  4 00 00  0  e 10		Tst timer, enable FBOUT, set DS
 dc  a 59  5 df 00  0  a 00
 dd  a 07  5 dc 00  0  a 00
 de  8 00  5 00 00  0  a 00		Don't reset PDREG here
 df  8 60  5 00 00  0  a 00
 e0  a 00  4 50 00  0  0 00		Return to idle if SS!=0 error
 e1  4 00  4 00 02  0  2 15		But reset it here on a good return
*
*	 end of fastbus write
*
 e2  8 19  4 00 00  0  2 00
 e3  a 19  4 e3 04  0  6 00
 e4  8 07  4 00 00  0  6 18
 e5  a 59  4 e8 00  0  2 00
 e6  a 07  4 e5 00  0  2 00
 e7  8 00  4 00 03  8  2 00
 e8  8 60  4 00 00  0  2 00
 e9  a 00  4 69 00  0  0 00
 ea  4 00  4 00 00  0  2 15
*
* 	start of primary addr sequence
*
 eb  8 5b  4 21 02  0  2 00		PDREG = 21 prim addr diagnostic
 ec  a 5b  4 ec 04  0  3 01
 ed  8 07  7 00 00  0  e 20
 ee  a 58  7 f1 00  0  f 01
 ef  a 07  7 ee 00  0  a 00
 f0  8 00  4 00 02  0  0 00
 f1  4 58  4 00 00  0  3 01
*
* 	end of primary addr sequence
*
 f2  8 07  5 1e 02  0  e 20
 f3  a 58  5 f6 00  0  e 00
 f4  a 07  5 f3 00  0  e 00
 f5  8 00  4 00 02  0  0 00
 f6  4 58  4 00 00  0  0 00
 f7  8 00  4 00 01  0  3 04
 f8  8 00  4 00 01  1  0 00
 f9  8 00  4 00 01  2  0 00
 fa  4 00  4 00 01  3  0 00
*
* 	clear bus and 32 bit reg
*
 fb  8 00  4 00 00  0  2 7f
 fc  8 00  4 00 09  0  0 3f
 fd  8 00  4 00 01  1  1 3f
 fe  8 00  4 00 01  2  2 bf
 ff  4 00  4 00 01  3  3 1f
*
