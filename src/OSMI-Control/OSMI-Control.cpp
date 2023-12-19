#include "OSMI-Control.h"
#include <driver/timer.h>
#include <freertos/queue.h>
#include "FluidDeliveryController.h"

#include <DRV8434S.h>

#include <hal/ledc_hal.h>
#include <driver/ledc.h>

#define STEPPER_CS 27
#define STEPPER_STEP 26

#define GROUP timer_group_t::TIMER_GROUP_0
#define TIMER timer_idx_t::TIMER_1

#define PWM_TIMER LEDC_TIMER_2
#define PWM_SPEED LEDC_SPEED_MODE_MAX
#define PWM_CHANNEL LEDC_CHANNEL_4

#define TIMER_DIVIDER 8
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER)

/// @brief  Interrupt handler for 500ms Timer
/// @param args
/// @return
static bool IRAM_ATTR control_timer_interrupt(void *args)
{
    BaseType_t high_task_awoken = pdFALSE;

    QueueHandle_t *control_queue = (QueueHandle_t *)args;

    xQueueSendFromISR(*control_queue, new CheckSystemEvent(), &high_task_awoken);

    return high_task_awoken == pdTRUE;
}

/// @brief Initialize and set GROUP : TIMER to be our periodic timer for our object to handle our dispatch.
static void tg_timer_init(QueueHandle_t *handle)
{
    timer_config_t config = {
        .alarm_en = TIMER_ALARM_EN,
        .counter_en = TIMER_PAUSE,
        .intr_type = TIMER_INTR_MAX,
        .counter_dir = TIMER_COUNT_UP,
        .auto_reload = (timer_autoreload_t)1,
        .divider = TIMER_DIVIDER, // TODO configure so that control ticks every .5s

    };

    timer_init(GROUP, TIMER, &config);
    timer_set_counter_value(GROUP, TIMER, 0);

    timer_set_alarm_value(GROUP, TIMER, 1 * TIMER_SCALE);
    timer_enable_intr(GROUP, TIMER);

    timer_isr_callback_add(GROUP, TIMER, control_timer_interrupt, handle, 0);

    timer_start(GROUP, TIMER);
}

/// @brief Setup PWM channel.
/// Set duty to 50% of 255 (duty doesn't matter, only frequency)
/// ESP_ERROR_CHECK(ledc_set_duty(PWM_SPEED, PWM_CHANNEL, 128));
/// ESP_ERROR_CHECK(ledc_update_duty(PWM_SPEED, PWM_CHANNEL));
void initPWM(void)
{
    const ledc_timer_config_t timerConfig = {
        .speed_mode = PWM_SPEED,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = PWM_TIMER,
        .freq_hz = 500,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    const ledc_channel_config_t chanConfig = {
        .gpio_num = STEPPER_STEP,
        .speed_mode = PWM_SPEED,
        .channel = PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = PWM_TIMER,
        .duty = 0,
        .hpoint = 0,
    };

    ESP_ERROR_CHECK(ledc_timer_config(&timerConfig));
    ESP_ERROR_CHECK(ledc_channel_config(&chanConfig));
}

void ControlTask(void *params)
{
    FluidDeliveryController *state = (FluidDeliveryController *)params;

    // Setup timer for sending an update fluid status
    QueueHandle_t handle = state->getQueue();

    //TODO call when ready: tg_timer_init(&handle)

    DRV8434S driver = DRV8434S();
    driver.setChipSelectPin(STEPPER_CS);
    driver.resetSettings();
    driver.clearFaults();

    driver.setCurrentMilliamps(1000);
    driver.enableSPIDirection();
    driver.setDirection(0);
    
    initPWM();
    ESP_ERROR_CHECK(ledc_set_duty(PWM_SPEED, PWM_CHANNEL, 128));
    ESP_ERROR_CHECK(ledc_update_duty(PWM_SPEED, PWM_CHANNEL));

    while (1)
    {
        // FluidControlEvent *e;

        // xQueueReceive(state->getQueue(), e, portMAX_DELAY);
        // state->handleDispatch(e);
        delay(1000);
    }
}

/*Implementation for ControlTask */

ControlState::ControlState(QueueHandle_t queue, float volumePerDistance, FluidDeliveryDriver *driver)
{
    this->queue = queue;
    this->p_Controller = FastPID(volumePerDistance, 0, 0, 2);
    this->driver = driver;
}

QueueHandle_t ControlState::getQueue()
{
    return this->queue;
}

bool ControlState::startFlow()
{
    return xQueueSend(this->queue, new StartFlowEvent(), portMAX_DELAY);
}

bool ControlState::stopFlow()
{
    return xQueueSend(this->queue, new StopFlowEvent(), portMAX_DELAY);
}

void ControlState::handleDispatch(FluidControlEvent *event)
{
    Serial.print("Dispatching Event: ");
    Serial.println(event->getID());

    switch (event->getID())
    {
    case 4:
        this->settings = ((SetDosageEvent *)event)->getSettings();
    case 0:
        Serial.println("Recalculating");
        // TODO GABE call your function here.
        break;
    default:
        Serial.println("Unknown event!");
    }

    free(event);
}

float ControlState::getVolumeDelivered()
{
    // TODO: Implement
    return 0;
}
