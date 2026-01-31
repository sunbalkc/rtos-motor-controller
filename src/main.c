/**
 * @file main.c
 * @brief Real-Time Motor Controller with FreeRTOS
 * @author [Sunbal Cheema]
 * @date 2020-01-31
 * 
 * Main application implementing BLDC motor control with:
 * - FreeRTOS task management
 * - PID control loop at 1kHz
 * - SPI encoder interface
 * - I2C display/EEPROM
 * - UART debug interface
 */

#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include <stdio.h>
#include <math.h>

/* Hardware peripheral handles */
SPI_HandleTypeDef hspi1;   /* Encoder */
I2C_HandleTypeDef hi2c1;   /* Display & EEPROM */
UART_HandleTypeDef huart1; /* Debug console */
TIM_HandleTypeDef htim1;   /* PWM generation */
ADC_HandleTypeDef hadc1;   /* Current sensing */

/* Task handles */
TaskHandle_t xPIDTaskHandle = NULL;
TaskHandle_t xTelemetryTaskHandle = NULL;
TaskHandle_t xSafetyTaskHandle = NULL;

/* Queue handles */
QueueHandle_t xMotorCommandQueue = NULL;

/* Mutex handles */
SemaphoreHandle_t xSPIMutex = NULL;
SemaphoreHandle_t xI2CMutex = NULL;

/* Event group for system flags */
EventGroupHandle_t xSystemEvents = NULL;
#define EVENT_EMERGENCY_STOP  (1 << 0)
#define EVENT_OVERCURRENT     (1 << 1)
#define EVENT_ENCODER_ERROR   (1 << 2)

/* Configuration constants */
#define PID_TASK_FREQUENCY_HZ   1000
#define TELEMETRY_FREQUENCY_HZ  10
#define SAFETY_FREQUENCY_HZ     100

#define PWM_FREQUENCY_HZ        20000
#define PWM_DEADTIME_NS         500

#define ENCODER_SPI_TIMEOUT_MS  10
#define ENCODER_BITS            14
#define ENCODER_RESOLUTION      (1 << ENCODER_BITS)
#define DEGREES_PER_COUNT       (360.0f / ENCODER_RESOLUTION)

#define CURRENT_LIMIT_A         10.0f
#define TEMP_LIMIT_C            85.0f

/* Motor control structure */
typedef struct {
    float setpoint_rpm;
    float current_rpm;
    float current_angle_deg;
    float duty_cycle_percent;
    uint32_t encoder_raw;
    float current_a;
    float current_b;
    float current_c;
    uint8_t enabled;
} MotorState_t;

static MotorState_t g_motor_state = {0};

/* PID controller structure */
typedef struct {
    float Kp;           /* Proportional gain */
    float Ki;           /* Integral gain */
    float Kd;           /* Derivative gain */
    float integral;     /* Integral accumulator */
    float prev_error;   /* Previous error for derivative */
    float output_min;   /* Minimum output limit */
    float output_max;   /* Maximum output limit */
    float dt;           /* Sample time (seconds) */
} PID_Controller_t;

static PID_Controller_t g_pid = {
    .Kp = 0.5f,
    .Ki = 0.1f,
    .Kd = 0.05f,
    .integral = 0.0f,
    .prev_error = 0.0f,
    .output_min = -100.0f,
    .output_max = 100.0f,
    .dt = 1.0f / PID_TASK_FREQUENCY_HZ
};

/* Motor command structure for queue */
typedef struct {
    enum {
        CMD_SET_SPEED,
        CMD_ENABLE,
        CMD_DISABLE,
        CMD_SET_PID_GAINS
    } type;
    union {
        float speed_rpm;
        struct {
            float kp;
            float ki;
            float kd;
        } pid_gains;
    } data;
} MotorCommand_t;

/* Function prototypes */
static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_ADC1_Init(void);

static void vPIDTask(void *pvParameters);
static void vTelemetryTask(void *pvParameters);
static void vSafetyMonitorTask(void *pvParameters);

static uint16_t Encoder_Read_Angle(void);
static float PID_Update(PID_Controller_t *pid, float setpoint, float measured);
static void Motor_Set_Duty_Cycle(float duty_percent);
static void Motor_Emergency_Stop(void);
static void Read_Phase_Currents(float *ia, float *ib, float *ic);

/**
 * @brief Main entry point
 */
int main(void)
{
    /* Initialize HAL */
    HAL_Init();
    
    /* Configure system clock to 168MHz */
    SystemClock_Config();
    
    /* Initialize peripherals */
    MX_GPIO_Init();
    MX_SPI1_Init();
    MX_I2C1_Init();
    MX_USART1_UART_Init();
    MX_TIM1_Init();
    MX_ADC1_Init();
    
    /* Create synchronization objects */
    xMotorCommandQueue = xQueueCreate(10, sizeof(MotorCommand_t));
    xSPIMutex = xSemaphoreCreateMutex();
    xI2CMutex = xSemaphoreCreateMutex();
    xSystemEvents = xEventGroupCreate();
    
    configASSERT(xMotorCommandQueue != NULL);
    configASSERT(xSPIMutex != NULL);
    configASSERT(xI2CMutex != NULL);
    configASSERT(xSystemEvents != NULL);
    
    /* Create tasks */
    xTaskCreate(vPIDTask, 
                "PID_Control", 
                512,  /* Stack size in words */
                NULL, 
                3,    /* Priority */
                &xPIDTaskHandle);
    
    xTaskCreate(vTelemetryTask,
                "Telemetry",
                1024,
                NULL,
                1,
                &xTelemetryTaskHandle);
    
    xTaskCreate(vSafetyMonitorTask,
                "Safety",
                256,
                NULL,
                4,    /* Highest priority */
                &xSafetyTaskHandle);
    
    /* Start PWM output */
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
    
    /* Print startup message */
    char msg[] = "Motor Controller Initialized\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
    
    /* Start FreeRTOS scheduler */
    vTaskStartScheduler();
    
    /* Should never reach here */
    while(1);
}

/**
 * @brief PID control task - runs at 1kHz
 */
static void vPIDTask(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(1000 / PID_TASK_FREQUENCY_HZ);
    
    uint32_t prev_encoder_raw = 0;
    uint32_t task_start_tick = 0;
    uint32_t task_exec_time_us = 0;
    
    /* Initialize the xLastWakeTime variable with current time */
    xLastWakeTime = xTaskGetTickCount();
    
    for(;;)
    {
        /* Wait for the next cycle */
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        
        /* Mark task start time for profiling */
        task_start_tick = DWT->CYCCNT;
        
        /* Check for emergency stop */
        EventBits_t events = xEventGroupGetBits(xSystemEvents);
        if(events & EVENT_EMERGENCY_STOP) {
            Motor_Emergency_Stop();
            continue;
        }
        
        /* Process any pending commands */
        MotorCommand_t cmd;
        if(xQueueReceive(xMotorCommandQueue, &cmd, 0) == pdTRUE) {
            switch(cmd.type) {
                case CMD_SET_SPEED:
                    g_motor_state.setpoint_rpm = cmd.data.speed_rpm;
                    break;
                case CMD_ENABLE:
                    g_motor_state.enabled = 1;
                    break;
                case CMD_DISABLE:
                    g_motor_state.enabled = 0;
                    Motor_Set_Duty_Cycle(0.0f);
                    break;
                case CMD_SET_PID_GAINS:
                    g_pid.Kp = cmd.data.pid_gains.kp;
                    g_pid.Ki = cmd.data.pid_gains.ki;
                    g_pid.Kd = cmd.data.pid_gains.kd;
                    break;
            }
        }
        
        if(!g_motor_state.enabled) {
            continue;
        }
        
        /* Read encoder position */
        uint16_t encoder_angle = Encoder_Read_Angle();
        g_motor_state.encoder_raw = encoder_angle;
        g_motor_state.current_angle_deg = encoder_angle * DEGREES_PER_COUNT;
        
        /* Calculate velocity from position delta */
        int32_t delta = (int32_t)encoder_angle - (int32_t)prev_encoder_raw;
        
        /* Handle angle wrap-around */
        if(delta > ENCODER_RESOLUTION / 2) {
            delta -= ENCODER_RESOLUTION;
        } else if(delta < -ENCODER_RESOLUTION / 2) {
            delta += ENCODER_RESOLUTION;
        }
        
        /* Convert to RPM: delta_counts * (60 sec/min) / (counts/rev * dt) */
        g_motor_state.current_rpm = (delta * 60.0f) / 
                                    (ENCODER_RESOLUTION * g_pid.dt);
        
        prev_encoder_raw = encoder_angle;
        
        /* Execute PID control */
        float pid_output = PID_Update(&g_pid, 
                                      g_motor_state.setpoint_rpm,
                                      g_motor_state.current_rpm);
        
        g_motor_state.duty_cycle_percent = pid_output;
        
        /* Update PWM output */
        Motor_Set_Duty_Cycle(pid_output);
        
        /* Calculate execution time */
        task_exec_time_us = (DWT->CYCCNT - task_start_tick) / (SystemCoreClock / 1000000);
        
        /* Monitor for timing violations */
        if(task_exec_time_us > 800) {
            /* Log warning - execution time approaching deadline */
        }
    }
}

/**
 * @brief Telemetry task - updates display and sends UART data at 10Hz
 */
static void vTelemetryTask(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(1000 / TELEMETRY_FREQUENCY_HZ);
    char uart_buffer[128];
    
    xLastWakeTime = xTaskGetTickCount();
    
    for(;;)
    {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        
        /* Format telemetry message */
        int len = snprintf(uart_buffer, sizeof(uart_buffer),
                          "RPM: %.1f/%.1f | Duty: %.1f%% | Angle: %.1f° | I: %.2fA\r\n",
                          g_motor_state.current_rpm,
                          g_motor_state.setpoint_rpm,
                          g_motor_state.duty_cycle_percent,
                          g_motor_state.current_angle_deg,
                          g_motor_state.current_a);
        
        /* Send via UART */
        HAL_UART_Transmit(&huart1, (uint8_t*)uart_buffer, len, 100);
        
        /* Update OLED display */
        /* TODO: Implement OLED update using I2C */
        
        /* Get task statistics */
        UBaseType_t pid_stack_remaining = uxTaskGetStackHighWaterMark(xPIDTaskHandle);
        if(pid_stack_remaining < 64) {
            /* Warning: Stack usage high */
        }
    }
}

/**
 * @brief Safety monitor task - checks limits at 100Hz
 */
static void vSafetyMonitorTask(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(1000 / SAFETY_FREQUENCY_HZ);
    
    xLastWakeTime = xTaskGetTickCount();
    
    for(;;)
    {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        
        /* Read phase currents */
        Read_Phase_Currents(&g_motor_state.current_a,
                           &g_motor_state.current_b,
                           &g_motor_state.current_c);
        
        /* Check overcurrent */
        if(g_motor_state.current_a > CURRENT_LIMIT_A ||
           g_motor_state.current_b > CURRENT_LIMIT_A ||
           g_motor_state.current_c > CURRENT_LIMIT_A) {
            xEventGroupSetBits(xSystemEvents, EVENT_OVERCURRENT | EVENT_EMERGENCY_STOP);
            Motor_Emergency_Stop();
        }
        
        /* Check encoder communication */
        /* TODO: Verify encoder CRC/parity */
        
        /* Check temperature */
        /* TODO: Read temperature sensor */
    }
}

/**
 * @brief Read encoder angle via SPI
 * @return 14-bit angle value
 */
static uint16_t Encoder_Read_Angle(void)
{
    uint8_t tx_data[2] = {0xFF, 0xFF};  /* Read command */
    uint8_t rx_data[2] = {0};
    uint16_t angle = 0;
    
    /* Acquire SPI mutex */
    if(xSemaphoreTake(xSPIMutex, pdMS_TO_TICKS(ENCODER_SPI_TIMEOUT_MS)) == pdTRUE) {
        
        /* Pull CS low */
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
        
        /* Perform SPI transaction */
        HAL_SPI_TransmitReceive(&hspi1, tx_data, rx_data, 2, ENCODER_SPI_TIMEOUT_MS);
        
        /* Pull CS high */
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
        
        /* Extract 14-bit angle (AS5047P format) */
        angle = ((rx_data[0] & 0x3F) << 8) | rx_data[1];
        
        /* Release mutex */
        xSemaphoreGive(xSPIMutex);
    } else {
        /* Timeout - set error flag */
        xEventGroupSetBits(xSystemEvents, EVENT_ENCODER_ERROR);
    }
    
    return angle;
}

/**
 * @brief Update PID controller
 * @param pid Pointer to PID structure
 * @param setpoint Desired value
 * @param measured Current measured value
 * @return Control output
 */
static float PID_Update(PID_Controller_t *pid, float setpoint, float measured)
{
    float error = setpoint - measured;
    float output;
    
    /* Proportional term */
    float p_term = pid->Kp * error;
    
    /* Integral term with anti-windup */
    pid->integral += error * pid->dt;
    
    /* Clamp integral to prevent windup */
    float max_integral = (pid->output_max - p_term) / pid->Ki;
    float min_integral = (pid->output_min - p_term) / pid->Ki;
    if(pid->integral > max_integral) pid->integral = max_integral;
    if(pid->integral < min_integral) pid->integral = min_integral;
    
    float i_term = pid->Ki * pid->integral;
    
    /* Derivative term with filter */
    float derivative = (error - pid->prev_error) / pid->dt;
    float d_term = pid->Kd * derivative;
    
    /* Calculate output */
    output = p_term + i_term + d_term;
    
    /* Apply output limits */
    if(output > pid->output_max) output = pid->output_max;
    if(output < pid->output_min) output = pid->output_min;
    
    /* Update previous error */
    pid->prev_error = error;
    
    return output;
}

/**
 * @brief Set motor PWM duty cycle
 * @param duty_percent Duty cycle from -100 to +100
 */
static void Motor_Set_Duty_Cycle(float duty_percent)
{
    /* Clamp to valid range */
    if(duty_percent > 100.0f) duty_percent = 100.0f;
    if(duty_percent < -100.0f) duty_percent = -100.0f;
    
    /* Convert to timer compare value */
    uint16_t arr = __HAL_TIM_GET_AUTORELOAD(&htim1);
    uint16_t ccr = (uint16_t)((fabsf(duty_percent) / 100.0f) * arr);
    
    /* Update all three phases (simplified - full FOC would be different) */
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, ccr);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, ccr);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, ccr);
}

/**
 * @brief Emergency stop - disable PWM immediately
 */
static void Motor_Emergency_Stop(void)
{
    Motor_Set_Duty_Cycle(0.0f);
    g_motor_state.enabled = 0;
    g_pid.integral = 0.0f;  /* Reset integrator */
}

/**
 * @brief Read three-phase motor currents
 */
static void Read_Phase_Currents(float *ia, float *ib, float *ic)
{
    uint32_t adc_values[3];
    
    /* Start ADC conversion */
    HAL_ADC_Start(&hadc1);
    
    /* Read channel 0 (Phase A) */
    HAL_ADC_PollForConversion(&hadc1, 10);
    adc_values[0] = HAL_ADC_GetValue(&hadc1);
    
    /* Read channel 1 (Phase B) */
    HAL_ADC_PollForConversion(&hadc1, 10);
    adc_values[1] = HAL_ADC_GetValue(&hadc1);
    
    /* Read channel 2 (Phase C) */
    HAL_ADC_PollForConversion(&hadc1, 10);
    adc_values[2] = HAL_ADC_GetValue(&hadc1);
    
    HAL_ADC_Stop(&hadc1);
    
    /* Convert to amperes (assuming 3.3V ref, 12-bit ADC, 0.1 ohm shunt, 20x gain) */
    const float ADC_TO_AMPS = 3.3f / 4096.0f / 0.1f / 20.0f;
    
    *ia = adc_values[0] * ADC_TO_AMPS;
    *ib = adc_values[1] * ADC_TO_AMPS;
    *ic = adc_values[2] * ADC_TO_AMPS;
}

/**
 * @brief GPIO interrupt handler for emergency stop button
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == GPIO_PIN_13) {  /* E-stop button */
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xEventGroupSetBitsFromISR(xSystemEvents, EVENT_EMERGENCY_STOP, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/* Peripheral initialization functions */
static void SystemClock_Config(void)
{
    /* Configure system clock to 168MHz using HSE and PLL */
    /* Implementation depends on specific board configuration */
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable GPIO clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    
    /* Configure encoder CS pin (PA4) */
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    
    /* Configure emergency stop button (PC13) with interrupt */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);  /* Lower priority than FreeRTOS */
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

static void MX_SPI1_Init(void)
{
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;  /* 10.5 MHz */
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    HAL_SPI_Init(&hspi1);
}

static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 400000;  /* 400kHz fast mode */
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    HAL_I2C_Init(&hi2c1);
}

static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    HAL_UART_Init(&huart1);
}

static void MX_TIM1_Init(void)
{
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};
    
    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 0;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = (SystemCoreClock / PWM_FREQUENCY_HZ) - 1;  /* 20kHz PWM */
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    HAL_TIM_PWM_Init(&htim1);
    
    /* Configure PWM channel */
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2);
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3);
    
    /* Configure dead-time (500ns) */
    uint32_t deadtime_ticks = (SystemCoreClock / 1000000000UL) * PWM_DEADTIME_NS;
    sBreakDeadTimeConfig.DeadTime = deadtime_ticks;
    sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
    HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig);
}

static void MX_ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = ENABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 3;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    HAL_ADC_Init(&hadc1);
    
    /* Configure channels */
    sConfig.Channel = ADC_CHANNEL_0;  /* Phase A current */
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    
    sConfig.Channel = ADC_CHANNEL_1;  /* Phase B current */
    sConfig.Rank = 2;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    
    sConfig.Channel = ADC_CHANNEL_2;  /* Phase C current */
    sConfig.Rank = 3;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

/* FreeRTOS hooks */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    /* Stack overflow detected - halt and blink LED */
    while(1) {
        HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
        HAL_Delay(100);
    }
}

void vApplicationMallocFailedHook(void)
{
    /* Heap allocation failed */
    while(1);
}