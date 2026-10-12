#include "blu2usb/domain/version.h"

#include <string.h>

int main(void)
{
    return strcmp(blu2usb_version(), "3.1.2") == 0 ? 0 : 1;
}
