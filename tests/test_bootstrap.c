#include "blu2usb/domain/version.h"

#include <string.h>

int main(void)
{
    return strcmp(blu2usb_version(), "1.0.0") == 0 ? 0 : 1;
}
