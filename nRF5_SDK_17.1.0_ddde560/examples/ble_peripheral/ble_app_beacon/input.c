#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "app_uart.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf.h"
#include "bsp.h"
#include "nrf_uart.h"
#include "input.h"


#define CHARACTER_CARRIAGE_RETURN     '\r'
#define CHARACTER_BACKSPACE           0x7F
#define INPUTBUFFER_LENGTH            42

#define COORDINATE_MULTIPLIER  1000000000000000
#define LATITUDE_MAX         ( 90 * COORDINATE_MULTIPLIER )
#define LATITUDE_MIN         (-90 * COORDINATE_MULTIPLIER )
#define LONGITUDE_MAX        ( 180 * COORDINATE_MULTIPLIER)
#define LONGITUDE_MIN        (-180 * COORDINATE_MULTIPLIER)

static coordinates inputCoordinates = {
    .latitude =     0,
    .longitude =    0,
};

static uint8_t bufferPtr = 0;
static uint8_t inputBuffer[INPUTBUFFER_LENGTH] = { 0 };

static input_result parseCoordinates();
static input_result checkLimits(coordinates c);
static input_result handleUARTInput();
static input_result parseCoordinate(char * ptr, int64_t * resultPtr);

input_result input_entry()
{
    input_result result = input_OK;

    result = handleUARTInput();
    if (result == input_OK)
    {
        result = parseCoordinates();
    }

    switch (result)
    {
        case input_OK:
            printf("\r\nInput OK...\r\n");
            printf("Latitude: %lld\r\n", inputCoordinates.latitude);
            printf("Longitude: %lld\r\n", inputCoordinates.longitude);
            break;

        case input_NOK:
            printf("\r\nInvalid format. Try again.\r\n");
            break;

        case input_OUTSIDE_RANGE:
            printf("\r\nEntered coordinates are outside the valid range. Latitude %d...%d Longitude %d...%d. Try again.\r\n", 
                (int32_t)(LATITUDE_MIN / COORDINATE_MULTIPLIER), 
                (int32_t)(LATITUDE_MAX / COORDINATE_MULTIPLIER), 
                (int32_t)(LONGITUDE_MIN / COORDINATE_MULTIPLIER), 
                (int32_t)(LONGITUDE_MAX / COORDINATE_MULTIPLIER)
                );
            break;

        case input_IN_PROGRESS:
            // Input in progress
            break;

        case input_TOO_LONG:
            printf("\r\nMaximum input length exceeded: %d. Try again.\r\n", INPUTBUFFER_LENGTH);
            break;

        default:
            break;
    }

    return result;
}

coordinates input_getCoordinates(void)
{
    return inputCoordinates;
}

static input_result handleUARTInput()
{
    input_result result = input_IN_PROGRESS;
    uint8_t input;

    if (app_uart_get(&input) == NRF_SUCCESS)
    {
        app_uart_put(input);
        if (input == CHARACTER_CARRIAGE_RETURN)
        {
            inputBuffer[bufferPtr] = input;
            input = 0;
            bufferPtr = 0;
            result = input_OK;
        }
        else if ((input == CHARACTER_BACKSPACE) && (bufferPtr > 0))
        {
            bufferPtr--;
            result = input_IN_PROGRESS;
        }
        else
        {
            inputBuffer[bufferPtr] = input;
            bufferPtr++;
            result = input_IN_PROGRESS;

            if (bufferPtr >= INPUTBUFFER_LENGTH)
            {
                bufferPtr = 0;
                result = input_TOO_LONG;
            }
        }
    }

    return result;
}

static input_result parseCoordinates()
{
    input_result result = input_NOK;
    uint8_t latitudeBuffer[INPUTBUFFER_LENGTH];
    uint8_t longitudeBuffer[INPUTBUFFER_LENGTH];
    coordinates c = {
        .latitude = 0, 
        .longitude = 0
    };

    result = (sscanf(inputBuffer,"%[^';'];%[^';''\r']\r", &latitudeBuffer, &longitudeBuffer) == 2) ? input_OK : input_NOK;
    
    if (result == input_OK)
    {
        result |= parseCoordinate(&latitudeBuffer[0], &c.latitude);
        result |= parseCoordinate(&longitudeBuffer[0], &c.longitude);
    }

    if (result == input_OK)
    {
        result |= checkLimits(c);
    }

    if (result == input_OK)
    {
        inputCoordinates.latitude = c.latitude;
        inputCoordinates.longitude = c.longitude;
    }

    return result;
}

static input_result parseCoordinate(char * ptr, int64_t * resultPtr)
{
    input_result result = input_OK;
    uint8_t * cPtr = ptr;
    uint64_t multiplier = COORDINATE_MULTIPLIER;
    uint8_t isMinus = 0;
    uint8_t integerCount = 0;
    uint8_t isDecimalPointDetected = 0;

    while (*cPtr != 0 && result == input_OK)
    {
        if ( isdigit(*cPtr) )
        {
            *resultPtr += ((uint8_t)(*cPtr - '0')) * multiplier;
            multiplier /= 10;
            integerCount++;
        }
        else if (*cPtr == '-')
        {
            isMinus = 1;
        }
        else if ( (*cPtr == '.') && !isDecimalPointDetected && (integerCount > 0) )
        {
            *resultPtr *= pow(10, integerCount - 1);
            multiplier *= pow(10, integerCount - 1);

            // Ensure only one decimal point can be detected
            isDecimalPointDetected = 1;
        }
        else
        {
            result = input_NOK;
        }

        cPtr++;
    }
    
    if (isMinus)
    {
        *resultPtr *= -1;
    }

    return result;
}

static input_result checkLimits(coordinates c)
{
    input_result result = input_OK;

    if (c.latitude  > LATITUDE_MAX       || 
        c.latitude  < LATITUDE_MIN       ||
        c.longitude > LONGITUDE_MAX      ||
        c.longitude < LONGITUDE_MIN       )
    {
        result = input_OUTSIDE_RANGE;
    }

    return result;
}

