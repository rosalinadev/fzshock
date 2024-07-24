#include <furi.h>

typedef enum {
    CH01,
    CH02,
    CH03,
} ShockerChannel;

typedef enum {
    Shock = 1,
    Vibrate,
    Beep,
} ShockerMode;

void send_payload(uint8_t strength);
