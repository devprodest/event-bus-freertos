/// \file event_bus.h
/// \brief Шина событий. Прослойка для уменьшения связности модулей приложения.
/// \attention Работает только под FreeRTOS
/// \anchor zaikin@dsol.ru
///
/// \details
///
//--------------------------------------------------- Include guards ---------------------------------------------------
#pragma once
//----------------------------------------------------- Includes -------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

//------------------------------------------------------ Macros --------------------------------------------------------
//----------------------------------------------------- Typedefs -------------------------------------------------------

/// \brief События
///
typedef enum event_bus_event_e
{
    EVENT_BUS_RESERVED = 0,
    EVENT_BUS_FRAME_BEGIN_SYNC,       ///< Начало получения кадра из камеры
    EVENT_BUS_FRAME_END_SYNC,         ///< Кадр получен
    EVENT_BUS_H264_ENCODE_COMPLETE,   ///< Модуль h264 сжал кадр
    EVENT_BUS_H264_ENCODE_ALL_FRAMES, ///< Полный прогон I+P кадров

    // ---
    EVENT_BUS_QTY,
} event_bus_event_e;


typedef void * event_task_handler_t;

//------------------------------------------------- Inline Functions ---------------------------------------------------
//------------------------------------------------ Function prototypes -------------------------------------------------

void event_bus_subscribe(event_bus_event_e event);
void event_bus_unsubscribe(event_bus_event_e event);
bool event_bus_wait(event_bus_event_e event, size_t timeout);

void event_bus_subscribe_ex(event_bus_event_e event, event_task_handler_t htask);
void event_bus_unsubscribe_ex(event_bus_event_e event, event_task_handler_t htask);

void event_bus_push(event_bus_event_e event);

//--------------------------------------------------- End Of File ------------------------------------------------------
