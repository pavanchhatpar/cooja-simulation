# RPL Network Authentication
RPL network simulation using 6lowpan for a hospital, with authentication added

## Features
 - RPL network using 6lowpan
 - Authentication scheme that makes Brute Force and MITM attacks very tough
 - COAP REST Engine to allow collection of data from 6lowpan network via applications on the Internet

## Run an example
 - Open `multi-room-contiki3.csc`
 - Make sure `Serial Socket {Server}` is on
 - On a terminal inside `contiki-3.0/tools` run the following commands
   ```
   make tunslip6   # only if you haven't done this ever before, needed only once
   sudo ./tunslip6 -a 127.0.0.1 aaaa::1/64   #needs to be run everytime the simulation is run
   ```
 - Now Start the simulation, since there are a lot of motes, ample amount of time will be taken to generate routes and start exchanging data
 - COAP requests (via Mozilla Firefox) can be made to any of the sensor motes to get relevant information
 
## Points to remember
The simulated environment has some restrictions
 - Mote 1 should be `rpl border router`
 - Motes 2, 9, 16, 23 and so on need to necessarily be `RFID servers`
These restrictions are required to make sure all sensors can properly identify the `IP Address` of their respective `RFID nodes`
