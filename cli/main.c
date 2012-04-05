#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>

#include "modem/modem.h"
#include "modem/modem_str.h"

/*------------------------------------------------------------------------*/

/* default names */
#define MODEMD_NAME "modemd"
#define MODEM_CLI "modem_cli"

#define BOOL2STR(i) (i ? "Yes" : "No")

/*------------------------------------------------------------------------*/

const char help[] =
    "Usage:\n"
    MODEM_CLI " -h\n"
    MODEM_CLI " [-s SOCKET] [-t] -d\n"
    MODEM_CLI " [-s SOCKET] [-t] -p PORT\n"
    MODEM_CLI " [-s SOCKET] -c COMMAND -d\n"
    MODEM_CLI " [-s SOCKET] -c COMMAND -p PORT \n\n"
    "Keys:\n"
    "-h - show this help\n"
    "-s - file socket path (default: /var/run/" MODEMD_NAME ".ctl)\n"
    "-d - detect all known modems\n"
    "-p - modem port, for example 1-1\n"
    "-c - execute at command\n"
    "-t - perform a standard sequence of commands on modem\n\n"
    "Examples:\n"
    "modem_cli -d -c ATI                             - show AT information\n"
    "modem_cli -d -c 'AT+CGDCONT=1,\"IP\",\"apn.com\"'   - set apn\n"
    "modem_cli -d -c 'AT+CPIN=\"1111\"'                - set pin code";

/*------------------------------------------------------------------------*/

static char opt_sock_path[0x100];
static char opt_modem_port[0x100];
static char opt_modem_cmd[0x100];
static int opt_detect_modems;
static int opt_modems_test;

/*------------------------------------------------------------------------*/

int analyze_parameters(int argc, char** argv)
{
    int param;

    /* receiving default parameters */
    snprintf(opt_sock_path, sizeof(opt_sock_path), "/var/run/%s.ctl", MODEMD_NAME);
    *opt_modem_port = 0;
    *opt_modem_cmd = 0;
    opt_detect_modems = 0;
    opt_modems_test = 0;

    /* analyze command line */
    while((param = getopt(argc, argv, "hs:dp:c:t")) != -1)
    {
        switch(param)
        {
            case 's':
                strncpy(opt_sock_path, optarg, sizeof(opt_sock_path) - 1);
                opt_sock_path[sizeof(opt_sock_path) - 1] = 0;
                break;

            case 'd':
                opt_detect_modems = 1;
                break;

            case 'p':
                strncpy(opt_modem_port, optarg, sizeof(opt_modem_port) - 1);
                opt_modem_port[sizeof(opt_modem_port) - 1] = 0;
                break;

            case 'c':
                strncpy(opt_modem_cmd, optarg, sizeof(opt_modem_cmd) - 1);
                opt_modem_cmd[sizeof(opt_modem_cmd) - 1] = 0;
                break;

            case 't':
                opt_modems_test = 1;
                break;

            default: /* '?' */
                puts(help);
                return(1);
        }
    }

    /* check input paramets and their conficts */
    if(!*opt_modem_port && !opt_detect_modems)
    {
        puts(help);
        return(1);
    }

    /* ... and their conficts */
    if(
        (opt_detect_modems && *opt_modem_port) ||
        (opt_modems_test && *opt_modem_cmd)
    )
    {
        puts(help);
        return(1);
    }

    return(0);
}

/*------------------------------------------------------------------------*/

void modem_test(const char* port)
{
    modem_fw_version_t fw_info;
    modem_signal_quality_t sq;
    const struct tm* tm;
    modem_info_t mi;
    char msg[0x100];
    modem_t* modem;
    time_t t;
    int cell;

    /* try open modem */
    if(!(modem = modem_open_by_port(port)))
        return;

    if(!opt_detect_modems && modem_get_info(modem, &mi))
        printf("Device: [port: %s] [%04hx:%04hx] [%s %s]\n",
            mi.port, mi.id_vendor, mi.id_product, mi.manufacturer, mi.product);

    /* show modem info */

    if(modem_get_imei(modem, msg, sizeof(msg)))
        printf("IMEI: [%s]\n", msg);

    if(modem_get_imsi(modem, msg, sizeof(msg)))
        printf("IMSI: [%s]\n", msg);

    if(modem_get_operator_name(modem, msg, sizeof(msg)))
        printf("Operator: [%s]\n", msg);

    if(modem_get_network_type(modem, msg, sizeof(msg)))
        printf("Network: [%s]\n", msg);

    if(!modem_get_signal_quality(modem, &sq))
        printf("Signal: %d dBm, %d Level\n", sq.dbm, sq.level);

    if((t = modem_get_network_time(modem)))
    {
        tm = gmtime(&t);
        strftime(msg, sizeof(msg), "%Y.%m.%d %H:%M:%S", tm);
        printf("Modem time: %s", asctime(tm));
    }

    printf("Registration: %s\n", str_network_registration(modem_network_registration(modem)));

    if(modem_get_fw_version(modem, &fw_info))
    {
        tm = gmtime(&fw_info.release);
        strftime(msg, sizeof(msg), "%Y.%m.%d %H:%M:%S", tm);
        printf("Firmware: %s, Release: %s\n", fw_info.firmware, msg);
    }

    if((cell = modem_get_cell_id(modem)))
        printf("Cell ID: %d\n", cell);

#if 0
    modem_oper_t *opers = NULL;
    int i;

    puts("Performing operator scan:");

    i = modem_operator_scan(modem, &opers);

    while(i > 0)
    {
        -- i;

        printf("stat=%d\nlong=%s\nshort=%s\nnumeric=%s\nact=%d\n\n",
            opers[i].stat, opers[i].longname, opers[i].shortname, opers[i].numeric, opers[i].act);
    }

    free(opers);
#endif

    /* close modem */
    modem_close(modem);
}

/*------------------------------------------------------------------------*/

void print_modem_at_cmd(const char* port, const char* cmd)
{
    modem_t* modem;
    char *answer;

    /* try open modem */
    if(!(modem = modem_open_by_port(port)))
        return;

    answer = modem_at_command(modem, opt_modem_cmd);

    if(answer)
        printf("Answer: [\n%s\n]\n", answer);

    free(answer);

    /* close modem */
    modem_close(modem);
}

/*------------------------------------------------------------------------*/

int main(int argc, char** argv)
{
    modem_info_t *mi;
    int res = 0;

    if((res = analyze_parameters(argc, argv)))
        goto exit;

    /* show configuration */
    printf(
        "     Basename: %s\n"
        "  Socket file: %s\n"
        "         Port: %s\n"
        "      Command: %s\n"
        "Detect modems: %s\n"
        "  Test modems: %s\n\n",
        argv[0], opt_sock_path, opt_modem_port, opt_modem_cmd,
        BOOL2STR(opt_detect_modems),
        BOOL2STR(opt_modems_test)
    );

    /* initialize modem library */
    if(modem_init(opt_sock_path))
    {
        perror("modem_init()");
        res = errno;
        goto exit;
    }

    if(opt_detect_modems)
    {
        /* let's find some modem's */
        mi = modem_find_first();

        while(mi)
        {
            printf("Device: [port: %s] [%04hx:%04hx] [%s %s]\n",
                mi->port, mi->id_vendor, mi->id_product, mi->manufacturer, mi->product);

            if(opt_modems_test)
                modem_test(mi->port);
            if(*opt_modem_cmd)
                print_modem_at_cmd(mi->port, opt_modem_cmd);

            free(mi);

            mi = modem_find_next();
        }
    }
    else if(*opt_modem_port)
    {
        if(opt_modems_test)
            modem_test(opt_modem_port);
        if(*opt_modem_cmd)
            print_modem_at_cmd(opt_modem_port, opt_modem_cmd);
    }

    modem_cleanup();

exit:
    return(res);
}
