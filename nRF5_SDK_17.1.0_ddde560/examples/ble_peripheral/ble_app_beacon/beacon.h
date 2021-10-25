

#define APP_BLE_CONN_CFG_TAG            1                                  /**< A tag identifying the SoftDevice BLE configuration. */
#define DATA_MAX_LENGTH 29

typedef struct beacon_manuf_data_type
{
    uint8_t device_type;
    uint8_t data_length;
    uint8_t data[DATA_MAX_LENGTH];
} beacon_manuf_data_type;

void beacon_advertising_init(void);
void beacon_advertising_start(void);
void beacon_advertising_stop(void);
void beacon_set_data(uint8_t length, uint8_t * dataPtr);