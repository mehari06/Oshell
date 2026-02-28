#include <unistd.h>
#include "errors.h"

void print_error(void)
{
    write(STDERR_FILENO, "An error has occurred\n", 22);
}

