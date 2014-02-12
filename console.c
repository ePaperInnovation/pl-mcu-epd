/*
  Plastic Logic EPD project on MSP430

  Copyright (C) 2013 Plastic Logic Limited

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*
 * console.c - serial console command line parser
 *
 * Authors: Nick Terry <nick.terry@plasticlogic.com>
 *
 */

#include "types.h"
#include <stdlib.h>
#include <string.h>
#include "msp430-uart.h"

typedef union arg
{
    s32  l;
    char*  s;
} ARG;

/*
 * Typedef for command functions - functions that implement term_command
 * See func entry in struct Command below.
 */
typedef int CommandFunction(u8, union arg *);

/*
 * Definition of a command entry.
 * command - is the command name - eg "help"
 * args    - is the argument descriptor for the command; it specifies
 *           the syntax for the command's arguments:
 *           b - binary number (not supported yet)
 *           d - decimal number
 *           x - hexadecimal number
 *           s - string (sequence of characters not including space)
 *           One character is the specifier specifies one argument to
 *           the command.
 *           If no arguments are required specify NULL as the argument string
 * func    - the function to call for the command.  Is called with the
 *           number of arguments processed and the arguments.
 *           Note: Argument conversion fails silently.
 */
typedef struct Command
{
    char *command;
    char *args;
    CommandFunction *func;
} Command;

/*
 * Macros
 */
#define TERM_MAX_INPUTLINE   40
#define TERM_MAX_ARGS		 10

/*
 * Function prototypes
 */
extern Command term_command[];
extern ARG args[];

int term_got_line(void);
int term_process_line(Command *cmd_table);


#define TERM_NO_COMMAND ((u8)0xFF)

char term_line[TERM_MAX_INPUTLINE];

ARG term_args[TERM_MAX_ARGS];
u8  term_last_cmd = TERM_NO_COMMAND;
u8  term_last_arg;

u32 term_hex(const char *);

CommandFunction pterm_command_help;

Command term_command[] = {
    { "help",       NULL,       pterm_command_help      },
};

int
pterm_command_help(u8 argcnt, union arg *args)
{
    u8 i;

    msp430_uart_puts("help: command listing (name followed by arg string)\n");

    for (i = 0; term_command[i].command; ++ i)
    {
        msp430_uart_puts(term_command[i].command);
        msp430_uart_puts("\t");
        if (term_command[i].args)
        {
            msp430_uart_puts(term_command[i].args);
        }
        msp430_uart_puts("\n");
    }
    return 0;
}

static char delim[] = {' ', '\t', '\0'};

/*
 * Identify the command and process its arguments (if any)
 */
int term_process(void)
{
	return term_process_line(term_command);
}
int
term_process_line(Command *cmd_table)
{
    u8 index, arg;
    char  *p;
    char  arg_type;
    int   r;

    /* get the command, if any */
    p = strtok(term_line, delim);
	if (p == NULL)
	{
		return 0;
	}

    /* if it is the reexecute command rerun the last command if one exists */
    if ( (p[0] == '!') && ((p[1] == '!') || (p[1] == '#')) )
    {
        if (term_last_cmd != TERM_NO_COMMAND)
        {
            do
            {
                r = cmd_table[term_last_cmd].func(term_last_arg, term_args);

                if (msp430_uart_getc() >= 0)
                {
                    break;
                }
            }
            while (r == 0 && p[1] == '#');

            return r;
        }
        return -ENOENT;
    }

    /* locate the command in the command table */
    for (index = 0; cmd_table[index].command; index++)
    {
        if (strcmp(p, cmd_table[index].command) == 0)
        {
            break;
        }
    }

    /* unknown term_command are reported */
    if (NULL == cmd_table[index].command)
    {
        return -ENOENT;
    }

    /* Now process the argumemts (if any) */
    arg = 0;
    if (cmd_table[index].args)
    {
        while (1)
        {
            arg_type = cmd_table[index].args[arg];

            if (arg_type == '\0')
            {
                break;
            }

            if (arg_type == 'l')
            {
                p = strtok(NULL, &delim[2]);
            }
            else
            {
                p = strtok(NULL, delim);
            }

            if (p == NULL)
            {
                break;
            }

            switch (arg_type)
            {
                case 'b':
                    /*
                     * :todo: implement me.
                     */
                    term_args[arg].l = 0xa5;
                    break;

                case 'i':
                case 'd':
                    term_args[arg].l = (s32)atol(p);
                    break;

                case 'x':
                    term_args[arg].l = (s32)term_hex(p);
                    break;

                case 's':
                case 'l':
                    term_args[arg].s = p;
                    break;

                default:
                    break;
            }
            ++arg;
        }
    }
    /* remember the last thing we executed */
    term_last_cmd = index;
    term_last_arg = arg;

    /* invoke the command handler with the arguments supplied */
    return cmd_table[index].func(arg, term_args);
}

/*
 * Assemble an input line.  Handles backspace and delete.
 */
int
term_got_line(void)
{
    static int outputPrompt = true;
    static int nextChar = 0;
    s16 c;

    if (outputPrompt)
    {
        msp430_uart_puts("> ");
        outputPrompt = false;
        nextChar = 0;
    }

    if ( (c = msp430_uart_getc()) >= 0)
    {
        switch (c)
        {
            case 0x7f:
            case '\b':
                if (nextChar > 0)
                {
                    msp430_uart_puts("\b \b");
                    nextChar--;
                }
                break;
            case '\r':
            case '\n':
                msp430_uart_putc('\n');
                term_line[nextChar] = '\0';
                outputPrompt = true;
                return true;

            default:
                if (nextChar < (sizeof(term_line) - 2))
                {
                    term_line[nextChar++] = (char)c;
                    msp430_uart_putc((char)c);
                }
                break;
        }
    }
    return false;
}

/*
 * Convert a single character from hexadecimal to a number.
 * This is a helper function used by term_hex.
 */
u8
term_1_hex(char c)
{
    static char xdigit[] = "0123456789abcdefABCDEF";
    u8 i;

    for (i = 0; i < ARRAY_SIZE(xdigit); ++i)
    {
        if (xdigit[i] == c)
        {
            if (i > 15)
            {
                i -= 6;
            }
            return i;
        }
    }
    return 0;
}

/*
 * Converts string from hexadecimal number to UInt32.
 */
u32
term_hex(const char *p)
{
    u32 x;

    x = 0;
    while (*p)
    {
        x <<= 4;
        x += term_1_hex(*p);
        ++p;
    }
    return x;
}


