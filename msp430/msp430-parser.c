// parser and file/image builder
#include "msp430-parser.h"

void parser(unsigned char inc_byte)
{
    unsigned int c_count;
    static unsigned int idle_1;
    static unsigned int CMD_SIG;
    c_count = 0;

    if(idle_1 == 1)
    {
        if(inc_byte == 0xAA)
        {
            c_count++;
            CMD_SIG++;
            if(CMD_SIG == 3)
            {
            idle_1 = 0;
            }
        }
    }



}
