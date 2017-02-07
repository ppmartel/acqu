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
##                  *** catchScaler.d ***
##--Description
##      Parameter data file
##      Specify setup files and other stuff for Catch TDC setup
##
##--Author      JRM Annand      24th Oct 2002
##--Update      JRM Annand
##
###--------------------------------------------------------------------------
###            	MapSize  Added Baya
CatchMapSize:	0x10000
##
# Binary file to program the CATCH FPGA's
#
FPGAFile: ./data/FPGA/catch-2003_7.hex
#
# TCS receiver file
# 
# a) for use with tcs_cntr_204_*
# TCSRecFile: ./data/FPGA/tcs_rec108.hex
#
# b) for use with tcs_cntr_206_*
TCSRecFile: ./data/FPGA/tcs_rec201.hex
#
#       All values decimal next line
#	    Used Chan
CatchParm:  128
#
# Now give the register address offsets and values to write for general
# setup of the catch scaler (values in hex)
# id is an optional parameter...set = 1 if you wish to add catchid to value
#		offset	value	id
CatchWrt:	0800	0400
CatchWrt:	0200	0001
CatchWrt:	1448	6000	1
CatchWrt:	08d0	2000	1
CatchWrt:	1440	1000
CatchWrt:	1004	0010
#	merge FPGA registers
CatchWrt:	2000	0000
CatchWrt:	2400	0000
CatchWrt:	2800	0000
CatchWrt:	2c00	0000
#	merge ADC mode
CatchWrt:	2050	0000
CatchWrt:	2450	0000
CatchWrt:	2850	0000
CatchWrt:	2c50	0000
#	no TCS dummy
CatchWrt:	1400	000d
#	reset spy buffer
CatchWrt:	1804	0043
#	Geog id's
CatchWrt:	2080	0000
CatchWrt:	2480	0004
CatchWrt:	2880	0008
CatchWrt:	2c80	000c
#
#	end of register programing

