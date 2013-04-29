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

/*! \file
\brief  Declaration of library functions

Any definitions in this file will be shared among GLUE Layer and internal Driver Stack.
*/

#ifndef _OSAL_TYPEDEF_H_
#define _OSAL_TYPEDEF_H_

typedef void VOID;
typedef void *PVOID;

typedef char CHAR;
typedef char *PCHAR;
typedef signed char INT8;
typedef signed char *PINT8;
typedef unsigned char UINT8;
typedef unsigned char *PUINT8;
typedef unsigned char UCHAR;
typedef unsigned char *PUCHAR;

typedef signed short INT16;
typedef signed short *PINT16;
typedef unsigned short UINT16;
typedef unsigned short *PUINT16;

typedef signed long LONG;
typedef signed long *PLONG;

typedef signed int INT32;
typedef signed int *PINT32;
typedef unsigned int UINT32;
typedef unsigned int *PUINT32;

typedef unsigned long ULONG;
typedef unsigned long *PULONG;

typedef int MTK_WCN_BOOL;
#ifndef MTK_WCN_BOOL_TRUE
#define MTK_WCN_BOOL_FALSE               ((MTK_WCN_BOOL)0)
#define MTK_WCN_BOOL_TRUE                ((MTK_WCN_BOOL)1)
#endif

#endif /*_OSAL_TYPEDEF_H_*/
