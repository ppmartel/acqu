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
##                  *** catchTDC.d ***
##--Description
##      Parameter data file
##      Specify setup files and other stuff for Catch TDC setup
##
##--Author      JRM Annand      24th Oct 2002
##--Update      JRM Annand
##
##--------------------------------------------------------------------------
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
# F1 TDC chip setup file
#
#F1File: ./data/FPGA/reference-v4.f1
# Catch triggers on leading edge:
F1File: ./data/FPGA/reference-v4_win20000.f1 
#
# Catch triggers on trailing edge:
#F1File: ./data/FPGA/reference-v4_lat10000_tr.f1
#
#
#       All values decimal next line
#          Used Chan      Multihits       Ref chan        Offset	bits
CatchParm: 128            2               0               2048		12
#
# Now give the register address offsets and values to write for general
# setup of the catch TDC (values in hex)
# id is an optional parameter...set = 1 if you wish to add catchid to value
#		offset	value	id
CatchWrt:	0800	0400
CatchWrt:	1448	0000	1
CatchWrt:	08d0	1000	1
CatchWrt:	1400	000d
CatchWrt:	1804	0043
#	Next line specifies break off register setup (to load F1) then resume
CatchWrt:	-1	0
#	sparce data mode (standard)
CatchWrt:	1004	0020
CatchWrt:	1448	0400	1
#	merge FPGA registers
CatchWrt:	2080	1000
CatchWrt:	2084	1000
CatchWrt:	2088	1000
CatchWrt:	208c	1000
#
CatchWrt:	2480	1004
CatchWrt:	2484	1004
CatchWrt:	2488	1004
CatchWrt:	248c	1004
#
CatchWrt:	2880	1008
CatchWrt:	2884	1008
CatchWrt:	2888	1008
CatchWrt:	288c	1008
#
CatchWrt:	2c80	100c
CatchWrt:	2c84	100c
CatchWrt:	2c88	100c
CatchWrt:	2c8c	100c
#
#	end of register programing
#
