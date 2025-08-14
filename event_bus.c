/// \file event_bus.c
/// \brief Шина событий. Прослойка для уменьшения связности модулей приложения.
/// \attention Работает только под FreeRTOS
/// \anchor zaikin@dsol.ru
///
/// \details
/// \code{c}
/// // Источник событий
/// static void task_1(void * arg)
/// {
///     /* INIT TASK CODE */
///     for (;;)
///     {
///         /* CODE */
///         event_bus_push(EVENT_COMPETE_1);
///     }
/// }
///
/// // Потребитель событий 1
/// static void task_2(void * arg)
/// {
///     /* INIT TASK CODE */
///     event_bus_subscribe(EVENT_COMPETE_1); // Подписать текущую задачу
///     event_bus_unsubscribe_ex(EVENT_COMPETE_1, task_3_handler); // Подписать задачу 3
///     for (;;)
///     {
///         if (event_bus_wait(EVENT_COMPETE_1, 100) == true)
///         {
///             /* CODE */
///         }
///     }
/// }
///
/// // Потребитель событий 2
/// static void task_3(void * arg)
/// {
///     for (;;)
///     {
///         if (event_bus_wait(EVENT_COMPETE_1, 100) != true)
///         {
///            continue;
///         }
///         /* CODE */
///     }
/// }
/// \endcode

//----------------------------------------------------- Includes -------------------------------------------------------

#include "event_bus.h"
#include "debug_tools/debug.h"

#include "FreeRTOS.h"
#include "task.h"

#include "global_inc_std.h"

//------------------------------------------------------ Guards --------------------------------------------------------

static_assert((configTASK_NOTIFICATION_ARRAY_ENTRIES == EVENT_BUS_QTY), //
              "configTASK_NOTIFICATION_ARRAY_ENTRIES must be equals EVENT_BUS_QTY");

//------------------------------------------------------ Macros --------------------------------------------------------

#define EVENT_LIST_DEPHT (5) ///< Глубина очереди событий. Количество задач которые могут подписаться на событие

//----------------------------------------------------- Typedefs -------------------------------------------------------
//---------------------------------------------------- Variables -------------------------------------------------------

/// \brief Массив очередей событий
/// \note events[ количество событий ][ глубина очереди ]
static event_task_handler_t events[ EVENT_BUS_QTY ][ EVENT_LIST_DEPHT ] = {0};

//------------------------------------------------- Static Functions ---------------------------------------------------

/// \brief Модифицирует ячейку списка событий
/// \param event событие, очередь которого необходимо изменить
/// \param old значение которое необходимо найти и изменить
/// \param new значение на которое нужно изменить
/// \return если успешно, то значение параметра "new", в противном случае NULL
/// \example
static event_task_handler_t event_bus_modify_value(event_bus_event_e event, //
                                                   event_task_handler_t old, event_task_handler_t new)
{
    for (size_t i = 0; i < EVENT_LIST_DEPHT; i++)
    {
        if (events[ event ][ i ] == old)
        {
            events[ event ][ i ] = new;
            return new;
        }
    }
    return NULL;
}

//---------------------------------------------------- Functions -------------------------------------------------------

/// \brief Подписаться на выбранное событие
/// \param event событие
/// \param h_task хендлер задачи, которой будет передаваться уведомление
void event_bus_subscribe_ex(event_bus_event_e event, event_task_handler_t h_task)
{
    event_task_handler_t evth = event_bus_modify_value(event, NULL, h_task);
    ASSERT_POINTER(evth);
}


/// \brief Отписаться от события
/// \param event событие
/// \param h_task хендлер задачи, котора будет исключена из списка рассылки уведомлений
void event_bus_unsubscribe_ex(event_bus_event_e event, event_task_handler_t h_task)
{
    event_bus_modify_value(event, h_task, NULL);
}


/// \brief Подписаться на выбранное событие
/// \param event событие
/// \warning Подписка должна быть выполнена непосредственно в задаче которая будет ожидать это событие,
/// в другом случае необходимо пользоваться функцией \ref event_bus_subscribe_ex(event, htask)
void event_bus_subscribe(event_bus_event_e event)
{
    event_task_handler_t h_task = xTaskGetCurrentTaskHandle();
    event_bus_subscribe_ex(event, h_task);
}


/// \brief Отписаться от события
/// \param event событие
/// \warning Должна быть выполнена непосредственно в задаче которая была подписана на это событие,
/// в другом случае необходимо пользоваться функцией \ref event_bus_unsubscribe_ex(event, htask)
void event_bus_unsubscribe(event_bus_event_e event)
{
    event_task_handler_t h_task = xTaskGetCurrentTaskHandle();
    event_bus_unsubscribe_ex(event, h_task);
}


/// \brief Ждать возникновения события
/// \param event событие
/// \param timeout время ожидания
/// \return true если событие произошло
/// \return false если прошел период ожидания, но событие так и не наступило
/// \warning Должна быть вызвана непосредственно в задаче которая была подписана на это событие.
bool event_bus_wait(event_bus_event_e event, size_t timeout)
{
    return ulTaskNotifyTakeIndexed(event, pdTRUE, timeout) != 0;
}


/// \brief Генерирует событие и рассылает уведомления подписанным задачам
/// \param event событие
void event_bus_push(event_bus_event_e event)
{
    for (size_t i = 0; i < EVENT_LIST_DEPHT; i++)
    {
        if (events[ event ][ i ] != NULL)
        {
            xTaskNotifyGiveIndexed(events[ event ][ i ], event);
        }
    }
    portYIELD();
}

//--------------------------------------------------- End Of File ------------------------------------------------------
