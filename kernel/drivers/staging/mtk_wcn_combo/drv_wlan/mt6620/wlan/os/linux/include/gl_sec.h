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

#ifndef _GL_SEC_H
#define _GL_SEC_H

extern void handle_sec_msg_1(unsigned char *msg_in,
			     int msg_in_len,
			     unsigned char *msg_out, int *msg_out_len);
extern void handle_sec_msg_2(unsigned char *msg_in,
			     int msg_in_len,
			     unsigned char *msg_out, int *msg_out_len);
extern void handle_sec_msg_3(unsigned char *msg_in,
			     int msg_in_len,
			     unsigned char *msg_out, int *msg_out_len);
extern void handle_sec_msg_4(unsigned char *msg_in,
			     int msg_in_len,
			     unsigned char *msg_out, int *msg_out_len);
extern void handle_sec_msg_5(unsigned char *msg_in,
			     int msg_in_len,
			     unsigned char *msg_out, int *msg_out_len);
extern void handle_sec_msg_final(unsigned char *msg_in,
				 int msg_in_len,
				 unsigned char *msg_out, int *msg_out_len);

#endif /* _GL_SEC_H */
