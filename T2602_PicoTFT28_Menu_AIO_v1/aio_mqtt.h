#ifndef __AIO_MQTT_H__
#define __AIO_MQTT_H__

#define   LOCATION_LABEL_LEN   20
#define   MAX_UPDATE_10_MIN     10*60*60*1000
#define   MAX_UPDATE_1_H        60*60*60*1000
#include "Adafruit_MQTT.h"

typedef enum 
{
  AIO_SUBS_TIME = 0,
  AIO_SUBS_LA_ID_TEMP,
  AIO_SUBS_VA_ID_TEMP,
  AIO_SUBS_VA_OD_TEMP,
  AIO_SUBS_VA_OD_LUX,
  AIO_SUBS_RUUVI_E6_TEMP,
  AIO_SUBS_RUUVI_EA_TEMP,
  AIO_SUBS_RUUVI_ED_TEMP,
  AIO_SUBS_DOCK_TEMP_WATER,
  AIO_SUBS_SCD30_TEMP,
  AIO_SUBS_SCD30_HUM,
  AIO_SUBS_SCD30_CO2,
  AIO_SUBS_NBR_OF,
  AIO_SUBS_FIRST = AIO_SUBS_LA_ID_TEMP
} aio_subs_et;

typedef enum
{
  SENSOR_UNDEFINED = 0,
  SENSOR_ACTIVE,
  SENSOR_UPDATED,
  SENSOR_TIMEOUT,
  SENSOR_NBR_OF
} sensor_state_et;

typedef void (*mqtt_cb)(double tmp);

typedef struct
{
  Adafruit_MQTT_Subscribe *aio_subs;
  char                    location[LOCATION_LABEL_LEN];
  unit_et                 unit_index;
  float                   value;
  float                   min_value;
  float                   max_value;
  sensor_state_et         state;
  bool                    active;
  bool                    updated;
  uint32_t                show_interval_ms;
  uint32_t                show_next_ms;
  uint32_t                max_update_interval;
  uint32_t                next_update_limit;
  mqtt_cb                 cb;
} value_st;



void aio_mqtt_initialize(void);



void aio_mqtt_stm();

bool aio_mqtt_is_updated(uint8_t sindx);


#endif