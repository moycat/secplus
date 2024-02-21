#ifndef SECPLUS_TRANSMISSION_HEADER
#define SECPLUS_TRANSMISSION_HEADER

#include <stdint.h>

void secplus_app_transmit(SecPlusApp* app, uint32_t fixed_code, uint32_t rolling_code);

#endif