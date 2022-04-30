/*=============================================================================
 * CESE
 * RTOS_2
 * Ejercicio: A.1
 * Alumno: Ing. Hernán Gomez Molino
 * Plataforma: EDU_CIAA_NXP
 * Fecha: 2022/04/30
 * Version: v1.0
 *===========================================================================*/

/*==================[inclusiones]============================================*/
#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"
#include "FreeRTOSConfig.h"
#include "keys.h"
#include "queue.h"

/*==================[definiciones y macros]==================================*/
#define RATE 1000
#define LED_RATE pdMS_TO_TICKS(RATE)
#define LED LEDB
#define QUEUE_Q 5
#define QUEUE_T 20
#define WELCOME_MSG  "Ejercicio A_1.\r\n"
#define USED_UART UART_USB
#define UART_RATE 115200
#define MALLOC_ERROR "Malloc Failed Hook!\n"
#define MSG_ERROR_QUE "Error al crear la cola.\r\n"
#define LED_ERROR LEDR

/*==================[definiciones de datos internos]=========================*/
const gpioMap_t leds_t[] = {LEDB};
const gpioMap_t gpio_t[] = {GPIO7};
QueueHandle_t cola_1;

/*==================[definiciones de datos externos]=========================*/
DEBUG_PRINT_ENABLE
;
extern t_key_config* keys_config;
#define LED_COUNT   sizeof(keys_config)/sizeof(keys_config[0])

/*==================[declaraciones de funciones internas]====================*/
void gpio_init(void);

/*==================[declaraciones de funciones externas]====================*/
TickType_t get_diff();
void clear_diff();

// Prototipo de funcion de la tarea
void tarea_a(void* taskParmPtr);
extern void tarea_b(void* taskParmPtr);
void tarea_c(void* taskParmPtr);
void tarea_tecla(void* taskParmPtr);

/*==================[funcion principal]======================================*/
// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main(void)
{
  // ---------- CONFIGURACIONES ------------------------------
  boardConfig();			// Inicializar y configurar la plataforma

  gpio_init();

  debugPrintConfigUart( USED_UART, UART_RATE );		// UART for debug messages
  printf( WELCOME_MSG);

  BaseType_t res;
  uint32_t i;

  res = xTaskCreate(tarea_a,           // Funcion de la tarea a ejecutar
      (const char *)"tarea_led",       // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE * 2,    // Cantidad de stack de la tarea
      0,                               // Parametros de tarea
      tskIDLE_PRIORITY + 1,            // Prioridad de la tarea
      0                                // Puntero a la tarea creada en el sistema
      );

  // Gestion de errores
  configASSERT(res == pdPASS);

  res = xTaskCreate(tarea_b,           // Funcion de la tarea a ejecutar
      (const char *)"tarea_teclas",    // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE * 2,    // Cantidad de stack de la tarea
      0,                               // Parametros de tarea
      tskIDLE_PRIORITY + 1,            // Prioridad de la tarea
      0                                // Puntero a la tarea creada en el sistema
      );

  // Gestion de errores
  configASSERT(res == pdPASS);

  res = xTaskCreate(tarea_c,           // Funcion de la tarea a ejecutar
      (const char *)"tarea_cola",      // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE * 2,    // Cantidad de stack de la tarea
      0,                               // Parametros de tarea
      tskIDLE_PRIORITY + 1,            // Prioridad de la tarea
      0                                // Puntero a la tarea creada en el sistema
      );

  // Gestion de errores
  configASSERT(res == pdPASS);

  /* inicializo driver de teclas */
  keys_Init();

  // Crear cola
  cola_1 = xQueueCreate(QUEUE_Q, QUEUE_T);

  // Gestion de errores de colas
  //configASSERT(queue_tec_pulsada != NULL);

  // Iniciar scheduler
  vTaskStartScheduler(); // Enciende tick| Crea idle y pone en ready|
                         // Evalua las tareas creadas| Prioridad mas alta pasa a running

  // ---------- REPETIR POR SIEMPRE --------------------------
  configASSERT(0);

  // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
  // directamenteno sobre un microcontroladore y no es llamado por ningun
  // Sistema Operativo, como en el caso de un programa para PC.
  return TRUE;
}

/*==================[definiciones de funciones internas]=====================*/
void gpio_init(void)
{
  uint32_t i;

  for (i = 0; i < LED_COUNT; i++)
  {
    gpioInit(gpio_t[i], GPIO_OUTPUT);
  }
}
/*==================[definiciones de funciones externas]=====================*/
// Implementacion de funcion de la tarea
void tarea_a(void* taskParmPtr)
{
  // ---------- CONFIGURACIONES ------------------------------
  TickType_t xPeriodicity = LED_RATE; // Tarea periodica cada 1000 ms
  TickType_t xLastWakeTime = xTaskGetTickCount();
  char mensaje_led[7] = "LED ON";
  bool_t led_flag = 0;

  // ---------- REPETIR POR SIEMPRE --------------------------
  while ( TRUE)
  {
    if (led_flag == 0)
    {
      gpioWrite(LED, TRUE);
      led_flag = 1;
      xQueueSend(cola_1, mensaje_led, portMAX_DELAY);

    }
    else
    {
      gpioWrite(LED, FALSE);
      led_flag = 0;
    }

    vTaskDelayUntil(&xLastWakeTime, xPeriodicity);
  }
}

void tarea_c(void* taskParmPtr)
{
  // ---------- CONFIGURACIONES ------------------------------
  char mensaje[10];

  vTaskDelay(250);
  TickType_t xPeriodicity = LED_RATE; // Tarea periodica cada 1000 ms
  TickType_t xLastWakeTime = xTaskGetTickCount();

  // ---------- REPETIR POR SIEMPRE --------------------------
  while ( TRUE)
  {
    gpioToggle(LED2);
    vTaskDelayUntil(&xLastWakeTime, xPeriodicity);
    xQueueReceive(cola_1, mensaje, (portTickType)portMAX_DELAY);
    printf("Tarea_C - Recibe: %s\r\n", mensaje);
  }
}

/* hook que se ejecuta si al necesitar un objeto dinamico, no hay memoria disponible */
void vApplicationMallocFailedHook()
{
  printf("Malloc Failed Hook!\n");
  configASSERT(0);
}
/*==================[plan de trabajo]========================================*/
//
//PRIMERA PARTE
//1- 3 tareas que enciendan un led c/u---OK
//2- 2 teclas con antirrebote------------OK
//3- crear cola de comunicación----------OK
//4- que la tarea b escriba en la cola---OK
//5- que la tarea c lea la cola----------OK
//6- adecuar los datos-------------------OK
//7- cambiar a memoria dinámica
/*==================[fin del archivo]========================================*/
