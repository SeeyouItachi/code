/** ###################################################################
**     THIS COMPONENT MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : CHG_CAN_POW.c
**     Project   : mcal
**     Processor : MC9S12XEP100MAG
**     Component : BitIO
**     Version   : Component 02.075, Driver 03.14, CPU db: 3.00.036
**     Compiler  : CodeWarrior HCS12X C Compiler
**     Date/Time : 2017/3/23, 21:47
**     Abstract  :
**         This component "BitIO" implements an one-bit input/output.
**         It uses one bit/pin of a port.
**         Note: This component is set to work in Output direction only.
**         Methods of this component are mostly implemented as a macros
**         (if supported by target language and compiler).
**     Settings  :
**         Used pin                    :
**             ----------------------------------------------------
**                Number (on package)  |    Name
**             ----------------------------------------------------
**                       116           |  PAD23_AN23
**             ----------------------------------------------------
**
**         Port name                   : AD1L
**
**         Bit number (in port)        : 7
**         Bit mask of the port        : $0080
**
**         Initial direction           : Output (direction cannot be changed)
**         Initial output value        : 1
**         Initial pull option         : off
**
**         Port data register          : PT1AD1    [$0279]
**         Port control register       : DDR1AD1   [$027B]
**
**         Optimization for            : speed
**     Contents  :
**         PutVal - void CHG_CAN_POW_PutVal(bool Val);
**
**     Copyright : 1997 - 2011 Freescale Semiconductor, Inc. All Rights Reserved.
**     
**     http      : www.freescale.com
**     mail      : support@freescale.com
** ###################################################################*/

/* MODULE CHG_CAN_POW. */

#include "CHG_CAN_POW.h"
  /* Including shared modules, which are used in the whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "Cpu.h"

#pragma DATA_SEG CHG_CAN_POW_DATA      /* Select data segment "CHG_CAN_POW_DATA" */
#pragma CODE_SEG CHG_CAN_POW_CODE
#pragma CONST_SEG CHG_CAN_POW_CONST    /* Constant section for this module */
/*
** ===================================================================
**     Method      :  CHG_CAN_POW_PutVal (component BitIO)
**
**     Description :
**         This method writes the new output value.
**     Parameters  :
**         NAME       - DESCRIPTION
**         Val             - Output value. Possible values:
**                           FALSE - logical "0" (Low level)
**                           TRUE - logical "1" (High level)
**     Returns     : Nothing
** ===================================================================
*/
void CHG_CAN_POW_PutVal(bool Val)
{
  if (Val) {
    setReg8Bits(PT1AD1, 0x80U);        /* PT1AD17=0x01U */
  } else { /* !Val */
    clrReg8Bits(PT1AD1, 0x80U);        /* PT1AD17=0x00U */
  } /* !Val */
}


/* END CHG_CAN_POW. */
/*
** ###################################################################
**
**     This file was created by Processor Expert 3.05 [04.46]
**     for the Freescale HCS12X series of microcontrollers.
**
** ###################################################################
*/
