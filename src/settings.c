#include "settings.h"

Settings makeSettings(int val)
{
    return (Settings){.val = val * 2};
}