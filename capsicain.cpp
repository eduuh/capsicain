#include "traybar.h"

int main(int argc, char* argv[])
{
    int ret = 0;

    // Before exiting, ensure tray icon is removed
    DeleteIconFromTraybar();
    return ret;
}