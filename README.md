# Communication-simulator-for-automotive-ECU-based-on-STM32
### Project Design Description

1. **Foundation and Hardware Selection**
   - The project is based on RT-Thread and its peripheral model, with the hardware being STM32.
   - The purpose of this approach is to leverage comprehensive open-source resources to build a robust software and hardware environment.

2. **Key Functional Points**
   - **a. Simulation of an Automotive Body Network Node**
     - Support for a complete UDSonCAN protocol stack.
     - Support for a DoIP (Diagnostic over Internet Protocol) protocol stack.
   - **b. Ethernet to CAN Gateway Routing**
     - Implement a gateway that routes data between Ethernet and CAN networks.