#ifndef __MODEM_TYPES_H
#define __MODEM_TYPES_H

#include <time.h>
#include <stdint.h>

/*------------------------------------------------------------------------*/

typedef uint32_t* modem_t;

/*------------------------------------------------------------------------*/

typedef struct
{
    /** port number */
    char port[8];

    /** vendor name */
    uint16_t id_vendor;

    /** product name */
    uint16_t id_product;

    /** manufacturer name */
    char manufacturer[256];

    /** product name */
    char product[256];
} __attribute__((__packed__)) modem_info_t;

/*------------------------------------------------------------------------*/

typedef struct
{
    /** signal quality in dBm */
    int16_t dbm;

    /** signal level 1-5 */
    uint8_t level;
} __attribute__((__packed__)) modem_signal_quality_t;

/*------------------------------------------------------------------------*/

typedef enum
{
    PPP_NONE = 0,
    PPP_PAP,
    PPP_CHAP
} __attribute__((__packed__)) ppp_auth_protocol_t;

/*------------------------------------------------------------------------*/

typedef struct
{
    /** APN */
    char apn[101];

    /** username */
    char username[33];

    /** password */
    char password[33];

    /** type of authrization (None, PAP, CHAP) */
    ppp_auth_protocol_t auth;
} __attribute__((__packed__)) modem_data_profile_t;

/*------------------------------------------------------------------------*/

typedef enum
{
    MODEM_NETWORK_REG_FAILED    = 0,
    MODEM_NETWORK_REG_HOME,
    MODEM_NETWORK_REG_SEARCHING,
    MODEM_NETWORK_REG_DENIED,
    MODEM_NETWORK_REG_UNKNOWN,
    MODEM_NETWORK_REG_ROAMING
} __attribute__((__packed__)) modem_network_reg_t;

/*------------------------------------------------------------------------*/

typedef struct
{
    char old_pin[16];

    char new_pin[16];
} __attribute__((__packed__)) modem_change_pin_t;

/*------------------------------------------------------------------------*/

typedef struct
{
    char firmware[0x100];

    time_t release;
} __attribute__((__packed__)) modem_fw_version_t;

/*------------------------------------------------------------------------*/

typedef enum
{
    MODEM_OPER_STAT_UNKNOWN   = 0,
    MODEM_OPER_STAT_AVAILABLE = 1,
    MODEM_OPER_STAT_CURRENT   = 2,
    MODEM_OPER_STAT_FORBIDDEN = 3
} __attribute__((__packed__)) modem_oper_stat_t;

/*------------------------------------------------------------------------*/

typedef enum
{
    MODEM_OPER_ACT_GSM     = 0,
    MODEM_OPER_ACT_COMPACT = 1,
    MODEM_OPER_ACT_UTRAN   = 2
} __attribute__((__packed__)) modem_oper_act_t;

/*------------------------------------------------------------------------*/

typedef enum
{
    MODEM_CPIN_STATE_UNKNOWN = -1,
    MODEM_CPIN_STATE_READY   = 0,
    MODEM_CPIN_STATE_PIN,
    MODEM_CPIN_STATE_PUK
} __attribute__((__packed__)) modem_cpin_state_t;

/*------------------------------------------------------------------------*/

typedef enum
{
    MODEM_COPS_MODE_UNKNOWN = -1,
    MODEM_COPS_MODE_AUTO    = 0,
    MODEM_COPS_MODE_MANUAL,
    MODEM_COPS_MODE_DEREGISTER,
    MODEM_COPS_MODE_SET_FORMAT,
    MODEM_COPS_MODE_MANUAL_AUTO
} __attribute__((__packed__)) modem_cops_mode_t;

/*------------------------------------------------------------------------*/

typedef struct
{
    modem_oper_stat_t stat;

    char longname[17];

    char shortname[9];

    char numeric[13];

    modem_oper_act_t act;
} __attribute__((__packed__)) modem_oper_t;

#endif /* __MODEM_TYPES_H */
