#include <stdint.h>

typedef enum input_result {
    input_OK = 0,
    input_NOK = 1,
    input_OUTSIDE_RANGE = 2,
    input_IN_PROGRESS = 4,
    input_TOO_LONG = 8
} input_result;

typedef struct coordinates
{
    int64_t latitude;
    int64_t longitude;
} coordinates;

input_result input_entry(void);
coordinates input_getCoordinates(void);