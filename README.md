* Data Communication Project – Error Detection & Corrupted Channel Simulation
This project simulates a complete Data Communication System using three applications:



Client 1 (Sender)


Server (Corruptor / Middle Agent)


Client 2 (Receiver & Error Detector)


The goal is to implement multiple error detection techniques, simulate data corruption through a noisy channel, and verify whether received data is Correct or Corrupted.


* Project Overview
.1. Client 1 – Sender


Accepts input text from the user.


User selects an error detection method.


Computes the control information for the chosen method.


Builds a packet:


DATA | METHOD | CONTROL



Sends the packet to the server over TCP.


Supported Detection Methods:


Parity Bit (Even)


2D Parity


CRC16 (CRC-CCITT)


Hamming (Parity nibble)


Internet Checksum (16-bit)



.2. Server – Corruptor


Receives packet from Client 1.


Extracts DATA, METHOD, CONTROL.


Applies random corruption to DATA only.


Builds a new packet:


CORRUPTED_DATA | METHOD | CONTROL



Waits for Client 2 and sends the corrupted packet.


Error Injection Methods:


Bit Flip


Character Substitution


Character Deletion


Character Insertion


Character Swapping


Multiple Bit Flips


Burst Error (3–8 chars)



.3. Client 2 – Receiver


Connects to the server.


Receives the corrupted packet.


Recomputes CONTROL based on the received DATA.


Compares values:


If computed == received → DATA CORRECT
Else → DATA CORRUPTED



Prints full status and details.



** Project Structure
data_com_project/
│-- client1/
│-- client2/
│-- server/
│-- common/
│-- .gitignore


> How to Run (Windows)
Start the Server
cd server
gcc server.c -o server.exe -lws2_32
./server.exe

Run Client 1
cd client1
gcc client1.c -o client1.exe -lws2_32
./client1.exe

Run Client 2
cd client2
gcc client2.c -o client2.exe -lws2_32
./client2.exe


* Example Output
Client 1 Packet
HELLO|CRC16|49D6

Server After Corruption
Applying error type: 4
Corrupted DATA: HEaLLO
Packet: HEaLLO|CRC16|49D6

Client 2 Result
DATA: HEaLLO
METHOD: CRC16
Sent CONTROL: 49D6
Computed CONTROL: 92B1
Status: DATA CORRUPTED


-> Author
Mohammed i t Alfara
Computer Engineering Student – Karabük University
