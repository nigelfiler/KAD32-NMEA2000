# KAD32 Engine Data on NMEA2000

## Overview
This project aims to send **KAD32 engine data** over the **NMEA2000 network** to a chart plotter, enabling real-time monitoring of key engine metrics. 

The primary data points of interest are:
- **Boost Pressure**
- **Engine Temperature**

However, after further research, I expanded the design to include additional data points:
- **RPM**
- **Coolant Pressure**
- **Alternator Voltage**


## Hardware and Libraries
- **Microcontroller:** Arduino Due  
    - Chosen for its **CAN bus support**, which is essential for NMEA2000 communication.
    - Designed a PCB to connect the Arduino to the various sensors
 
- **Library:** [Timo Lappalainen's NMEA2000 library](https://github.com/ttlappalainen/NMEA2000)  
    - This library provides the foundation for NMEA2000 data transmission, making it a great starting point for this project. I would like to thank Timo for creating his library and extensively documenting it. Without this I probably would not have succeeded.

## Implementation
1. **Data Acquisition:** The Arduino Due collects engine data from the sensors installed on the KAD32 Engine
2. **NMEA2000 Transmission:** The data is formatted and sent over the N2K network via CAN bus.
3. **Chart Plotter Display:** The chart plotter receives and displays the engine metrics in real time.

Sensors used:

**Boost Pressure** uses a 0-30psi (0.5-4.5v) pressure transducer, this is mounted in the engine inlet manifold where there is a blanking plug.

**Coolant Temperature** uses a standard Engine temperature sender mounted in the thermostat housing. The KAD32 has a spare location for this. I used an earth insulated type. M18x1.5

Temperature sender was calibrated using the Steinhart-Hart equation. 
Calibrated on the bench at 7c, 21c, 68c.
Code uses EMWA (Moving average filter) - this provides a stable temperature reading, without it the temperature reading was slighly erratic.

## Screenshots
Here is my chart plotter receiving data from my sensors: [Chart Plotter](./Chartplotter.jpeg)


## Project Status

Project is working reliably for more than 1 year for **Coolant Temperature** and **Engine Boost**, but the other functionality I wanted to add is not complete. 

This includes; Coolant Pressure, Voltage, RPM.

**RPM** had a design fault. The code is complete but untested.

**Coolant Pressure** should work but was not coded and I did not decide on a suitable location for the sensor. 
Ideally a dual temperature and pressure sensor that mounts in the thermostat housing would be ideal, but I did not find a suitable one yet.

## Future Enhancements
- Support for additional engine parameters.
- Refining the code for efficiency and accuracy.
- Remove blocking delay

