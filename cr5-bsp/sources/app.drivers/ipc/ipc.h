/*
***************************************************************************************************
*
*   FileName : ipc.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*
*   TCC Version 1.0
*
*   This source code contains confidential information of Telechips.
*
*   Any unauthorized use without a written permission of Telechips including not limited to
*   re-distribution in source or binary form is strictly prohibited.
*
*   This source code is provided "AS IS" and nothing contained in this source code shall constitute
*   any express or implied warranty of any kind, including without limitation, any warranty of
*   merchantability, fitness for a particular purpose or non-infringement of any patent, copyright
*   or other third party intellectual property right. No warranty is made, express or implied,
*   regarding the information's accuracy,completeness, or performance.
*
*   In no event shall Telechips be liable for any claim, damages or other liability arising from,
*   out of or in connection with this source code or the use in the source code.
*
*   This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between
*   Telechips and Company.
*   This source code is provided "AS IS" and nothing contained in this source code shall constitute
*   any express or implied warranty of any kind, including without limitation, any warranty
*   (of merchantability, fitness for a particular purpose or non-infringement of any patent,
*   copyright or other third party intellectual property right. No warranty is made, express or
*   implied, regarding the information's accuracy, completeness, or performance.
*   In no event shall Telechips be liable for any claim, damages or other liability arising from,
*   out of or in connection with this source code or the use in the source code.
*   This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement
*   between Telechips and Company.
*
***************************************************************************************************
*/

#ifndef TCC_IPC_H
#define TCC_IPC_H

#include <sal_internal.h>

#define IPC_SUCCESS                     (0)              /* Success */
#define IPC_ERR_COMMON                  (-1)             /* common error*/
#define IPC_ERR_BUSY                    (-2)             /* IPC is busy. You got the return, After a while you should try.*/
#define IPC_ERR_NOTREADY                (-3)             /* IPC is not ready. Other processor is not ready yet.*/
#define IPC_ERR_TIMEOUT                 (-4)             /* Other process is not responding. */
#define IPC_ERR_WRITE                   (-5)
#define IPC_ERR_READ                    (-6)
#define IPC_ERR_BUFFER                  (-7)
#define IPC_ERR_ARGUMENT                (-8)             /* Invalid argument */
#define IPC_ERR_RECEIVER_NOT_SET        (-9)             /* Receiver processor not set */
#define IPC_ERR_RECEIVER_DOWN           (-10)            /* Mbox is not set */
#define IPC_ERR_RECV_NACK               (-11)            /* AP sent NACK */
#define IPC_ERR_INVALID_DL              (-800)
#define IPC_ERR_NOT_SUPPORTED           (-900)

#define IPC_FLAG_USENAMCK               (0x00000001U)
#define IPC_NACK_REASON_BUFFFUL         (0x00000001U)
#define IPC_NACK_REASON_BUFFERR         (0x00000002U)
/*
***********************************************************************************
*   IPC CMD ID - Key
***********************************************************************************
*/
#define TCC_IPC_CMD_KEY                 (0x02) // define for CMD1
#define TCC_IPC_CMD_KEY_EXT             (0x01) // define for CMD2
#define TCC_IPC_CMD_KEY_ROTARY          (0x02) // define for CMD2
/*
**********************************************************************************
*   IPC CMD ID - System Monitoring
***********************************************************************************
*/
#define TCC_IPC_CMD_SYS_MONITORING      (0x04U) // define for CMD1
#define IPC_SYS_MON_RECOG_REQ           (0x01U) // define for CMD2 : (MICOM => Peer Core)
#define IPC_SYS_MON_RECOG_RES           (0x01U) // define for CMD2 : (Peer Core => MICOM)

typedef void (*IPCCallback)
(
    uint16                              uiCmd,
    uint8 *                             pucData,
    uint16                              uiLength
);

typedef enum IPCSvcCh
{
#if defined(TCC805x)
    IPC_SVC_CH_CA53_SECURE,
    IPC_SVC_CH_CA53_NONSECURE,
    IPC_SVC_CH_CA72_SECURE,
    IPC_SVC_CH_CA72_NONSECURE,
#else
    #error no target device designated.
#endif
    IPC_SVC_CH_MAX,
} IPCSvcCh_t;

typedef enum IPCIoCmd
{
    IOCTL_IPC_WRITE                     = 0,
    IOCTL_IPC_READ                      = 1,
    IOCTL_IPC_SET_PARAM                 = 2,
    IOCTL_IPC_GET_PARAM                 = 3,
    IOCTL_IPC_PING_TEST                 = 4,
    IOCTL_IPC_FLUSH                     = 5,
    IOCTL_IPC_ISREADY                   = 6,
} IPCIoCmd_t;


void IPC_Create
(
    void
);

void IPC_RegisterCbFunc
(
    IPCSvcCh_t                          siCh,
    uint8                               ucId,
    IPCCallback                         pFunc,
    void *                              pArg1,
    void *                              pArg2
);

int32 IPC_SendPacket
(
    IPCSvcCh_t                          siCh,
    uint16                              uiCmd1,
    uint16                              uiCmd2,
    const uint8 *                       pucData,
    uint16                              uiLength
);

int32 IPC_IsReady
(
    uint32                              uiCh,
    uint32 *                            uiIpcReady
);

// -------------------------- MBOX_TEST ---------------------------------
#include "ipc_typedef.h"

int32 IPC_init
(
    void
);

int32 IPC_SendToMainCtlCmd
(
    IPCCmdId_t                          cmdID
);

int32 IPC_SendToMainData
(
    uint32                               data
);

int32 IPC_CreateTask(void);
#endif

