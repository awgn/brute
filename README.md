# Welcome to Brute HomePage!

## What is BRUTE

BRUTE is the acronym of Brawny and RobUstT Traffic Engine. It is a user space application running on the top 
Linux operating system designed to produce high load of customizable network traffic. The design of the software 
has been driven to achieve high precision and performance in the traffic generation. 
Currently Brute has been tested on AI32 Intel Pentium, PA-RISC 2.0 (PA8500) and AMD64 Athlon architectures, 
over both fast and gigabit ethernet.


## Why BRUTE

BRUTE has been developed as part of the MIUR project EURO "University Experiment of an Open Router", 
funded by the Italian Ministry for Education, University and Research. Its purpose is to provide a high performance 
and high precision traffic generator for common low-end PCs running Linux operating system. 
BRUTE has been presented at the "International Symposium on Performance Evaluation of Computer and Telecommunication Systems" 
(Philadelphia, July 24-28, 2005 SPECTS05) with the paper entitled 
_"BRUTE: A High Performance and Extensible Traffic Generator"_.


## Features

* User space application running on the top of Linux (kernel 2.6.x).
* Extensible design with modular architecture.
* High performance, up to 1.4 Million fps (64bytes length) over a Gigabit Ethernet on common P4.
* High precision in the rate generation, up to three order of magnitudes more accurate than paired softwares (rude, mgen).
* vsyscall support and soft-realtime scheduling policy available from the linux kernel 2.6.
* PF_PACKET and PF_INET sockets to allow address randomization at different layers.
* IPv4 and IPv6 compliant.
* Traffic modules implemented:
    * CBR (costant bit rate)
    * MULTICBR (multiple cbr flows)
    * CIDT (constant inter-departure time)
    * POISSON (exponential inter-departure time)
    * PAB (Poisson Arrival of Burst)
    * CBR-EXP/OFF-EXP (voip)
    * RTCP SR (send-report message to measure RTT)
    * TRIMODAL (trimodal ethernet distribution)

