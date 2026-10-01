#include "can.h"

static uint32_t CAN_GetAHBPrescaler(void)
{
    uint32_t hpre;

    hpre = (RCC->CFGR >> 4) & 0x0F;

    if (hpre < 8)
    {
        return 1;
    }

    return 1 << (hpre - 7);
}

static uint32_t CAN_GetAPB1Prescaler(void)
{
    uint32_t ppre1;

    ppre1 = (RCC->CFGR >> 10) & 0x07;

    if (ppre1 < 4)
    {
        return 1;
    }

    return 1 << (ppre1 - 3);
}

static uint32_t CAN_GetPLLSourceClock(void)
{
    if (RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC)
    {
        return 8000000;
    }

    return 16000000;
}

uint32_t CAN_GetSYSCLK(void)
{
    uint32_t sw;
    uint32_t pllm;
    uint32_t plln;
    uint32_t pllp;
    uint32_t pll_source;

    sw = (RCC->CFGR >> 2) & 0x03;

    switch (sw)
    {
        case 0:
            return 16000000;

        case 1:
            return 8000000;

        case 2:
            pll_source = CAN_GetPLLSourceClock();

            pllm = RCC->PLLCFGR & 0x3F;

            plln = (RCC->PLLCFGR >> 6) & 0x1FF;

            pllp = (RCC->PLLCFGR >> 16) & 0x03;

            pllp = (pllp * 2) + 2;

            if (pllm == 0 || pllp == 0)
            {
                return 0;
            }

            return ((pll_source / pllm) * plln) / pllp;

        default:
            return 0;
    }
}

uint32_t CAN_GetHCLK(void)
{
    return CAN_GetSYSCLK() / CAN_GetAHBPrescaler();
}

uint32_t CAN_GetPCLK1(void)
{
    return CAN_GetHCLK() / CAN_GetAPB1Prescaler();
}

uint8_t CAN_ConfigBitRate(CAN_TypeDef *CANx, uint32_t bitrate)
{
    uint32_t pclk1;
    uint32_t tq;
    uint32_t brp;
    uint32_t ts1;
    uint32_t ts2;

    uint32_t best_brp = 0;
    uint32_t best_ts1 = 0;
    uint32_t best_ts2 = 0;
    uint32_t best_error = 0xFFFFFFFF;

    pclk1 = CAN_GetPCLK1();

    if (pclk1 == 0 || bitrate == 0)
    {
        return 0;
    }

    for (tq = 8; tq <= 25; tq++)
    {
        if (pclk1 % (bitrate * tq) != 0)
        {
            continue;
        }

        brp = pclk1 / (bitrate * tq);

        if (brp < 1 || brp > 1024)
        {
            continue;
        }

        for (ts2 = 1; ts2 <= 8; ts2++)
        {
            ts1 = tq - 1 - ts2;

            if (ts1 < 1 || ts1 > 16)
            {
                continue;
            }

            uint32_t sample_point;
            uint32_t error;

            sample_point = ((1 + ts1) * 1000) / tq;

            if (sample_point > CAN_SAMPLE_POINT_TARGET)
            {
                error = sample_point - CAN_SAMPLE_POINT_TARGET;
            }
            else
            {
                error = CAN_SAMPLE_POINT_TARGET - sample_point;
            }

            if (error < best_error)
            {
                best_error = error;

                best_brp = brp;
                best_ts1 = ts1;
                best_ts2 = ts2;
            }
        }
    }

    if (best_brp == 0)
    {
        return 0;
    }

    CANx->BTR = ((best_brp - 1) << 0) |
                ((best_ts1 - 1) << 16) |
                ((best_ts2 - 1) << 20);

    return 1;
}

uint32_t CAN_GetBitRate(CAN_TypeDef *CANx)
{
    uint32_t brp;
    uint32_t ts1;
    uint32_t ts2;
    uint32_t tq;

    brp = (CANx->BTR & CAN_BTR_BRP) + 1;

    ts1 = ((CANx->BTR & CAN_BTR_TS1) >> 16) + 1;

    ts2 = ((CANx->BTR & CAN_BTR_TS2) >> 20) + 1;

    tq = 1 + ts1 + ts2;

    return CAN_GetPCLK1() / (brp * tq);
}

uint32_t CAN_GetSamplePoint(CAN_TypeDef *CANx)
{
    uint32_t ts1;
    uint32_t ts2;
    uint32_t tq;

    ts1 = ((CANx->BTR & CAN_BTR_TS1) >> 16) + 1;

    ts2 = ((CANx->BTR & CAN_BTR_TS2) >> 20) + 1;

    tq = 1 + ts1 + ts2;

    return ((1 + ts1) * 1000) / tq;
}

static void CAN1_GPIO_Init_PA(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    GPIOA->MODER &= ~((3U << (11 * 2)) |
                      (3U << (12 * 2)));

    GPIOA->MODER |= ((2U << (11 * 2)) |
                     (2U << (12 * 2)));

    GPIOA->OTYPER &= ~((1U << 11) |
                       (1U << 12));

    GPIOA->OSPEEDR |= ((3U << (11 * 2)) |
                       (3U << (12 * 2)));

    GPIOA->PUPDR &= ~((3U << (11 * 2)) |
                      (3U << (12 * 2)));

    GPIOA->AFR[1] &= ~((0xFU << 12) |
                       (0xFU << 16));

    GPIOA->AFR[1] |= ((9U << 12) |
                      (9U << 16));
}

static void CAN1_GPIO_Init_PD(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

    GPIOD->MODER &= ~((3U << (0 * 2)) |
                      (3U << (1 * 2)));

    GPIOD->MODER |= ((2U << (0 * 2)) |
                     (2U << (1 * 2)));

    GPIOD->OTYPER &= ~((1U << 0) |
                       (1U << 1));

    GPIOD->OSPEEDR |= ((3U << (0 * 2)) |
                       (3U << (1 * 2)));

    GPIOD->PUPDR &= ~((3U << (0 * 2)) |
                      (3U << (1 * 2)));

    GPIOD->AFR[0] &= ~((0xFU << 0) |
                       (0xFU << 4));

    GPIOD->AFR[0] |= ((9U << 0) |
                      (9U << 4));
}

static void CAN2_GPIO_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    GPIOB->MODER &= ~((3U << (12 * 2)) |
                      (3U << (13 * 2)));

    GPIOB->MODER |= ((2U << (12 * 2)) |
                     (2U << (13 * 2)));

    GPIOB->OTYPER &= ~((1U << 12) |
                       (1U << 13));

    GPIOB->OSPEEDR |= ((3U << (12 * 2)) |
                       (3U << (13 * 2)));

    GPIOB->PUPDR &= ~((3U << (12 * 2)) |
                      (3U << (13 * 2)));

    GPIOB->AFR[1] &= ~((0xFU << 16) |
                       (0xFU << 20));

    GPIOB->AFR[1] |= ((9U << 16) |
                      (9U << 20));
}

uint8_t CAN_Init(CAN_TypeDef *CANx, uint32_t bitrate, uint8_t alternative)
{
    if (CANx == CAN1)
    {
        RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;
				if(alternative == 0) CAN1_GPIO_Init_PA();
        else CAN1_GPIO_Init_PD();
    }
    else if (CANx == CAN2)
    {
        RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;
        RCC->APB1ENR |= RCC_APB1ENR_CAN2EN;
			
				CAN2_GPIO_Init();
    }
    else
    {
        return 0;
    }

    CANx->MCR |= CAN_MCR_INRQ;

    while ((CANx->MSR & CAN_MSR_INAK) == 0)
    {
    }

    CANx->MCR |= CAN_MCR_ABOM;
    CANx->MCR |= CAN_MCR_AWUM;

    CANx->MCR &= ~CAN_MCR_NART;
    CANx->MCR &= ~CAN_MCR_RFLM;
    CANx->MCR &= ~CAN_MCR_TXFP;

    if (CAN_ConfigBitRate(CANx, bitrate) == 0)
    {
        return 0;
    }

    CAN_FilterConfig(CAN1, 0, 0, 0, 0);

    CANx->MCR &= ~CAN_MCR_INRQ;

    while (CANx->MSR & CAN_MSR_INAK)
    {
    }

    return 1;
}

uint8_t CAN_FilterConfig(CAN_TypeDef *CANx, uint8_t bank, uint32_t id, uint32_t mask, uint8_t extended)
{
    uint32_t filter_id;
    uint32_t filter_mask;

    if (CANx != CAN1)
    {
        return 0;
    }

    if (bank > 13)
    {
        return 0;
    }

    if (extended)
    {
        if (id > 0x1FFFFFFF || mask > 0x1FFFFFFF)
        {
            return 0;
        }

        filter_id = (id << 3) | CAN_RI0R_IDE;
        filter_mask = (mask << 3) | CAN_RI0R_IDE;
    }
    else
    {
        if (id > 0x7FF || mask > 0x7FF)
        {
            return 0;
        }

        filter_id = id << 21;
        filter_mask = mask << 21;
    }

    CAN1->FMR |= CAN_FMR_FINIT;

    CAN1->FA1R &= ~(1U << bank);

    CAN1->FM1R &= ~(1U << bank);

    CAN1->FS1R |= (1U << bank);

    CAN1->FFA1R &= ~(1U << bank);

    CAN1->sFilterRegister[bank].FR1 = filter_id;
    CAN1->sFilterRegister[bank].FR2 = filter_mask;

    CAN1->FA1R |= (1U << bank);

    CAN1->FMR &= ~CAN_FMR_FINIT;

    return 1;
}

CAN_Status CAN_GetStatus(CAN_TypeDef *CANx)
{
    if (CANx->ESR & CAN_ESR_BOFF)
    {
        return CAN_STATUS_BUS_OFF;
    }

    if (CANx->ESR & CAN_ESR_EPVF)
    {
        return CAN_STATUS_ERROR_PASSIVE;
    }

    if (CANx->ESR & CAN_ESR_EWGF)
    {
        return CAN_STATUS_ERROR_WARNING;
    }

    return CAN_STATUS_OK;
}

uint8_t CAN_GetTxErrorCount(CAN_TypeDef *CANx)
{
    return (CANx->ESR >> 16) & 0xFF;
}

uint8_t CAN_GetRxErrorCount(CAN_TypeDef *CANx)
{
    return (CANx->ESR >> 24) & 0xFF;
}

void CAN_AbortTx(CAN_TypeDef *CANx, uint8_t mailbox)
{
    if (mailbox > 2)
    {
        return;
    }

    CANx->TSR |= CAN_TSR_ABRQ0 << mailbox;
}

uint8_t CAN_Send(CAN_TypeDef *CANx, CAN_Message *message)
{
    uint8_t mailbox;
    uint32_t timeout;

    if (message->len > 8)
    {
        return 0;
    }

    if (message->extended)
    {
        if (message->id > 0x1FFFFFFF)
        {
            return 0;
        }
    }
    else
    {
        if (message->id > 0x7FF)
        {
            return 0;
        }
    }

    if (CANx->TSR & CAN_TSR_TME0)
    {
        mailbox = 0;
    }
    else if (CANx->TSR & CAN_TSR_TME1)
    {
        mailbox = 1;
    }
    else if (CANx->TSR & CAN_TSR_TME2)
    {
        mailbox = 2;
    }
    else
    {
        return 0;
    }

    if (message->extended)
    {
        CANx->sTxMailBox[mailbox].TIR =
            (message->id << 3) | CAN_TI0R_IDE;
    }
    else
    {
        CANx->sTxMailBox[mailbox].TIR =
            message->id << 21;
    }

    CANx->sTxMailBox[mailbox].TDTR = message->len;

    CANx->sTxMailBox[mailbox].TDLR =
        ((uint32_t)message->data[0]) |
        ((uint32_t)message->data[1] << 8) |
        ((uint32_t)message->data[2] << 16) |
        ((uint32_t)message->data[3] << 24);

    CANx->sTxMailBox[mailbox].TDHR =
        ((uint32_t)message->data[4]) |
        ((uint32_t)message->data[5] << 8) |
        ((uint32_t)message->data[6] << 16) |
        ((uint32_t)message->data[7] << 24);

    CANx->sTxMailBox[mailbox].TIR |= CAN_TI0R_TXRQ;

    timeout = CAN_TIMEOUT;

    while (timeout--)
    {
        if (CANx->TSR & (CAN_TSR_RQCP0 << mailbox))
        {
            if (CANx->TSR & (CAN_TSR_TXOK0 << mailbox))
            {
                CANx->TSR |= CAN_TSR_RQCP0 << mailbox;

                return 1;
            }

            CANx->TSR |= CAN_TSR_RQCP0 << mailbox;

            return 0;
        }
    }

    return 0;
}

uint8_t CAN_Receive(CAN_TypeDef *CANx, CAN_Message *message)
{
    uint32_t rir;
    uint32_t rdlr;
    uint32_t rdhr;

    if ((CANx->RF0R & CAN_RF0R_FMP0) == 0)
    {
        return 0;
    }

    rir = CANx->sFIFOMailBox[0].RIR;
    rdlr = CANx->sFIFOMailBox[0].RDLR;
    rdhr = CANx->sFIFOMailBox[0].RDHR;

    if (rir & CAN_RI0R_IDE)
    {
        message->extended = 1;
        message->id = (rir >> 3) & 0x1FFFFFFF;
    }
    else
    {
        message->extended = 0;
        message->id = (rir >> 21) & 0x7FF;
    }

    message->len = CANx->sFIFOMailBox[0].RDTR & 0x0F;

    if (message->len > 0)
    {
        message->data[0] = rdlr & 0xFF;
    }

    if (message->len > 1)
    {
        message->data[1] = (rdlr >> 8) & 0xFF;
    }

    if (message->len > 2)
    {
        message->data[2] = (rdlr >> 16) & 0xFF;
    }

    if (message->len > 3)
    {
        message->data[3] = (rdlr >> 24) & 0xFF;
    }

    if (message->len > 4)
    {
        message->data[4] = rdhr & 0xFF;
    }

    if (message->len > 5)
    {
        message->data[5] = (rdhr >> 8) & 0xFF;
    }

    if (message->len > 6)
    {
        message->data[6] = (rdhr >> 16) & 0xFF;
    }

    if (message->len > 7)
    {
        message->data[7] = (rdhr >> 24) & 0xFF;
    }

    CANx->RF0R |= CAN_RF0R_RFOM0;

    return 1;
}
