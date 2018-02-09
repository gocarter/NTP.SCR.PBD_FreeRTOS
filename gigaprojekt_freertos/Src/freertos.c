/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"	//
#include "task.h"
#include "cmsis_os.h"
#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include "myDataTypes.h"
#include "mySensorDefines.h"

/* Variables -----------------------------------------------------------------*/
osThreadId vAutomatTaskHandle;
osThreadId vhComRxTaskHandle;
osThreadId vhComTxTaskHandle;
osTimerId processTimerHandle;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
osMailQId ComRxISRMailQueueHandle;
osMailQId AutomatNewOrderMailQueueHandle;
osMailQId ComTxMailQueueHandle;

/* Function prototypes -------------------------------------------------------*/
void StartAutomatTask(void const * argument);
void StartComRxTask(void const * argument);
void StartComTxTask(void const * argument);
void processTimerCallback(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationIdleHook(void);
void vApplicationTickHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{

}

__weak unsigned long getRunTimeCounterValue(void)
{
return 0;
}
/* USER CODE END 1 */

/* USER CODE BEGIN 2 */
__weak void vApplicationIdleHook( void )
{
   /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
   to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
   task. It is essential that code added to this hook function never attempts
   to block in any way (for example, call xQueueReceive() with a block time
   specified, or call vTaskDelay()). If the application makes use of the
   vTaskDelete() API function (as this demo application does) then it is also
   important that vApplicationIdleHook() is permitted to return to its calling
   function, because it is the responsibility of the idle task to clean up
   memory allocated by the kernel to any task that has since been deleted. */
	  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, false);

}
/* USER CODE END 2 */

/* USER CODE BEGIN 3 */
__weak void vApplicationTickHook( void )
{
   /* This function will be called by each tick interrupt if
   configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h. User code can be
   added here, but the tick hook is called from an interrupt context, so
   code must not attempt to block, and only the interrupt safe FreeRTOS API
   functions can be used (those that end in FromISR()). */
}
/* USER CODE END 3 */

/* USER CODE BEGIN 4 */
__weak void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
_Error_Handler(pcTaskName, 0);

}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
__weak void vApplicationMallocFailedHook(void)
{
   /* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created. It is also called by various parts of the
   demo application. If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information on how the remaining heap might be fragmented). */
	_Error_Handler("Malloc Failed", 0);

}
/* USER CODE END 5 */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* Create the timer(s) */
  osTimerDef(processTimer, processTimerCallback);
  processTimerHandle = osTimerCreate(osTimer(processTimer), osTimerPeriodic, NULL);

 /* Create the thread(s) */

  osThreadDef(vAutomatTask, StartAutomatTask, osPriorityHigh, 0, 1024);
  vAutomatTaskHandle = osThreadCreate(osThread(vAutomatTask), NULL);

  osThreadDef(vhComRxTask, StartComRxTask, osPriorityNormal, 0, 300);
  vhComRxTaskHandle = osThreadCreate(osThread(vhComRxTask), NULL);

  osThreadDef(vhComTxTask, StartComTxTask, osPriorityRealtime, 0, 200);
  vhComTxTaskHandle = osThreadCreate(osThread(vhComTxTask), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */

  osMailQDef(ComRxISRMailQueue, 8, message_recive_frame_type);
  ComRxISRMailQueueHandle = osMailCreate(osMailQ(ComRxISRMailQueue), NULL);

  osMailQDef(AutomatNewOrderMailQueue, 1, new_order_type);
  AutomatNewOrderMailQueueHandle = osMailCreate(osMailQ(AutomatNewOrderMailQueue), NULL);

  osMailQDef(ComTxMailQueue, 18, message_send_frame_type);
  ComTxMailQueueHandle = osMailCreate(osMailQ(ComTxMailQueue), NULL);
}


/* StartAutomatTask function */
void StartAutomatTask(void const * argument)
{
  /* USER CODE BEGIN StartAutomatTask */
	// Czujniki i ich inicjalizacja
	sensor Czujnik_Poziomu_A1={0, 0, CZ_POZIOMU_A1, DIGITAL_IN1_GPIO_Port, DIGITAL_OUT1_Pin},
			Czujnik_Poziomu_B1={0,0,CZ_POZIOMU_B1, DIGITAL_IN2_GPIO_Port, DIGITAL_IN2_Pin},
			Czujnik_Poziomu_C1={0,0,CZ_POZIOMU_C1, DIGITAL_IN3_GPIO_Port, DIGITAL_IN3_Pin},
			Czujnik_Poziomu_A2={0,0,CZ_POZIOMU_A2, DIGITAL_IN4_GPIO_Port, DIGITAL_IN4_Pin},
			Czujnik_Poziomu_B2={0,0,CZ_POZIOMU_B2, DIGITAL_IN5_GPIO_Port, DIGITAL_IN5_Pin},
			Czujnik_Poziomu_C2={0,0,CZ_POZIOMU_C2, DIGITAL_IN6_GPIO_Port, DIGITAL_IN6_Pin},
			Czujnik_Wypchniecia_A={0,0,CZ_WYPCHNIECIA_A, DIGITAL_IN7_GPIO_Port, DIGITAL_IN7_Pin},
			Czujnik_Wypchniecia_B={0,0,CZ_WYPCHNIECIA_B, DIGITAL_IN16_GPIO_Port, DIGITAL_IN16_Pin},
			Czujnik_Wypchniecia_C={0,0,CZ_WYPCHNIECIA_C, DIGITAL_IN17_GPIO_Port, DIGITAL_IN17_Pin},
			Czujnik_Pozycji_A={0,0,CZ_POZYCJI_A, DIGITAL_IN10_GPIO_Port, DIGITAL_IN10_Pin},
			Czujnik_Pozycji_B={0,0,CZ_POZYCJI_B, DIGITAL_IN11_GPIO_Port, DIGITAL_IN11_Pin},
			Czujnik_Pozycji_C={0,0,CZ_POZYCJI_C, DIGITAL_IN18_GPIO_Port, DIGITAL_IN18_Pin},
			Czujnik_Pozycji_Koncowej={0,0,CZ_POZYCJI_KONCOWEJ, DIGITAL_IN13_GPIO_Port, DIGITAL_IN13_Pin},
			Przycisk_Bezpieczenstwa={0,0,PRZYCISK_BEZPIECZENSTWA, DIGITAL_IN14_GPIO_Port, DIGITAL_IN14_Pin},
			Przycisk_Gotowosci={0,0,PRZYCISK_GOTOWOSCI, DIGITAL_IN15_GPIO_Port, DIGITAL_IN15_Pin};

	sensor *sensorPtrTab[NUMBER_OF_SENSORS]={&Czujnik_Poziomu_A1, &Czujnik_Poziomu_B1, &Czujnik_Poziomu_C1,
											&Czujnik_Poziomu_A2, &Czujnik_Poziomu_B2, &Czujnik_Poziomu_C2,
											&Czujnik_Wypchniecia_A, &Czujnik_Wypchniecia_B, &Czujnik_Wypchniecia_C,
											&Czujnik_Pozycji_A, &Czujnik_Pozycji_B, &Czujnik_Pozycji_C,
											&Czujnik_Pozycji_Koncowej, &Przycisk_Bezpieczenstwa, &Przycisk_Gotowosci};

	//wyjœcia i ich inicjalizacja
	output Tlok_A={0, DIGITAL_OUT1_GPIO_Port, DIGITAL_OUT1_Pin},
			Tlok_B={0, DIGITAL_OUT2_GPIO_Port, DIGITAL_OUT2_Pin},
			Tlok_C={0, DIGITAL_OUT3_GPIO_Port, DIGITAL_OUT3_Pin},
			Silnik_Tasma={0, DIGITAL_OUT4_GPIO_Port, DIGITAL_OUT4_Pin},
			Kontrolka_A={0, DIGITAL_OUT5_GPIO_Port, DIGITAL_OUT5_Pin},
			Kontrolka_B={0, DIGITAL_OUT6_GPIO_Port, DIGITAL_OUT6_Pin},
			Kontrolka_C={0, DIGITAL_OUT7_GPIO_Port, DIGITAL_OUT7_Pin};

	output *outputPtrTab[NUMBER_OF_OUTPUTS]={&Tlok_A, &Tlok_B, &Tlok_C, &Silnik_Tasma, &Kontrolka_A, &Kontrolka_B, &Kontrolka_C};

	//reszta zmiennych automatu
	//Liczniki i Timery
	char	Timer_Opoznienia_A = 0, Timer_Opoznienia_B = 0, Timer_Opoznienia_C = 0,
			Stan_Podajnik_A = 1, Stan_Podajnik_B = 1, Stan_Podajnik_C = 1,
			Stan_Tasmociag = 1;
	//zmienne iloœci cukierków
	uint16_t A = 0, B = 0, C = 0;

	//zmienne wejœciowe
	osEvent sig, msg;
	new_order_type *order = NULL;
	message_send_frame_type *tmpMailPtr;
	volatile bool working = 0;

  /* Infinite loop */
  for(;;)
  {
	  sig = osSignalWait(0x00,osWaitForever);
	  HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin, true);
	  switch(sig.value.signals){
	  case PAUSE_SIGNAL:
		  working = 0;
		  break;
	  case RESET_SIGNAL:
		  working = 0;
		  osMailFree(AutomatNewOrderMailQueueHandle, order);
		  Tlok_A.value = 0;
		  Tlok_B.value = 0;
		  Tlok_C.value = 0;
		  Timer_Opoznienia_A = 0;
		  Timer_Opoznienia_B = 0;
		  Timer_Opoznienia_C = 0;
		  Stan_Podajnik_A = 1;
		  Stan_Podajnik_B = 1;
		  Stan_Podajnik_C = 1;
		  Stan_Tasmociag = 1;
		  osTimerStop(processTimerHandle);
		  break;
	  case MAIL_SIGNAL:
		  msg = osMailGet(AutomatNewOrderMailQueueHandle, 10);
		  order = msg.value.p;
		  A = order->quantity_A;
		  B = order->quantity_B;
		  C = order->quantity_C;
		  working = 1;
		  /* no break */
	  case START_SIGNAL:
	  		  working = 1;
	  		  //inicjalizacja timera procesu
	  		  osTimerStart(processTimerHandle, 100);
	  		  break;
	  case TICK_SIGNAL:
		  //odczytanie wejsc
		  for(int i=0; i<NUMBER_OF_SENSORS;i++){
			  sensorPtrTab[i]->value = HAL_GPIO_ReadPin(sensorPtrTab[i]->port, sensorPtrTab[i]->pin);
			  if(sensorPtrTab[i]->value != sensorPtrTab[i]->prev_value){//wys³anie maila do procesy vhComTx z now¹ wartoœci¹ czujnika
				  tmpMailPtr = osMailAlloc(ComTxMailQueueHandle, 10);
				  tmpMailPtr->type=SENSOR_VALUE;
				  tmpMailPtr->sensor_change.id=sensorPtrTab[i]->id;
				  tmpMailPtr->sensor_change.value=sensorPtrTab[i]->value;
				  osMailPut(ComTxMailQueueHandle, tmpMailPtr);
				  osSignalSet(vhComTxTaskHandle, MAIL_SIGNAL);
				  sensorPtrTab[i]->prev_value=sensorPtrTab[i]->value;
			  }
		  }

		  if(working && !Przycisk_Bezpieczenstwa.value){

			  //obliczenie wyjsc na podstawie stanu automatu
			  if(Czujnik_Pozycji_A.value && Stan_Tasmociag == 3){
    			switch (Stan_Podajnik_A){

    			case 1:	Tlok_A.value = 0;
    					if(A!=0) Stan_Podajnik_A = 2;
    					break;

    			case 2:	Tlok_A.value = 0;
    					if(Czujnik_Poziomu_A1.value == 1) Stan_Podajnik_A = 3;
    					if(Czujnik_Poziomu_A1.value == 0) Stan_Podajnik_A = 6;
    					break;

    			case 3:	Tlok_A.value = 1;
    					if(Czujnik_Wypchniecia_A.value == 1) Stan_Podajnik_A = 4;
    					break;

    			case 4:	Tlok_A.value = 0;
    					if(Czujnik_Wypchniecia_A.value == 0) {Stan_Podajnik_A = 5; A--;}
    					break;

    			case 5:	Tlok_A.value = 0;
    					if(A!=0) Stan_Podajnik_A = 2;
    					if(A==0) Stan_Podajnik_A = 1;
    					break;

    			case 6:	Tlok_A.value = 0;
    					if(Czujnik_Poziomu_A1.value == 1) Stan_Podajnik_A = 1;
    					break;

    			}
    		}

    		if(Czujnik_Pozycji_B.value && Stan_Tasmociag == 6){
    			switch (Stan_Podajnik_B){

    			case 1:	Tlok_B.value = 0;
    				if(B!=0) Stan_Podajnik_B = 2;
    				break;

    			case 2:	Tlok_B.value = 0;
    				if(Czujnik_Poziomu_B1.value == 1) Stan_Podajnik_B = 3;
    				if(Czujnik_Poziomu_B1.value == 0) Stan_Podajnik_B = 6;
    				break;

    			case 3:	Tlok_B.value = 1;
    				if(Czujnik_Wypchniecia_B.value == 1) Stan_Podajnik_B = 4;
    				break;

    			case 4:	Tlok_B.value = 0;
    				if(Czujnik_Wypchniecia_B.value == 0) {Stan_Podajnik_B = 5; B--;}
    				break;

    			case 5:	Tlok_B.value = 0;
    				if(B!=0) Stan_Podajnik_B = 2;
    				if(B==0) Stan_Podajnik_B = 1;
    				break;

    			case 6:	Tlok_B.value = 0;
    				if(Czujnik_Poziomu_B1.value == 1) Stan_Podajnik_B = 1;
    				break;
    			}
    		}

    		if(Czujnik_Pozycji_C.value && Stan_Tasmociag == 9){
    			switch (Stan_Podajnik_C){

    			case 1:	Tlok_C.value = 0;
    				if(C!=0) Stan_Podajnik_C = 2;
    				break;

    			case 2:	Tlok_C.value = 0;
    				if(Czujnik_Poziomu_C1.value == 1) Stan_Podajnik_C = 3;
    				if(Czujnik_Poziomu_C1.value == 0) Stan_Podajnik_C = 6;
    				break;

    			case 3:	Tlok_C.value = 1;
    				if(Czujnik_Wypchniecia_C.value == 1) Stan_Podajnik_C = 4;
    				break;

    			case 4:	Tlok_C.value = 0;
    				if(Czujnik_Wypchniecia_C.value == 0) {Stan_Podajnik_C = 5; C--;}
    				break;

    			case 5:	Tlok_C.value = 0;
    				if(C!=0) Stan_Podajnik_C = 2;
    				if(C==0) Stan_Podajnik_C = 1;
    				break;

    			case 6:	Tlok_C.value = 0;
    				if(Czujnik_Poziomu_C1.value == 1) Stan_Podajnik_C = 1;
    				break;

    			}
    		}

    		switch (Stan_Tasmociag){

    		case 1: 	Silnik_Tasma.value = 0;
    				if(Przycisk_Gotowosci.value == 1) Stan_Tasmociag = 2;
    				break;

    		case 2:	Silnik_Tasma.value = 1;
    				if(Czujnik_Pozycji_A.value == 1) Stan_Tasmociag = 3;
    				break;

    		case 3: 	Silnik_Tasma.value = 0;
    				if(A == 0) {Stan_Tasmociag = 4; Timer_Opoznienia_A = TIMER_A;}
    				break;

    		case 4:	Silnik_Tasma.value = 0;								// U¿yty timer, nie wiadomo jak postawiæ warunek by
    				if(Timer_Opoznienia_A == 0) Stan_Tasmociag = 5;	// to dzia³a³o jak nale¿y
    				break;

    		case 5: 	Silnik_Tasma.value = 1;
    				if(Czujnik_Pozycji_B.value == 1) Stan_Tasmociag = 6;
    				break;

    		case 6:	Silnik_Tasma.value = 0;
    				if(B == 0) {Stan_Tasmociag = 7; Timer_Opoznienia_B = TIMER_B;}
    				break;

    		case 7: 	Silnik_Tasma.value = 0;								// U¿yty timer, nie wiadomo jak postawiæ warunek by
    				if(Timer_Opoznienia_B == 0) Stan_Tasmociag = 8;	// to dzia³a³o jak nale¿y
    				break;

    		case 8:	Silnik_Tasma.value = 1;
    				if(Czujnik_Pozycji_C.value == 1) Stan_Tasmociag = 9;
    				break;

    		case 9: Silnik_Tasma.value = 0;
    				if(C == 0) {Stan_Tasmociag = 10; Timer_Opoznienia_C = TIMER_C;}
    				break;

    		case 10: 	Silnik_Tasma.value = 0;
    				if(Timer_Opoznienia_C == 0) Stan_Tasmociag = 11;
    				break;

    		case 11:	Silnik_Tasma.value = 1;
    				if(Czujnik_Pozycji_Koncowej.value == 1) Stan_Tasmociag = 12;
    				break;

    		case 12: Silnik_Tasma.value = 0;
    				if(Czujnik_Pozycji_Koncowej.value == 0) {
    					Stan_Tasmociag = 1;
    					// zamówienie kompletne, mo¿na wys³ac potwierdzenie
    				    tmpMailPtr = osMailAlloc(ComTxMailQueueHandle, 10);
    				    tmpMailPtr->type = ORDER_COMPLETE;
    				    tmpMailPtr->order_complete.quantity_A = order->quantity_A;
    				   	tmpMailPtr->order_complete.quantity_B = order->quantity_B;
    				   	tmpMailPtr->order_complete.quantity_C = order->quantity_C;
    				    osMailPut(ComTxMailQueueHandle, tmpMailPtr);
    				    osMailFree(AutomatNewOrderMailQueueHandle, order);
    				    osSignalSet(vhComTxTaskHandle, MAIL_SIGNAL);
    				}
    				break;
    		}
    		if(Timer_Opoznienia_A)Timer_Opoznienia_A--;
    		if(Timer_Opoznienia_B)Timer_Opoznienia_B--;
    		if(Timer_Opoznienia_C)Timer_Opoznienia_C--;
		  }
		  else Silnik_Tasma.value = 0;

		  //zapalanie kontrolek sygnalizuj¹cych niski poziom cukierków w pojemniku
		  Kontrolka_A.value = !Czujnik_Poziomu_A2.value;
		  Kontrolka_B.value = !Czujnik_Poziomu_B2.value;
		  Kontrolka_C.value = !Czujnik_Poziomu_C2.value;

		  //w przypadku braku cukierków wysylany jest komunikat oraz zatrzymywany jest proces
		  if((Stan_Podajnik_A == 6 || Stan_Podajnik_B == 6 || Stan_Podajnik_C == 6) && working){

			  working = false; //zamro¿enie procesu

			  //tworzenie komunikatu z informacj¹ jakie pojemniki sa puste
			  tmpMailPtr = osMailAlloc(ComTxMailQueueHandle, 10);
			  tmpMailPtr->type = EMPTY_CONTAINER;
			  tmpMailPtr->empty_container.A = (Stan_Podajnik_A == 6);
			  tmpMailPtr->empty_container.B = (Stan_Podajnik_B == 6);
			  tmpMailPtr->empty_container.C = (Stan_Podajnik_C == 6);

			  //wys³anie maila do procesu komunikacyjnego, oraz ustawienie sygna³y ¿e mail zota³ wys³any
			  osMailPut(ComTxMailQueueHandle, tmpMailPtr);
			  osSignalSet(vhComTxTaskHandle, MAIL_SIGNAL);
		  }

		  //ustawienie wyjsc zgodnie ze zmiennymi automatu
		  for(int i=0; i<NUMBER_OF_OUTPUTS;i++) HAL_GPIO_WritePin(outputPtrTab[i]->port, outputPtrTab[i]->pin, outputPtrTab[i]->value);
		  break;
    	}

  }
  /* USER CODE END StartAutomatTask */
}

/* StartComRxTask function */
void StartComRxTask(void const * argument)
{
  /* USER CODE BEGIN StartComRxTask */
	osEvent msg;
	message_recive_frame_type *message;
	new_order_type *newOrderTmp; //tymczasowe zmienne ilosci cukierkow

  /* Infinite loop */
  for(;;)
  {
	  msg = osMailGet(ComRxISRMailQueueHandle, osWaitForever); //oczekiwanie na wiadomosc
	  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, true);
	  if(msg.status==osEventMail){//przysz³a wiadmosc
		  message = msg.value.p;
		  if(message->m_start == '#'){
			  switch(message->m_type){
			  case 'Q': //odebrane zamówienie z iloœcia cukierkow
				  if((message->order.hundred_digit_A>='0' && message->order.hundred_digit_A<='9') &&
						  (message->order.hundred_digit_B>='0' && message->order.hundred_digit_B<='9') &&
						  (message->order.hundred_digit_C>='0' && message->order.hundred_digit_C<='9') &&
						  (message->order.ten_digit_A>='0' && message->order.ten_digit_A<='9') &&
						  (message->order.ten_digit_B>='0' && message->order.ten_digit_B<='9') &&
						  (message->order.ten_digit_C>='0' && message->order.ten_digit_C<='9') &&
						  (message->order.unit_digit_A>='0' && message->order.unit_digit_A<='9') &&
						  (message->order.unit_digit_B>='0' && message->order.unit_digit_B<='9') &&
						  (message->order.unit_digit_C>='0' && message->order.unit_digit_C<='9') &&
						  message->order.end_of_frame == '@') { //sprawdzenie poprwnoœci ramki

					  newOrderTmp = osMailAlloc(AutomatNewOrderMailQueueHandle, 0);
					  if(newOrderTmp != NULL){
						  newOrderTmp->quantity_A = (message->order.hundred_digit_A-'0')*100 + (message->order.ten_digit_A-'0')*10 + (message->order.unit_digit_A-'0');
						  newOrderTmp->quantity_B = (message->order.hundred_digit_B-'0')*100 + (message->order.ten_digit_B-'0')*10 + (message->order.unit_digit_B-'0');
						  newOrderTmp->quantity_C = (message->order.hundred_digit_C-'0')*100 + (message->order.ten_digit_C-'0')*10 + (message->order.unit_digit_C-'0');
						  osMailPut(AutomatNewOrderMailQueueHandle, newOrderTmp);
						  osSignalSet(vAutomatTaskHandle, MAIL_SIGNAL);
						  osSignalSet(vhComTxTaskHandle, ECHO_SIGNAL);
					  }
					  else {
						  //jakieœ zamówienie jest jeszcze wykonywane, jezeli nie to zalecane zresetowanie automatu
						  osSignalSet(vhComTxTaskHandle, BAD_FRAME_SIGNAL);
					  }
				  }
				  else {
					  osSignalSet(vhComTxTaskHandle, BAD_FRAME_SIGNAL);
				  }
				  break;

			  case 'D': //odebrana komenda wznowienia/wstrzymania/resetu
				  if(message->command.end_of_frame == '@')
					  switch(message->command.command){
					  case 'w':
						  osSignalSet(vAutomatTaskHandle, PAUSE_SIGNAL);
						  osSignalSet(vhComTxTaskHandle, ECHO_SIGNAL);
						  break;
					  case 's':
						  osSignalSet(vAutomatTaskHandle, START_SIGNAL);
						  osSignalSet(vhComTxTaskHandle, ECHO_SIGNAL);
						  break;
					  case 'r':
						  osSignalSet(vAutomatTaskHandle, RESET_SIGNAL);
						  osSignalSet(vhComTxTaskHandle, ECHO_SIGNAL);
						  break;
					  default:
						  osSignalSet(vhComTxTaskHandle, BAD_FRAME_SIGNAL);
						  break;
					  }
				  else {
					  osSignalSet(vhComTxTaskHandle, BAD_FRAME_SIGNAL);
				  }
				  break;

			  case '@': //odebrane echo
				  osSignalSet(vhComTxTaskHandle, ECHO_SIGNAL);
				  break;

			  default: //gdy nie zgadza siê drugi znak ramki
				  osSignalSet(vhComTxTaskHandle, BAD_FRAME_SIGNAL);
				  break;
			  }

		  }
		  else {// gdy nie zgadza sie znak pocz¹tku ramki
			  osSignalSet(vhComTxTaskHandle, BAD_FRAME_SIGNAL);
		  }
		  osMailFree(ComRxISRMailQueueHandle, message);
	  }

  }
  /* USER CODE END StartComRxTask */
}


/* StartComTxTask function */
void StartComTxTask(void const * argument)
{
  /* USER CODE BEGIN StartComTxTask */
	osEvent msg, sig;
	char buffor[17];
	char signal;
	message_send_frame_type *message;
  /* Infinite loop */
  for(;;)
  {
	  sig = osSignalWait(0x0, osWaitForever);
	  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, true);
	  for(int i=0; i<17;i++) buffor[i]='\0';
	  if(sig.status == osEventSignal){
		  switch(sig.value.signals){
		  case BAD_FRAME_SIGNAL:
			  signal = '!';
			  snprintf(&buffor, 17, "#%c", signal);
			  //osSignalClear(vhComTxTaskHandle,BAD_FRAME_SIGNAL);
			  break;
		  case ECHO_SIGNAL:
			  signal = '@';
			  snprintf(&buffor, 17, "#%c", signal);
			  //osSignalClear(vhComTxTaskHandle,ECHO_SIGNAL);
			  break;
		  case MAIL_SIGNAL:
			  msg = osMailGet(ComTxMailQueueHandle, osWaitForever);
			  message = msg.value.p;
			  switch(message->type){
			  case ORDER_COMPLETE:
			 	  snprintf(&buffor, 17, "#Q/%d/%d/%d/@",message->order_complete.quantity_A, message->order_complete.quantity_B, message->order_complete.quantity_C);
			 	  break;
			  case EMPTY_CONTAINER:
				  snprintf(&buffor, 17, "#E/%d/%d/%d/@",message->empty_container.A, message->empty_container.B, message->empty_container.C);
				  break;
			  case SENSOR_VALUE:
			 	  snprintf(&buffor, 17, "#S/%d/%d/@",message->sensor_change.id, message->sensor_change.value);
			 	  break;
			  default:
				  break;
			  }
			  osMailFree(ComTxMailQueueHandle, message);
			  break;
		  default:
			  signal = '*';
			  snprintf(&buffor, 17, "#%c", signal);
			  break;
		  }
		  HAL_UART_Transmit_DMA(&huart1, &buffor, 16);
		  //HAL_UART_Transmit_DMA(&huart2, &buffor, 16);

	  }

  }
  /* USER CODE END StartComTxTask */
}

/* processTimerCallback function */
void processTimerCallback(void const * argument)
{
  /* USER CODE BEGIN processTimerCallback */
  osSignalSet(vAutomatTaskHandle, TICK_SIGNAL);
  /* USER CODE END processTimerCallback */
}

/* USER CODE BEGIN Application */
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
