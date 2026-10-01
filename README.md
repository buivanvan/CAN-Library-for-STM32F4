# CAN-Library-for-STM32F4
CAN Library for STM32F4 using register

## Note: This is CAN Normal Library not CAN FD

## I use Logic Analyzer to show logic at CAN Tx pin
In this example, I send an array from 1 to 8 to id 0x123
<img width="2234" height="139" alt="image" src="https://github.com/user-attachments/assets/c6ab1f0d-c146-4e4a-8461-4f4cf18f5e66" />

## How to use by KeilC
Step 1: Download the can.c and can.h files to your computer.

Step 2: Copy can.c file to User/Core/Src and can.h file to User/Core/Inc

Step 3: Add existing file to on KeilC and include library
```c
#include "can.h"
```

Step 4: Done, you can use library to send or receive data on CAN bus

## About function in this library
First step after include library, you must init CAN peripheral by

```c
CAN_Init(CANx, bitrate, alternative);
```
with CANx is CAN bus you use, alternative = 0 if you use normal pin and = 1 if you use alternative pin

Then, you create new message with data type is CAN_Message. Example:
```c
CAN_Message mes;
```

Finally, you can change data in this truct and send to bus. In this struct have four ojbects: id, len, extended and 1 array data 8 byte.
"id" is identifier you want to send data to, "len" is data length, "extended" = 0 if id is standard 11 bit, = 1 if extended id 29bit. Because this is CAN Normal, you only can send maximum of 8 bytes data at a time. 

After you fill data in mes, you can send data by CAN_Send(CANx, &message) function. Example: 
```c
CAN_Send(CAN1, &mes);
```

And you can receive data by CAN_Receive(CANx, &message) function. Example:
```c
if(CAN_Receive(CAN1, &mes)) {
// code }
```

## Small Example 
In this example, I use 1 board STM32F407VET6 act as sender and 1 board STM32VGT6 act as receiver. The result is picture on the top.
### Board sender
```c
#include "can.h"
CAN_Message mes_rx;

int main(void)
{
	CAN_Init(CAN1, 500000, 1);
  mes.id = 0x123;
	mes.len = 8;
  mes.extended = 0;

	mes.data[0] = 1;
	mes.data[1] = 2;
	mes.data[2] = 3;
	mes.data[3] = 4;
	mes.data[4] = 5;
	mes.data[5] = 6;
	mes.data[6] = 7;
	mes.data[7] = 8;

  CAN_Send(CAN1, &mes_tx);
  while(1)
  {
  }
}
```

### Board Receiver
```c
#include "can.h"
CAN_Message mes_rx;

int main(void)
{
  while(1)
  {
    //pulling to receive data
    if (CAN_Receive(CAN1, &mes_rx))
    {
    }
  }
}
