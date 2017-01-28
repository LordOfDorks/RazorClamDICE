/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */
#ifdef __cplusplus
extern "C"
{
#endif

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SPI3_CS_Pin GPIO_PIN_2
#define SPI3_CS_GPIO_Port GPIOD
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
#ifndef NDEBUG
#define DebugPrintf(...) printf(__VA_ARGS__)
#define FULLSTOP(__WHY) FullStop(__WHY,__func__,__FILE__,__LINE__)
void FullStop(const char *why, const char *func, const char *file, int line);
#else
#define DebugPrintf(...)
#define FULLSTOP(__WHY) for(;;);
#endif

#ifdef __cplusplus
} //extern "C"
#ifndef NDEBUG
#include "wolfssl/wolfcrypt/memory.h"
typedef struct __ExceptionInfo
{
    unsigned int code;
    char *func;
    char *file;
    char *auxInfo;
    int line;
    __ExceptionInfo* inner;
} ExceptionInfo, *PExceptionInfo;

#define MX_ENTRY(__proc) \
try\
{\
    __proc;\
}\
catch(PExceptionInfo pInfo)\
{\
    PrintExceptionChain(pInfo);\
    for(;;);\
}\
catch(...)\
{\
    printf("UNHANDLED EXCEPTION!\r\n");\
    for(;;);\
}
#define MX_RETHROW(__code, __auxInfo, __inner) \
{\
    PExceptionInfo exception = (PExceptionInfo)wolfSSL_Malloc(sizeof(ExceptionInfo));\
    memset(exception, 0x00, sizeof(*exception));\
    exception->code = __code;\
    exception->func = (char *)&__func__;\
    exception->file = (char *)&__FILE__;\
    exception->auxInfo = __auxInfo;\
    exception->line = __LINE__;\
    exception->inner = __inner;\
    throw exception;\
}
#define MX_THROW(__code, __auxInfo) MX_RETHROW(__code, __auxInfo, NULL)
#define MX_ASSERT(__lvalue, __rvalue, __auxInfo) \
{\
    if(__lvalue != __rvalue) MX_THROW(0, __auxInfo);\
}
#define MX_CALL(__proc, __expected, __auxInfo) \
{\
    unsigned int result = 0;\
    if((result = __proc) != __expected)\
    {\
        MX_THROW(result, __auxInfo);\
    }\
}
#define MX_WRAP(__code, __auxInfo) \
catch(PExceptionInfo pInfo)\
{\
    MX_RETHROW(__code, __auxInfo, pInfo);\
}
void PrintExceptionChain(PExceptionInfo pInfo);
#else //#ifndef NDEBUG
#define MX_ENTRY(__proc) \
try\
{\
    __proc;\
}\
catch(...)\
{\
    for(;;);\
}
#define MX_RETHROW(__code, __auxInfo, __inner) \
{\
    throw __code;\
}
#define MX_THROW(__code, __auxInfo) MX_RETHROW(__code, __auxInfo, NULL)
#define MX_ASSERT(__lvalue, __rvalue, __auxInfo) \
{\
    if(__lvalue != __rvalue) MX_THROW(0, __auxInfo);\
}
#define MX_CALL(__proc, __expected, __auxInfo) \
{\
    unsigned int result = 0;\
    if((result = __proc) != __expected)\
    {\
        MX_THROW(result, __auxInfo);\
    }\
}
#define MX_WRAP(__code, __auxInfo) \
catch(PExceptionInfo pInfo)\
{\
    MX_RETHROW(__code, __auxInfo, pInfo);\
}
#endif //#ifndef NDEBUG
#endif //#ifdef __cplusplus
/* USER CODE END Private defines */

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
