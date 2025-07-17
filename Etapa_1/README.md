# **BioSmartCooler - Recipiente Inteligente para Transporte de Órgãos**  

**Institution**: Instituto Hardware BR-DF  
**Course**: Technological Residency in Embedded Systems  
**Authors**: **Giovana Ferreira Santos** e **Filipe Alves de Sousa**  
**Location**: Brasília-DF  
**Date**: Julho de 2025  

---

## **About This Repository**  
This repository contains the development of **BioSmartCooler**, an intelligent thermal container designed to modernize organ transportation by incorporating embedded electronic systems and IoT (Internet of Things) principles. The project focuses on real-time monitoring of critical parameters such as temperature, humidity, light exposure, and physical impacts, ensuring organ viability during transport.  

---

## **Project Overview**  

### **Problem Description**  
The conventional method of organ transportation relies on passive thermal boxes with ice, lacking real-time monitoring and alert systems. This can lead to organ degradation due to uncontrolled environmental conditions, physical shocks, or improper handling.  

### **Solution Objective**  
Develop a smart thermal container prototype that:  
- Monitors **temperature, humidity, light, and impacts** in real time.  
- Provides **visual and auditory alerts** in case of anomalies.  
- Stores or transmits monitored data for remote tracking.  

---

## **Functional Requirements (RF)**  

| Code | Functional Requirement |  
|------|------------------------|  
| RF01 | Measure and display internal temperature. |  
| RF02 | Measure and display internal humidity. |  
| RF03 | Detect and log sudden movements or falls. |  
| RF04 | Detect lid opening (light exposure change). |  
| RF05 | Display data on an external screen. |  
| RF06 | Emit auditory alerts (buzzer) in critical situations. |  
| RF07 | Autonomous power supply via battery or power bank. |  

---

## **Non-Functional Requirements (RNF)**  

| Code | Non-Functional Requirement |  
|------|---------------------------|  
| RNF01 | Operate for at least 4 hours without recharge. |  
| RNF02 | User-friendly interface for medical personnel. |  
| RNF03 | High-precision sensors with fast response time. |  
| RNF04 | Secure component fixation to withstand movement. |  
| RNF05 | Adequate thermal insulation (simulated). |  

---

## **Materials List**  

| Item | Quantity | Description |  
|------|----------|-------------|  
| Thermal insulation box (8-12L) | 1 | Base container with insulation |  
| BitDogLab (Raspberry Pi Pico W) | 1 | Microcontroller with integrated peripherals (OLED, buzzer, buttons, RGB LED) |  
| Temperature & Pressure Sensor (BMP280) | 1 | Embedded in BitDogLab |  
| Humidity & Temperature Sensor (SHT31) | 1 | Embedded in BitDogLab |  
| Light Sensor (BH1750) | 1 | Embedded in BitDogLab |  
| Accelerometer (MPU6050) | 1 | Embedded in BitDogLab |  
| 5V Cooling Fan | 1 | Optional active cooling simulation |  
| Power Bank / Li-ion Battery | 1 | Portable power supply |  
| Jumper Wires & Mounting Materials | As needed | For internal connections |  

---

## **Project Structure**  

### **Key Features**  
✅ **Real-Time Monitoring**: Sensors track temperature, humidity, light, and impacts.  
✅ **Alert System**: Buzzer and LED alerts for critical deviations.  
✅ **Data Logging**: Stores or transmits data for remote supervision.  
✅ **Autonomous Operation**: Battery-powered for portability.  

### **System Workflow**  
1. **Sensors** collect environmental and motion data.  
2. **BitDogLab (RP2040)** processes data and triggers alerts if needed.  
3. **OLED Display** shows real-time parameters.  
4. **Wi-Fi Connectivity** (optional) enables remote monitoring.  

---

## **Images & Schematics**  

### **Prototype Diagram**  
![BioSmartCooler Diagram](./assets/prototype_diagram.jpg)  

### **Hardware Setup**  
![BitDogLab Integration](./assets/hardware_setup.jpg)  

---

## **References**  
- **Organ Procurement and Transplantation Network (OPTN)** – Guidelines on organ transport.  
- **World Health Organization (WHO)** – Global standards for medical logistics.  
- **Al-Fuqaha, A. et al. (2015)** – IoT enabling technologies.  
- **Gubbi, J. et al. (2013)** – IoT applications in healthcare.  
- **Tanenbaum, A. & Wetherall, D. (2021)** – Computer networks for embedded systems.  

---

## **License**  
This project is licensed under the **[MIT License](LICENSE)**.  

---  
