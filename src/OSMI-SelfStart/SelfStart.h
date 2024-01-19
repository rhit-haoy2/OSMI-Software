#ifndef __SELFSTART_H
#define __SELFSTART_H

extern "C"
{
#include "esp_partition.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
}

void SelfStartTestTask(void *params){}

#endif