/******************************************************************************
 *    Copyright (C) 2014 Hisilicon STB Development Dept
 *    All rights reserved.
 * ***
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *   http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
******************************************************************************/
/*
 * 1. R0 should be reserved
 * 2. SP used space: 0x200 bytes
 * 3. this function use physical address.
 * 4. this function will destory ddr data 0x8000_0000 - 0x8000_000c.
 * 5. wakeup should restore following registers:
 *     0X101E_00C0 -> 0X1010083C
 *     0X101E_00C4 -> 0X101009C0
 *     0X101E_00C8 -> 0X101008E0
 *     0X101E_00CC -> 0X10100920
 * 6. return: 0 -> success, 1 -> fail
 */

.globl ddrphy_train_route

_pad:
	/* move this train between 2k */
	.fill 200,1,0

 ddrphy_train_route:
	STMFD SP!, {R1-R12,R14}

	LDR   R1, =0X101E0000
	LDR   R2, [R1, #0XD0]
	AND   R2, R2, #0XF
	CMP   R2, #1
	BEQ   DDRPHY_TRAIN_RETURN

	/* Used space */
	SUB   R8, SP, #0X200

	/* PHY Training Bypass */
	LDR   R0, =0x10100800
	MOV   R1, #0X2
	STR   R1, [R0,#0X8]         // calibrarion bypass

	LDR   R1, =0X600C0A00
	LDR   R2, [R1, #0X8]        // ECO Version
	LSR   R2, R2, #0X18
	AND   R2, R2, #0XF
	CMP   R2, #1
	BEQ   CHECK_CONFIG

	LDR   R1, [R0, #0XE0]
	BIC   R1, R1, #0X7
	ORR   R1, R1, #0X2
	STR   R1, [R0, #0XE0]

	LDR   R1, [R0, #0X120]
	BIC   R1, R1, #0X7
	ORR   R1, R1, #0X2
	STR   R1, [R0, #0X120]

	MOV   R1, #0XB
	STR   R1, [R0, #0XD4]
	STR   R1, [R0, #0X114]
	MOV   R1, #0X1E
	STR   R1, [R0, #0X204]
	B     BYPASS_MST_DLL

CHECK_CONFIG:
	LDR   R1, =0X101E0000
	LDR   R2, [R1, #0XD0]
	LSR   R2, R2, #0X4
	AND   R2, R2, #0XF
	CMP   R2, #1
	BEQ   TRAIN_LOW_START

	/* Bypass DDRPHY Master DLL */
BYPASS_MST_DLL:
	MOV   R1, #0
	LDR   R7, [R0, #0X3C0]
	AND   R2, R7, #0X3F
LOCK_CODE_SUM:
	LDR   R7, [R0, #0X3C0]
	AND   R3, R7, #0X3F
	ADD   R2, R2, R3
	ADD   R1, R1, #1
	CMP   R1, #0X20
	BCC   LOCK_CODE_SUM
	LSR   R2, R2, #5
	STR   R2, [R0,#0X200]
	LDR   R1, [R0,#0X204]
	ORR   R1, R1, #0X40
	STR   R1, [R0,#0X204]

TRAIN_LOW_START:
	MOV   R1, #0              // DLL_SEL0
	MOV   R2, #0              // OPH_SEL0
	MOV   R3, #1              // CYC_SEL0
	MOV   R4, #2              // LOW_BYTE_COUNT
	STR   R1, [R8,#0X0]
	STR   R2, [R8,#0X4]
	STR   R3, [R8,#0X8]
	STR   R1, [R8,#0X10]
	STR   R1, [R8,#0X14]
	STR   R1, [R8,#0X18]

TRAIN_LOW_LOOP:
	BL    DDRPHY_TRAIN_DATA
	LDR   R5,  =0XD22D
	LDR   R6,  =0XFFFF
	AND   R9, R9, R6
	CMP   R9, R5
	BNE   TRAIN_LOW_ADJ
	LDR   R5,  =0X5AA5
	LDR   R6,  =0XFFFF
	AND   R10, R10, R6
	CMP   R10, R5
	BNE   TRAIN_LOW_ADJ
	LDR   R5,  =0X7887
	LDR   R6,  =0XFFFF
	AND   R11, R11, R6
	CMP   R11, R5
	BNE   TRAIN_LOW_ADJ
	LDR   R5,  =0XC33C
	LDR   R6,  =0XFFFF
	AND   R12, R12, R6
	CMP   R12, R5
	BNE   TRAIN_LOW_ADJ
	LSL   R5, R4, #4
	STR   R1, [R8,R5]
	ADD   R5, R5, #0X4
	STR   R2, [R8,R5]
	ADD   R5, R5, #0X4
	STR   R3, [R8,R5]
	ADD   R4, R4, #1
	BL    TRAIN_LOW_STEP
	B     TRAIN_LOW_LOOP

TRAIN_LOW_ADJ:
	CMP   R4, #5
	BHI   TRAIN_LOW_FINISH
	BL    TRAIN_LOW_STEP
	B     TRAIN_LOW_LOOP

TRAIN_LOW_STEP:
	LDR   R5, [R0, #0X204]
	AND   R5, R5, #0X7
	CMP   R1, R5
	BEQ   OPH_SEL0_ADD
	ADD   R1, R1, #1
	STR   R1, [R8,#0X0]  // DLL_SEL0
	B     SEL0_ADD_BACK

OPH_SEL0_ADD:
	MOV   R1, #0
	STR   R1, [R8,#0X0]  // DLL_SEL0
	CMP   R2, #1
	BEQ   CYC_SEL0_ADD
	ADD   R2, R2, #1
	STR   R2, [R8,#0X4]  // OPH_SEL0
	B     SEL0_ADD_BACK

CYC_SEL0_ADD:
	MOV   R2, #0
	STR   R2, [R8,#0X4]  // OPH_SEL0
	CMP   R3, #7
	BEQ   CYC_SEL0_FULL
	ADD   R3, R3, #1
	STR   R3, [R8,#0X8]  // CYC_SEL0
	B     SEL0_ADD_BACK

CYC_SEL0_FULL:
	CMP   R4, #2
	MOVEQ R5, #0
	BEQ   TRAIN_LOW_RESULT
	CMP   R4, #6
	MOVCC R5, #2
	BCC   TRAIN_LOW_RESULT
	MOV   R3, #0
	STR   R3, [R8,#0X8]  // CYC_SEL0

SEL0_ADD_BACK:
	BX    LR

TRAIN_LOW_FINISH:
	ADD   R5, R4, #1
	LSR   R5, R5, #1

TRAIN_LOW_RESULT:
	LSL   R5, R5, #4
	LDR   R6, [R8, R5]
	STR   R6, [R8, #0X0]
	ADD   R5, R5, #4
	LDR   R6, [R8, R5]
	STR   R6, [R8, #0X4]
	ADD   R5, R5, #4
	LDR   R6, [R8, R5]
	STR   R6, [R8, #0X8]

TRAIN_HIGH_START:
	MOV   R1, #0              // DLL_SEL1
	MOV   R2, #0              // OPH_SEL1
	MOV   R3, #1              // CYC_SEL1
	MOV   R4, #2              // HIGH_BYTE_COUNT
	STR   R1, [R8,#0X10]
	STR   R2, [R8,#0X14]
	STR   R3, [R8,#0X18]

TRAIN_HIGH_LOOP:
	BL    DDRPHY_TRAIN_DATA
	LDR   R7,  =0XE11E
	LSR   R9, R9, #16
	CMP   R9, R7
	BNE   TRAIN_HIGH_ADJ
	LDR   R7,  =0X6996
	LSR   R10, R10, #16
	CMP   R10, R7
	BNE   TRAIN_HIGH_ADJ
	LDR   R7,  =0XA55A
	LSR   R11, R11, #16
	CMP   R11, R7
	BNE   TRAIN_HIGH_ADJ
	LDR   R7,  =0XB44B
	LSR   R12, R12, #16
	CMP   R12, R7
	BNE   TRAIN_HIGH_ADJ
	LSL   R5, R4, #4
	STR   R1, [R8,R5]
	ADD   R5, R5, #0X4
	STR   R2, [R8,R5]
	ADD   R5, R5, #0X4
	STR   R3, [R8,R5]
	ADD   R4, R4, #1
	BL    TRAIN_HIGH_STEP
	B     TRAIN_HIGH_LOOP

TRAIN_HIGH_ADJ:
	CMP   R4, #5
	BHI   TRAIN_HIGH_FINISH
	BL    TRAIN_HIGH_STEP
	B     TRAIN_HIGH_LOOP

TRAIN_HIGH_STEP:
	LDR   R5, [R0, #0X204]
	AND   R5, R5, #0X7
	CMP   R1, R5
	BEQ   OPH_SEL1_ADD
	ADD   R1, R1, #1
	STR   R1, [R8,#0X10]  // DLL_SEL1
	B     SEL1_ADD_BACK

OPH_SEL1_ADD:
	MOV   R1, #0
	STR   R1, [R8,#0X10]  // DLL_SEL1
	CMP   R2, #1
	BEQ   CYC_SEL1_ADD
	ADD   R2, R2, #1
	STR   R2, [R8,#0X14]  // OPH_SEL1
	B     SEL1_ADD_BACK

CYC_SEL1_ADD:
	MOV   R2, #0
	STR   R2, [R8,#0X14]  // OPH_SEL1
	CMP   R3, #7
	BEQ   CYC_SEL1_FULL
	ADD   R3, R3, #1
	STR   R3, [R8,#0X18]  // CYC_SEL1
	B     SEL1_ADD_BACK

CYC_SEL1_FULL:
	CMP   R4, #2
	MOVEQ R5, #0
	BEQ   TRAIN_HIGH_RESULT
	CMP   R4, #6
	MOVCC R5, #2
	BCC   TRAIN_HIGH_RESULT
	MOV   R3, #0
	STR   R3, [R8,#0X18]  // CYC_SEL1

SEL1_ADD_BACK:
	BX    LR

TRAIN_HIGH_FINISH:
	ADD   R5, R4, #1
	LSR   R5, R5, #1

TRAIN_HIGH_RESULT:
	LSL   R5, R5, #4
	LDR   R6, [R8, R5]
	STR   R6, [R8, #0X10]
	ADD   R5, R5, #4
	LDR   R6, [R8, R5]
	STR   R6, [R8, #0X14]
	ADD   R5, R5, #4
	LDR   R6, [R8, R5]
	STR   R6, [R8, #0X18]
	BL    DDRPHY_TRAIN_DATA
	LDR   R7, =0XE11ED22D
	CMP   R7, R9
	BNE   DDRPHY_TRAIN_ERROR
	LDR   R7, =0X69965AA5
	CMP   R7, R10
	BNE   DDRPHY_TRAIN_ERROR
	LDR   R7, =0XA55A7887
	CMP   R7, R11
	BNE   DDRPHY_TRAIN_ERROR
	LDR   R7, =0XB44BC33C
	CMP   R7, R12
	BNE   DDRPHY_TRAIN_ERROR

DDRPHY_TRAIN_RETURN:
	MOV   R0, #0
	LDMFD SP!, {R1-R12,PC}       // RETURN

DDRPHY_TRAIN_DATA:

	LDR   R7, [R8,#0X8]
	LDR   R9, [R8,#0X18]
	ADD   R10, R7, R9, LSL #3    // {CYC_SEL1[2:0],CYC_SEL0[2:0]}
	STR   R10, [R0,#0X3C]

	LDR   R7, [R8,#0X4]
	LDR   R9, [R8,#0X14]
	ORR   R7, R7, #4
	ORR   R9, R9, #4
	ADD   R10, R7, R9, LSL #3    // {1, OPH_SEL1[4:3], 1, OPH_SEL0[4:3]}
	STR   R10, [R0,#0X1C0]

	LDR   R7, [R8,#0X0]
	LSL   R7, R7, #3
	LDR   R9, [R0,#0XE0]
	AND   R9, R9, #7
	ADD   R9, R9, R7
	STR   R9, [R0,#0XE0]         // {DLL_SEL0[7:5],3'bXXX}

	LDR   R7, [R8,#0X10]
	LSL   R7, R7, #3
	LDR   R9, [R0,#0X120]
	AND   R9, R9, #7
	ADD   R9, R9, R7
	STR   R9, [R0,#0X120]        // {DLL_SEL1[7:5],3'bXXX}

	LDR     R7,  =0x80101000
	LDR     R9,  =0XE11ED22D
	LDR     R10, =0X69965AA5
	LDR     R11, =0XA55A7887
	LDR     R12, =0XB44BC33C
	STMIA   R7, {R9-R12}
	LDMIA   R7, {R9-R12}
	LDR     R7,  =0x80201800
	STMIA   R7, {R9-R12}
	LDMIA   R7, {R9-R12}
	LDR     R7,  =0x80302000
	STMIA   R7, {R9-R12}
	LDMIA   R7, {R9-R12}
	LDR     R7,  =0x80402800
	STMIA   R7, {R9-R12}
	LDMIA   R7, {R9-R12}

	BX      LR

DDRPHY_TRAIN_ERROR:
	MOV   R0, #1
	LDMFD SP!, {R1-R12,PC}       // RETURN

