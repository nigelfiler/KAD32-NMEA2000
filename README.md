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

## Why This Project?
I'm not inventing anything new—many people have created similar solutions. My goal is to **adapt existing technologies** to meet my needs and build something **professionally designed** and reliable.

## Hardware and Libraries
- **Microcontroller:** Arduino Due  
    - Chosen for its **CAN bus support**, which is essential for NMEA2000 communication.
- **Library:** [Timo Lappalainen's NMEA2000 library](https://github.com/ttlappalainen/NMEA2000)  
    - This library provides the foundation for NMEA2000 data transmission, making it a great starting point for this project. I would like to thank Timo for creating his library and extensively documenting it. Without this I probably would not have succeeded.

## Implementation
1. **Data Acquisition:** The Arduino Due collects engine data from the KAD32 sensors.
2. **NMEA2000 Transmission:** The data is formatted and sent over the N2K network via CAN bus.
3. **Chart Plotter Display:** The chart plotter receives and displays the engine metrics in real time.

Sensors used:

Boost pressure

## Screenshots
Here is my chart plotter receiving data from my sensors:  
*(Add screenshots or photos here)*

## Future Enhancements
- Support for additional engine parameters.
- Improved board design for durability and reliability.
- Refining the code for efficiency and accuracy.
