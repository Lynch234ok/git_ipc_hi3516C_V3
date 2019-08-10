#if defined(BLUETOOTH)

typedef enum {
    emAPP_BLUETOOTH_ERR_CODE_OK = 0,
    emAPP_BLUETOOTH_ERR_CODE_ERROR = -1,
    emAPP_BLUETOOTH_ERR_CODE_NOT_SUPPORT = -2
}emAPP_BLUETOOTH_ERR_CODE;

typedef enum {
    emAPP_BLUETOOTH_STATUS_NONWORKING = 0,
    emAPP_BLUETOOTH_STATUS_WORKING,
}emAPP_BLUETOOTH_STATUS;

extern emAPP_BLUETOOTH_STATUS APP_BLUETOOTH_status(char *status);

extern int APP_BLUETOOTH_get_to_json(struct json_object *bluetoothJson, bool properties_flag);

extern int APP_BLUETOOTH_init();

extern bool APP_BLUETOOTH_is_support();

#endif //defined(BLUETOOTH)

