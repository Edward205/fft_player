#include "common.h"
#include "ui/components.h"

settingText amp_set;
settingText play_speed_set;
settingText chunk_play_set;
settingText separation_thresh_set;
bool pause = false;;
int modul(int a)
{
    if(a < 0)
        return -a;
    return a;
}
