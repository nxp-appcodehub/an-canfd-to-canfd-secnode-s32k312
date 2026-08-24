/*==================================================================================================
*   Project              : RTD AUTOSAR 4.9
*   Platform             : CORTEXM
*   Peripheral           :
*   Dependencies         : none
*
*   Autosar Version      : 4.9.0
*   Autosar Revision     : ASR_REL_4_9_REV_0000
*   Autosar Conf.Variant :
*   SW Version           : 7.0.1
*   Build Version        : S32K3_RTD_7_0_1_D2602_ASR_REL_4_9_REV_0000_20260206
*
*   Copyright 2020 - 2026 NXP
*
*   NXP Proprietary. This software is owned or controlled by NXP and may only be
*   used strictly in accordance with the applicable license terms. By expressly
*   accepting such terms or by downloading, installing, activating and/or otherwise
*   using the software, you are agreeing that you have read, and that you agree to
*   comply with and are bound by, such license terms. If you do not agree to be
*   bound by the applicable license terms, then you may not retain, install,
*   activate or otherwise use the software.
==================================================================================================*/

#ifdef __cplusplus
extern "C" {
#endif


/*==================================================================================================
*                                        INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/
#include "Mcal.h"
#include "Clock_Ip.h"
#include "FlexCAN_Ip.h"
#include "IntCtrl_Ip.h"
#include "Siul2_Port_Ip.h"
#include "Siul2_Dio_Ip.h"

/*==================================================================================================
*                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/


/*==================================================================================================
*                                       LOCAL MACROS
==================================================================================================*/
#define CAN_TX_NODE 				(1U)
#define CAN_RX_NODE				(0U)
#define CAN_LOOPBACK_MODE			(0U)

#define MASTER_ID					(0xC0DE)
#define DEVICE_ID					(0xC0DE)

#define RX_MB_IDX                  (0U)
#define TX_MB_IDX                  (1U)
#define TX_BUF_LEN					 (8U)
#define FLEXCAN_NUMBER_OF_MSG      (10U)
#define TIMEOUT_VALUE              (1000U)
/*==================================================================================================
*                                      LOCAL CONSTANTS
==================================================================================================*/


/*==================================================================================================
*                                      LOCAL VARIABLES
==================================================================================================*/


/*==================================================================================================
*                                      GLOBAL CONSTANTS
==================================================================================================*/


/*==================================================================================================
*                                      GLOBAL VARIABLES
==================================================================================================*/

uint8 au8Data[TX_BUF_LEN] = {0x01U, 0x02U, 0x03U, 0x04U, 0x05U, 0x06U, 0x07U, 0x08U};
volatile uint8 u8TxConfirmCnt = 0U;
volatile uint8 u8RxIndicationCnt = 0U;
volatile boolean bTxFlag = FALSE;
volatile boolean bRxFlag = FALSE;
Flexcan_Ip_MsgBuffType aRxDataBuffer[FLEXCAN_NUMBER_OF_MSG];
Flexcan_Ip_DataInfoType RxInfo = {
        .msg_id_type = FLEXCAN_MSG_ID_STD,
        .data_length = 8U,
        .is_polling = FALSE,
        .is_remote = FALSE
};
Flexcan_Ip_DataInfoType TxInfo = {
        .msg_id_type = FLEXCAN_MSG_ID_STD,
        .data_length = 8U,
        .is_polling = TRUE,
        .is_remote = FALSE
};
/*==================================================================================================
*                                   LOCAL FUNCTION PROTOTYPES
==================================================================================================*/


/*==================================================================================================
*                                       LOCAL FUNCTIONS
==================================================================================================*/

void FlexCAN_UserCallback(uint8 instance,
                          Flexcan_Ip_EventType eventType,
                          uint32 buffIdx,
                          const struct FlexCANState *driverState)

{
#if (CAN_RX_NODE || CAN_LOOPBACK_MODE)
    if (FLEXCAN_EVENT_RX_COMPLETE == eventType)
    {
		/* Configure Rx message buffer */
		FlexCAN_Ip_ConfigRxMb(INST_FLEXCAN_0, RX_MB_IDX, &RxInfo, MASTER_ID);
		/* Prepare to receive the next message */
		FlexCAN_Ip_Receive(INST_FLEXCAN_0, RX_MB_IDX, &aRxDataBuffer[0], TRUE);

		bRxFlag = TRUE;
    }
#endif
#if (CAN_TX_NODE || CAN_LOOPBACK_MODE)
    if (FLEXCAN_EVENT_TX_COMPLETE == eventType)
    {
    	bTxFlag = TRUE;
    }
#endif
    (void)instance;
    (void)buffIdx;
    (void)driverState;
}
/*==================================================================================================
*                                       GLOBAL FUNCTIONS
==================================================================================================*/

int main(void)
{
    /* Initialize Clock */
    Clock_Ip_Init(&Clock_Ip_aClockConfig[0]);

    /* Initialize Interrupt */
    IntCtrl_Ip_Init(&IntCtrlConfig_0);

    /* Initialize Port Driver */
    Siul2_Port_Ip_Init(NUM_OF_CONFIGURED_PINS_PortContainer_0_BOARD_InitPeripherals, g_pin_mux_InitConfigArr_PortContainer_0_BOARD_InitPeripherals);

    /* Initialize FlexCAN driver */
    FlexCAN_Ip_Init(INST_FLEXCAN_0, &FlexCAN_State0, &FlexCAN_Config0);

#if (CAN_RX_NODE || CAN_LOOPBACK_MODE)
    uint8_t RxSuccessful = TRUE;

    /* Set Rx filter mask type */
    FlexCAN_Ip_SetRxMaskType(INST_FLEXCAN_0, FLEXCAN_RX_MASK_INDIVIDUAL);

    /* Set Rx individual mask value */
    /* Expect to receive all IDs, mask = 0x0 */
    FlexCAN_Ip_SetRxIndividualMask(INST_FLEXCAN_0, RX_MB_IDX, 0x0U);

    /* Configure Rx message buffer */
    FlexCAN_Ip_ConfigRxMb(INST_FLEXCAN_0, RX_MB_IDX, &RxInfo, 0x0U);

    /* Start trigger to receive messages */
    FlexCAN_Ip_Receive(INST_FLEXCAN_0, RX_MB_IDX, &aRxDataBuffer[0], TRUE);
#endif

    /* Start FlexCAN controller */
    FlexCAN_Ip_SetStartMode(INST_FLEXCAN_0);

    /* Enable CANPHY */
    Siul2_Dio_Ip_WritePin(CAN0_EN_PORT, CAN0_EN_PIN, 1U);
    Siul2_Dio_Ip_WritePin(CAN0_STB_PORT, CAN0_STB_PIN, 1U);

    /* Turn RGB LED Off */
    Siul2_Dio_Ip_WritePin(RED_LED_PORT, RED_LED_PIN, 0U);
    Siul2_Dio_Ip_WritePin(GREEN_LED_PORT, GREEN_LED_PIN, 0U);
    Siul2_Dio_Ip_WritePin(BLUE_LED_PORT, BLUE_LED_PIN, 0U);

    for(;;)
    {
#if CAN_TX_NODE
        /* Perform CAN Transmission */
        FlexCAN_Ip_Send(INST_FLEXCAN_0, TX_MB_IDX, &TxInfo, DEVICE_ID, (uint8*)au8Data);

        /* Turn BLUE LED ON */
        Siul2_Dio_Ip_WritePin(RED_LED_PORT, RED_LED_PIN, 0U);
        Siul2_Dio_Ip_WritePin(GREEN_LED_PORT, GREEN_LED_PIN, 0U);
        Siul2_Dio_Ip_WritePin(BLUE_LED_PORT, BLUE_LED_PIN, 1U);

    	while (FALSE == bTxFlag)
    	{
    		FlexCAN_Ip_MainFunctionWrite(INST_FLEXCAN_0, TX_MB_IDX);
    	}
    	bTxFlag = FALSE;
    	for(uint32_t i=0; i<5000000; i++);
#elif CAN_RX_NODE
    	/* Process received Rx MessageBuffer */
    	FlexCAN_Ip_MainFunctionRead(INST_FLEXCAN_0, RX_MB_IDX);
    	if(TRUE == bRxFlag)
    	{
    		/* Checked the received buffer */
    		for(uint8_t u8MsgIdx=0; u8MsgIdx < TX_BUF_LEN; u8MsgIdx++)
    		{
    			if(aRxDataBuffer->data[u8MsgIdx] != au8Data[u8MsgIdx])
    			{
    				/* Message was not received correctly */
    				/* Turn RED LED ON */
    				Siul2_Dio_Ip_WritePin(RED_LED_PORT, RED_LED_PIN, 1U);
    			    Siul2_Dio_Ip_WritePin(GREEN_LED_PORT, GREEN_LED_PIN, 0U);
    			    Siul2_Dio_Ip_WritePin(BLUE_LED_PORT, BLUE_LED_PIN, 0U);
    			    RxSuccessful = FALSE;
    				break;
    			}
    		}
    		if(RxSuccessful)
    		{
				/* Message was received correctly */
    			/* Turn GREEN LED ON */
				Siul2_Dio_Ip_WritePin(RED_LED_PORT, RED_LED_PIN, 0U);
			    Siul2_Dio_Ip_WritePin(GREEN_LED_PORT, GREEN_LED_PIN, 1U);
			    Siul2_Dio_Ip_WritePin(BLUE_LED_PORT, BLUE_LED_PIN, 0U);
    		}
    		bRxFlag = FALSE;
    	}

#elif CAN_LOOPBACK_MODE
    	uint32 u32TimeOut = 0U;

        /* Perform CAN Transmission of multiple messages */
        for (uint8 MsgIdx = 1U; MsgIdx <= FLEXCAN_NUMBER_OF_MSG; MsgIdx++)
        {
            u32TimeOut = TIMEOUT_VALUE;
            FlexCAN_Ip_Send(INST_FLEXCAN_0, TX_MB_IDX, &TxInfo, MsgIdx, au8Data);
            while ((u8TxConfirmCnt != MsgIdx) && (u32TimeOut != 0U))
            {
                FlexCAN_Ip_MainFunctionWrite(INST_FLEXCAN_0, TX_MB_IDX);
                u32TimeOut--;
            }
        }

#endif
    }

    /* De-Initialize FlexCAN driver */
    FlexCAN_Ip_Deinit(INST_FLEXCAN_0);

    return (0U);
}

#ifdef __cplusplus
}
#endif

/** @} */
