
set traceFile trace

#we use some itcl :-)
package require Itcl

#load the avr-simulator package
load ../../src/simulavr.so
puts "simulavr loaded"

#now start external generic gui server 
exec wish ../../tests/gui.tcl &

#start the trace output to given filename
StartTrace $traceFile

#start the user interface client 
set ui [new_UserInterface 7777 ]

#create new device
set dev1 [new_AvrDevice_at90s4433]

#load elf file to the device 
AvrDevice_Load $dev1 "./anacomp"

#set the clock cycle time [ns]
AvrDevice_SetClockFreq $dev1 250

#systemclock must know that this device will be stepped from application
$systemClock Add $dev1

#also the gui updates after each cycle
$systemClock AddAsyncMember $ui


#create some external pins
ExtAnalogPin pain0 0 $ui "ain0" ".x"
ExtAnalogPin pain1 0 $ui "ain1" ".x"
ExtPin epb $Pin_TRISTATE $ui "->BO" ".x"


#create some nets which connect the pins 
Net ain0
ain0 Add pain0
ain0 Add [AvrDevice_GetPin $dev1 "D6"]

Net ain1
ain1 Add pain1
ain1 Add [AvrDevice_GetPin $dev1 "D7"]


Net portb
portb Add epb
portb Add [AvrDevice_GetPin $dev1 "B0"]

#exec xterm -e tail -f $traceFile &

puts "Simulation runs endless, please press CTRL-C to abort"

exec ddd --debugger avr-gdb --command checkdebug.gdb &

#Now start the gdb-server inside the simulav.so lib.
gdb_interact $dev1 1212 0
