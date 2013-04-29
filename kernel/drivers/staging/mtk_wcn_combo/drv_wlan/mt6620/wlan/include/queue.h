/*******************************************************************************
* Copyright (c) 2013, MediaTek Inc.
*
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License version 2 as published by the Free
* Software Foundation.
*
* Alternatively, this software may be distributed under the terms of BSD
* license.
********************************************************************************
*/

/*******************************************************************************
* THIS SOFTWARE IS PROVIDED BY MEDIATEK "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT OR FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL MEDIATEK BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
*/

#ifndef _QUEUE_H
#define _QUEUE_H

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "gl_typedef.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/
/* Singly Queue Structures - Entry Part */
typedef struct _QUE_ENTRY_T {
	struct _QUE_ENTRY_T *prNext;
	struct _QUE_ENTRY_T *prPrev;	/* For Rx buffer reordering used only */
} QUE_ENTRY_T, *P_QUE_ENTRY_T;

/* Singly Queue Structures - Queue Part */
typedef struct _QUE_T {
	P_QUE_ENTRY_T prHead;
	P_QUE_ENTRY_T prTail;
	UINT_32 u4NumElem;
} QUE_T, *P_QUE_T;

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/
#define QUEUE_INITIALIZE(prQueue) \
{ \
(prQueue)->prHead = (P_QUE_ENTRY_T)NULL; \
(prQueue)->prTail = (P_QUE_ENTRY_T)NULL; \
(prQueue)->u4NumElem = 0; \
}

#define QUEUE_IS_EMPTY(prQueue)             (((P_QUE_T)(prQueue))->prHead == \
(P_QUE_ENTRY_T)NULL)

#define QUEUE_IS_NOT_EMPTY(prQueue)         ((prQueue)->u4NumElem > 0)

#define QUEUE_GET_HEAD(prQueue)             ((prQueue)->prHead)

#define QUEUE_GET_TAIL(prQueue)             ((prQueue)->prTail)

#define QUEUE_GET_NEXT_ENTRY(prQueueEntry)  ((prQueueEntry)->prNext)

#define QUEUE_INSERT_HEAD(prQueue, prQueueEntry) \
{ \
ASSERT(prQueue); \
ASSERT(prQueueEntry); \
(prQueueEntry)->prNext = (prQueue)->prHead; \
(prQueue)->prHead = (prQueueEntry); \
if ((prQueue)->prTail == (P_QUE_ENTRY_T)NULL) {	\
(prQueue)->prTail = (prQueueEntry); \
} \
((prQueue)->u4NumElem)++; \
}

#define QUEUE_INSERT_TAIL(prQueue, prQueueEntry) \
{ \
ASSERT(prQueue); \
ASSERT(prQueueEntry); \
(prQueueEntry)->prNext = (P_QUE_ENTRY_T)NULL; \
if ((prQueue)->prTail) { \
((prQueue)->prTail)->prNext = (prQueueEntry); \
} else { \
(prQueue)->prHead = (prQueueEntry); \
} \
(prQueue)->prTail = (prQueueEntry); \
((prQueue)->u4NumElem)++; \
}

/* NOTE: We assume the queue entry located at the beginning of "prQueueEntry Type",
* so that we can cast the queue entry to other data type without doubts.
* And this macro also decrease the total entry count at the same time.
*/
#define QUEUE_REMOVE_HEAD(prQueue, prQueueEntry, _P_TYPE) \
{ \
ASSERT(prQueue); \
prQueueEntry = (_P_TYPE)((prQueue)->prHead); \
if (prQueueEntry) { \
(prQueue)->prHead = \
((P_QUE_ENTRY_T)(prQueueEntry))->prNext; \
if ((prQueue)->prHead == (P_QUE_ENTRY_T)NULL) {	\
(prQueue)->prTail = (P_QUE_ENTRY_T)NULL; \
} \
((P_QUE_ENTRY_T)(prQueueEntry))->prNext = \
(P_QUE_ENTRY_T)NULL; \
((prQueue)->u4NumElem)--; \
} \
}

#define QUEUE_MOVE_ALL(prDestQueue, prSrcQueue)	\
{ \
ASSERT(prDestQueue); \
ASSERT(prSrcQueue); \
*(P_QUE_T)prDestQueue = *(P_QUE_T)prSrcQueue; \
QUEUE_INITIALIZE(prSrcQueue); \
}

#define QUEUE_CONCATENATE_QUEUES(prDestQueue, prSrcQueue) \
{ \
ASSERT(prDestQueue); \
ASSERT(prSrcQueue); \
if (prSrcQueue->u4NumElem > 0) { \
if ((prDestQueue)->prTail) { \
((prDestQueue)->prTail)->prNext = \
(prSrcQueue)->prHead; \
} else { \
(prDestQueue)->prHead = (prSrcQueue)->prHead; \
} \
(prDestQueue)->prTail = (prSrcQueue)->prTail; \
((prDestQueue)->u4NumElem) += ((prSrcQueue)->u4NumElem); \
QUEUE_INITIALIZE(prSrcQueue); \
} \
}

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

#endif /* _QUEUE_H */
