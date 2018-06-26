EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:switches
LIBS:relays
LIBS:motors
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:circus_agent
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L C8051F850-C-GU U?
U 1 1 5B327CEA
P 5500 3150
F 0 "U?" H 5750 4200 60  0000 C CNN
F 1 "C8051F850-C-GU" H 6150 2850 60  0000 C CNN
F 2 "" H 5500 3150 60  0001 C CNN
F 3 "" H 5500 3150 60  0001 C CNN
	1    5500 3150
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 5B327D75
P 4300 2700
F 0 "#PWR?" H 4300 2450 50  0001 C CNN
F 1 "GND" H 4300 2550 50  0000 C CNN
F 2 "" H 4300 2700 50  0001 C CNN
F 3 "" H 4300 2700 50  0001 C CNN
	1    4300 2700
	1    0    0    -1  
$EndComp
Wire Wire Line
	5500 2600 4300 2600
Wire Wire Line
	4300 2600 4300 2700
$Comp
L TSOP38438 U?
U 1 1 5B328025
P 7350 1950
F 0 "U?" H 7750 2400 60  0000 C CNN
F 1 "TSOP38438" H 7700 2000 60  0000 C CNN
F 2 "" H 7700 1900 60  0001 C CNN
F 3 "" H 7700 1900 60  0001 C CNN
	1    7350 1950
	1    0    0    -1  
$EndComp
$EndSCHEMATC
