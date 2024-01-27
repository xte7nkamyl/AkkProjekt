/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "lcd.h"
#include "LCD_Keypad.h"
#include "stdio.h"
#include <stdio.h>
#include "stdlib.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_RTC_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch)
{
  if (ch == '\n') {
    __io_putchar('\r');
  }

  HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);

  return 1;
}

typedef struct tevent{
	int id;
  int day;
  int month;
  int year;
  int hour;
  int minutes;
  int hour_k;
  int minutes_k;
  char description[17];
}event;

typedef struct tnode
{
	event data;
	struct tnode* next;
}node;

node** element;

void list_add_event(node** element, event* e)
{
	node* n = (node*)malloc(sizeof(node));
	n->data = *e;
	n->next = *element;
	*element = n;
}

void list_print(node* element)
{
	while(element)
	{
		printf("%d %s %d/%d/%d %d:%d-%d:%d \n", element->data.id,element->data.description,element->data.day,element->data.month,element->data.year,element->data.hour,element->data.minutes,element->data.hour_k,element->data.minutes_k);
		element = element->next;
	}


}

void list_remove_by_id(node** element,int event_id)
{
	if(*element == NULL)
	{
		return;
	}
	node* prev = NULL;
	node* curr = *element;
	while(curr)
	{
		if(curr->data.id == event_id)
		{
			if(prev)
			{
				prev->next = curr->next;
			}
			else {
				*element = (*element)->next;
			}
			node *tmp = curr;
			curr = curr->next;
			free(tmp);
		}
		else {
			prev = curr;
			curr = curr->next;
		}
	}
}

event* list_find_by_id(node* element, int event_id) {
    while (element) {
        if (element->data.id == event_id)
            return &element->data;
        element = element->next;
    }
    return NULL;
}

#define LINE_MAX_LENGTH	80

static char line_buffer[LINE_MAX_LENGTH + 1];
static uint32_t line_length;

void line_append(uint8_t value)
{
	if (value == '\r' || value == '\n') {
		// odebraliśmy znak końca linii
		if (line_length > 0) {
			// jeśli bufor nie jest pusty to dodajemy 0 na końcu linii
			line_buffer[line_length] = '\0';
			// przetwarzamy dane
			printf("Otrzymano: %s\n", line_buffer);
			// zaczynamy zbieranie danych od nowa
			line_length = 0;
		}
	}
	else {
		if (line_length >= LINE_MAX_LENGTH) {
			// za dużo danych, usuwamy wszystko co odebraliśmy dotychczas
			line_length = 0;
		}
		// dopisujemy wartość do bufora
		line_buffer[line_length++] = value;
	}
}


void read(const char* promt)
{
	printf("%s", promt);
	fflush(stdout);
	uint8_t value;

	while(1){
		if (HAL_UART_Receive(&huart2, &value, 1, 0) == HAL_OK){
			  line_append(value);
			  if (value == '\r' || value == '\n') break;
		}
	}

	fflush(stdin);
}

int check_free_time(node* element, int suma_min_p, int suma_min_k)
{
	while(element)
	{
		int sum_p = element->data.hour*60+element->data.minutes;
		int sum_k = element->data.hour_k*60+element->data.minutes_k;
		if((suma_min_p>=sum_p && suma_min_p <=sum_k) )
		{
			return 1;
		}
		element = element->next;
	}
	return 0;
}

void add_event(node** element)
{
	uint8_t value;
	event e;
	int suma_min_p,suma_min_k;
	do{
		read("\nPodaj id osoby> ");
		e.id = atoi(line_buffer);
		strcpy(line_buffer, "");
	}while(e.id < 1);

	do{
		read("\nPodaj nazwe eventu> ");
		for (int i = 0; line_buffer[i] != '\0'; i++)
			e.description[i] = line_buffer[i];

		strcpy(line_buffer, "");
		if (strlen(e.description)<3)
			printf("Zbyt krotka nazwa wydarzenia \n");
	}while(strlen(e.description)<3);

	do{
		read("\nPodaj dzien> ");
		e.day = atoi(line_buffer);
		strcpy(line_buffer, "");
		if (e.day <1 || e.day>31)
			printf("Nie ma takiego dnia \n");
	}while(e.day <1 || e.day>31);

	do{
		read("\nPodaj miesiac> ");
		e.month = atoi(line_buffer);
		strcpy(line_buffer, "");
		if (e.month<1 || e.month>12)
			printf("Nie ma takiego miesiaca \n");
	}while(e.month<1 || e.month>12);
	do{
		read("\nPodaj rok> ");
		e.year = atoi(line_buffer);
		strcpy(line_buffer, "");
	}while(e.year<2024);
	int x=0;
	do{
		do{
	      read("\nPodaj godzine> ");
	      e.hour = atoi(line_buffer);
	      strcpy(line_buffer, "");
	      if(e.hour>23 || e.hour<0)
	    	  printf("Niewlasciwa godzina \n");
	     }while(e.hour>23 || e.hour<0);
	    do{
	      read("\nPodaj minute ");
	      e.minutes = atoi(line_buffer);
	      strcpy(line_buffer, "");
	      if(e.minutes>59 || e.minutes<0)
	      	  printf("Niewlasciwa minuta \n");
	     } while(e.minutes>59 || e.minutes<0);
	    suma_min_p = e.hour*60+e.minutes;

	 do
	    {
	    do{
	      read("\nPodaj godzine konca> ");
	      e.hour_k = atoi(line_buffer);
	      strcpy(line_buffer, "");
	      if(e.hour_k>23 || e.hour_k<0)
	    	  printf("Niewlasciwa godzina \n");
	     } while(e.hour_k>23 || e.hour_k<0);
	    do{
	      read("\nPodaj minute konca> ");
	      e.minutes_k = atoi(line_buffer);
	      strcpy(line_buffer, "");
	      if(e.minutes_k>59 || e.minutes_k<0)
	      	  printf("Niewlasciwa minuta \n");
	     } while(e.minutes_k>59 || e.minutes_k<0);
	      suma_min_k = e.hour_k*60+e.minutes_k;
	      if(suma_min_p>suma_min_k)
	    	  printf("Czas konca wydarzenia jest mniejszy niz jego poczatek \n");
	    }while(suma_min_p>suma_min_k);
	 x = check_free_time(*element, suma_min_p,suma_min_k);
	 if(x == 1)
		 printf("Zajety termin \n");
	}while(x==1);

	list_add_event(element, &e);
	printf("\nEvent o id: %d zostala dodana\n",e.id);
}


void remove_event(node** element)
{
	int event_id;
	read("\nPodaj id osoby> ");
	event_id = atoi(line_buffer);
	strcpy(line_buffer, "");

	event* e = list_find_by_id(*element, event_id);
	  if(e == NULL)
	  {
	    printf("Brak wydarzenia o id: %d\n",event_id);
	    return;
	  }
	  list_remove_by_id(element, event_id);
	  printf("usunieta wydarzenie o id %d\n", event_id);
}

void edit_event(node* element)
{
  int id;
  char tab[17];
  int zmienna;
  int suma_min_p,suma_min_k;
  read("\nPodaj id osoby> ");
  id = atoi(line_buffer);
  strcpy(line_buffer, "");
  event *e = list_find_by_id(element, id);
  if(e == NULL)
  {
    printf("Brak wydarzenia o id: %d\n",id);
    return;
  }
  do{
  		read("\nPodaj nazwe eventu> ");
  		for (int i = 0; line_buffer[i] != '\0'; i++)
  			tab[i] = line_buffer[i];

  		strcpy(line_buffer, "");
  		if (strlen(tab)<3)
  			printf("Zbyt krotka nazwa wydarzenia \n");
  	}while(strlen(tab)<3);
  	strcpy(e->description,tab);

  	do{
  		read("\nPodaj dzien> ");
  		zmienna = atoi(line_buffer);
  		strcpy(line_buffer, "");
  		if (zmienna <1 || zmienna>31)
  			printf("Nie ma takiego dnia \n");
  	}while(zmienna <1 || zmienna>31);
  	e->day = zmienna;

  	do{
  		read("\nPodaj miesiac> ");
  		zmienna = atoi(line_buffer);
  		strcpy(line_buffer, "");
  		if (zmienna<1 || zmienna>12)
  			printf("Nie ma takiego miesiaca \n");
  	}while(zmienna<1 || zmienna>12);
  	e->month = zmienna;
  	do{
  		read("\nPodaj rok> ");
  		zmienna = atoi(line_buffer);
  		strcpy(line_buffer, "");
  	}while(zmienna<2024);
  	e->year = zmienna;

  	int x=0;
  	do{
  		do{
  		    read("\nPodaj godzine> ");
  		    zmienna = atoi(line_buffer);
  		    strcpy(line_buffer, "");
  		    if(zmienna>23 || zmienna<0)
  		    	  printf("Niewlasciwa godzina \n");
  		  }while(zmienna>23 || zmienna<0);
  		e->hour = zmienna;
  		  do{
  		      read("\nPodaj minute ");
  		      zmienna = atoi(line_buffer);
  		      strcpy(line_buffer, "");
  		      if(zmienna>59 || zmienna<0)
  		      	  printf("Niewlasciwa minuta \n");
  		     } while(zmienna>59 || zmienna<0);
  		  e->minutes = zmienna;
  		    suma_min_p = e->hour*60+e->minutes;

  		 do {
  		    do{
  		      read("\nPodaj godzine konca> ");
  		      zmienna = atoi(line_buffer);
  		      strcpy(line_buffer, "");
  		      if(zmienna>23 || zmienna<0)
  		    	  printf("Niewlasciwa godzina \n");
  		     } while(zmienna>23 || zmienna<0);
  		    e->hour_k = zmienna;
  		    do{
  		      read("\nPodaj minute konca> ");
  		      zmienna = atoi(line_buffer);
  		      strcpy(line_buffer, "");
  		      if(zmienna>59 || zmienna<0)
  		      	  printf("Niewlasciwa minuta \n");
  		     } while(zmienna>59 || zmienna<0);
  		    e->minutes_k = zmienna;
  		      suma_min_k = e->hour_k*60+e->minutes_k;
  		      if(suma_min_p>suma_min_k)
  		    	  printf("Czas konca wydarzenia jest mniejszy niz jego poczatek \n");
  		    }while(suma_min_p>suma_min_k);
  		 x = check_free_time(element, suma_min_p,suma_min_k);
  		 if(x == 1)
  			 printf("Zajety termin \n");
  		}while(x==1);

  printf("Zmodyfikowano dane wydarzenia o id: %d\n", e->id);
}

void menu(node** element){
	char ch;
	do{

		printf("\n1 Dodaj event\n");
		printf("2 wyswietl liste\n");
		printf("3 usun po id\n");
		printf("4 edytuj po id\n");
		printf("5 zapisz zmiany\n");
		//scanf("%d", &option);
		while(1){
			if (HAL_UART_Receive(&huart2, &ch, 1, 0) == HAL_OK){
				if(ch == '1' || ch == '2' || ch == '3' || ch == '4' || ch == '5') break;
			}
		}
		fflush(stdin);

		switch(ch)
		{
		case '1':
			add_event(element);
			break;
		case '2':
			list_print(*element);
			break;
		case '3':
			remove_event(element);
			break;
		case '4':
			edit_event(*element);
			break;
		case '5':
			return;
		default:
			printf("brak opcji");
			break;
		}

	} while(ch != '5');
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  if(GPIO_Pin == B1_Pin)  menu(&element);
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_RTC_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  LCD_init();
  KPAD_init(&hadc1);
  HAL_ADC_Start(&hadc1);
  lcd_init(_LCD_4BIT,_LCD_FONT_5x8, _LCD_2LINE);

  RTC_TimeTypeDef RtcTime;
  RTC_DateTypeDef RtcDate;

  char tab[17];
  char description1[17];
  		char description2[17];
  char message[17];
  int flaga = 0;
  int stan=1;
  int minuta, godzina,suma_minut_p, suma_minut_k, suma_minut_zegara;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
     HAL_ADC_Start(&hadc1);
	 HAL_Delay(1000);
	  if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
	  {
		 if(KPAD_IsKeyPressed()){
		  	 int buttonState2 = KPAD_getkey();
		  	 if(buttonState2 == KPAD_KEYRIGHT){
		  		 if(stan ==1)
		  			 stan = 0;
		  	  	  else
		  	  			stan =1;
		  	  	}
		    }
		if(stan ==0){
			lcd_clear();
		    node* current2 = element;
		    while(current2){
		 		  strcpy(description2,current2->data.description);
		 		  lcd_print(1, 1, description1);
		 		  lcd_print(2, 1,description2);
		 		  if(KPAD_IsKeyPressed()){
		 		 	int buttonState3 = KPAD_getkey();
		 		 	if(buttonState3 == KPAD_KEYDOWN){
		 		 		current2 = current2->next;
		 		 	 	strcpy(description1 ,description2);
		 		 	 	if (current2 == NULL) {
		 		 	 		current2 = element;
		 		 	 	}
		 		 	}else if(buttonState3 == KPAD_KEYRIGHT)
		 		 	{
		 		 		stan=1;
		 		 		break;
		 		 	}
		 		  }
		    }
		 }
		 else if(stan == 1){
	    lcd_clear();
	    HAL_RTC_GetTime(&hrtc, &RtcTime, RTC_FORMAT_BIN);
	    HAL_RTC_GetDate(&hrtc, &RtcDate, RTC_FORMAT_BIN);

	    sprintf(tab, "%04d-%02d-%02d %02d:%02d", 2000 + RtcDate.Year, RtcDate.Month, RtcDate.Date, RtcTime.Hours, RtcTime.Minutes);
	    lcd_print(2, 1, tab);

	    node* current = element;
	  	while(current)
	  	{
	  		suma_minut_zegara = RtcTime.Hours*60+RtcTime.Minutes;
	  		suma_minut_p = current->data.hour*60+current->data.minutes;
	  		suma_minut_k = current->data.hour_k*60+current->data.minutes_k;
	  		minuta = current->data.minutes-1;
	  		godzina = current->data.hour;
	  		if (minuta < 0) {
	  		        minuta += 60;
	  		        godzina--;
	  		    }
	  		if(current->data.day == RtcDate.Date &&
	  	 	  	    	current->data.month == RtcDate.Month &&
	  	 				current->data.year == RtcDate.Year + 2000 &&
						suma_minut_k <= suma_minut_zegara){
	  			list_remove_by_id(&element, current->data.id);
	  			lcd_print(1, 1, "Zakonczono");
	  			HAL_Delay(1000);
	  			LCD_clear();
	  			 break;
	  		}

	  		 if (current->data.day == RtcDate.Date &&
	  			  	 	  	    	current->data.month == RtcDate.Month &&
	  			  	 				current->data.year == RtcDate.Year + 2000 &&
	  			  	 				suma_minut_p <= suma_minut_zegara &&
	  			  	 				suma_minut_k >= suma_minut_zegara) {

	  			 flaga = 1;
	  			  if(KPAD_IsKeyPressed()){
	  				  int buttonState = KPAD_getkey();
	  				  if(buttonState == KPAD_KEYSELECT){
	  					  list_remove_by_id(&element, current->data.id);
	  					  lcd_print(1, 1, "Zakonczono");
	  					  HAL_Delay(1000);
	  					  LCD_clear();
	  					  break;
	  				  }
	  			  }
	  		 }else if (current->data.day == RtcDate.Date &&
	  	    	current->data.month == RtcDate.Month &&
				current->data.year == RtcDate.Year + 2000 &&
				godzina == RtcTime.Hours &&
				minuta == RtcTime.Minutes) {
	  	    	flaga = 2;
	  	      }
	  		 else {
	  			 flaga = 0;
	  		 }

	  	    if(flaga == 1){
	  	    	lcd_print(1, 1, "TRWA: ");
	  	    	lcd_print(1, 7, current->data.description);

	  	    }else if(flaga == 2){
	  	    	lcd_print(1, 1, "PRZYP: ");
	  	    	lcd_print(1, 8, current->data.description);
	  	    }

	  	  current = current->next;
	  	 }
	  }
	 }

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_LSE
                              |RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x12;
  sTime.Minutes = 0x58;
  sTime.Seconds = 0x10;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_TUESDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x9;
  sDate.Year = 0x24;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXOVERRUNDISABLE_INIT;
  huart2.AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LD2_Pin|LCD_D7_Pin|LCD_RS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_D6_Pin|LCD_D5_Pin|LCD_D4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD2_Pin LCD_D7_Pin LCD_RS_Pin */
  GPIO_InitStruct.Pin = LD2_Pin|LCD_D7_Pin|LCD_RS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_D6_Pin LCD_D5_Pin LCD_D4_Pin */
  GPIO_InitStruct.Pin = LCD_D6_Pin|LCD_D5_Pin|LCD_D4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_EN_Pin */
  GPIO_InitStruct.Pin = LCD_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_EN_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
