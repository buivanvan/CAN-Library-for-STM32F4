#ifndef CAN_H
#define CAN_H

#include "stm32f4xx.h"
#include <stdint.h>

#define CAN_SAMPLE_POINT_TARGET    875
#define CAN_TIMEOUT    100000U

typedef enum
{
    CAN_STATUS_OK = 0,
    CAN_STATUS_ERROR,
    CAN_STATUS_TIMEOUT,
    CAN_STATUS_BUS_OFF,
    CAN_STATUS_ERROR_PASSIVE,
    CAN_STATUS_ERROR_WARNING
} CAN_Status;

typedef struct
{
    uint32_t id;
    uint8_t data[8];
    uint8_t len;
    uint8_t extended;
} CAN_Message;

uint32_t CAN_GetSYSCLK(void);
uint32_t CAN_GetHCLK(void);
uint32_t CAN_GetPCLK1(void);

uint32_t CAN_GetBitRate(CAN_TypeDef *CANx);
uint32_t CAN_GetSamplePoint(CAN_TypeDef *CANx);

uint8_t CAN_ConfigBitRate(CAN_TypeDef *CANx, uint32_t bitrate);
uint8_t CAN_FilterConfig(CAN_TypeDef *CANx, uint8_t bank, uint32_t id, uint32_t mask, uint8_t extended);
uint8_t CAN_Init(CAN_TypeDef *CANx, uint32_t bitrate, uint8_t alternative);
CAN_Status CAN_GetStatus(CAN_TypeDef *CANx);

uint8_t CAN_GetTxErrorCount(CAN_TypeDef *CANx);
uint8_t CAN_GetRxErrorCount(CAN_TypeDef *CANx);

void CAN_AbortTx(CAN_TypeDef *CANx, uint8_t mailbox);
uint8_t CAN_Send(CAN_TypeDef *CANx, CAN_Message *message);
uint8_t CAN_Receive(CAN_TypeDef *CANx, CAN_Message *message);

#endif