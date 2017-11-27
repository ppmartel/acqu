##----------------------------------------------------------------------------
##                      Acqu++
##              Data Acquisition and Analysis
##                      for
##              Nuclear Physics Experiments
##
##              C/C++ UNIX-Based DAQ
##              supported on PPC/lynxos, PC/linux
##              Preferred compiler g++
##
##                  *** catchTCS.d ***
##--Description
##      Parameter data file
##      Specify setup files and other stuff for Catch TCS setup
##
##--Author      JRM Annand      24th Oct 2002
##--Update      JRM Annand
##
##--------------------------------------------------------------------------
###            	MapSize  Added Baya
CatchMapSize:	0x1000
#
# Binary file to program the TCS FPGA (uncomment only one file here!)
#
# a) Files for use with V2 Controller:
# TCSFile: ./data/FPGA/tcs_cntr_204_3k.rbt
# TCSFile: ./data/FPGA/tcs_cntr_206_3k.rbt
# b) Files for use with V3 Controller:
TCSFile: ./data/FPGA/tcs_cntr_206_1k.rbt
#
# Now give the register address offsets and values to write for general
# setup of the catch TDC (values in hex)
# id is an optional parameter...do not use it here
# 	   	offset	value	id
#	TCS stop
CatchWrt:	0024	0000
#	Reset FE
CatchWrt:	0028	18800
#	Receiver 0
CatchWrt:	0028	80080
CatchWrt:	0028	d0001
#	Receiver 1
CatchWrt:	0028	80480
CatchWrt:	0028	d0101
#	Receiver 2
CatchWrt:	0028	80880
CatchWrt:	0028	d0201
#	Receiver 3
CatchWrt:	0028	80c80
CatchWrt:	0028	d0301
#	Receiver 4
CatchWrt:	0028	81080
CatchWrt:	0028	d0401
#	Receiver 5
CatchWrt:	0028	81480
CatchWrt:	0028	d0501
#	Receiver 6
CatchWrt:	0028	81880
CatchWrt:	0028	d0601
#	Receiver 7
CatchWrt:	0028	81c80
CatchWrt:	0028	d0701
#	Receiver 8
CatchWrt:	0028	82080
CatchWrt:	0028	d0801
#	Receiver 9
CatchWrt:	0028	82480
CatchWrt:	0028	d0901
#	Receiver 10
CatchWrt:	0028	82880
CatchWrt:	0028	d0a01
#	Receiver 11
CatchWrt:	0028	82880
CatchWrt:	0028	d0b01
#	Receiver 12
CatchWrt:	0028	82c80
CatchWrt:	0028	d0c01
#	Receiver 13
CatchWrt:	0028	83080
CatchWrt:	0028	d0d01
#	Receiver 14
CatchWrt:	0028	83480
CatchWrt:	0028	d0e01
#	Receiver 15
CatchWrt:	0028	83880
CatchWrt:	0028	d0f01
#	Receiver 16
CatchWrt:	0028	83c80
CatchWrt:	0028	d1001
#	Receiver 17
CatchWrt:	0028	84080
CatchWrt:	0028	d1101
#	Reset FE
CatchWrt:	0028	10800
#
#	Configure Controller
#	deadtime
CatchWrt:	0064	0010
CatchWrt:	0084	0004
CatchWrt:	00a4	0640
#	start run
CatchWrt:	0044	fffff
CatchWrt:	0024	0101
#	End
