#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#include "modem/modem.h"
#include "rpc.h"
#include "thread.h"
#include "modem/types.h"

/*------------------------------------------------------------------------*/

typedef rpc_packet_t* (*rpc_function_t)(modemd_client_thread_t*, rpc_packet_t*);

/*------------------------------------------------------------------------*/

typedef struct
{
	char name[0x100];

	rpc_function_t func;
} rpc_function_info_t;

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_find_first_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	modem_find_first_next_t find_res;
	rpc_packet_t *res = NULL;

	if((find_res.find = modem_find_first(&find_res.mi)))
		res = rpc_create(TYPE_RESPONSE, p->func, (uint8_t*)&find_res, sizeof(find_res));

	return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_find_next_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	modem_find_first_next_t find_res;
	rpc_packet_t *res = NULL;

	find_res.find = *((modem_find_t**)p->data);

	if((find_res.find = modem_find_next(find_res.find, &find_res.mi)))
		res = rpc_create(TYPE_RESPONSE, p->func, (uint8_t*)&find_res, sizeof(find_res));

	return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_find_close_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	modem_find_t* find;

	find = (modem_find_t*)p->data;

	modem_find_close(find);

	return(NULL);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_open_by_port_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	rpc_packet_t *res = NULL;
	char port[0x100] = {0};
	int path_len;

	path_len = p->hdr.data_len >= sizeof(port) ? sizeof(port) - 1 : p->hdr.data_len;
	memcpy(port, p->data, path_len);
	port[path_len] = 0;

	if((priv->modem = modem_open_by_port(port)))
		res = rpc_create(TYPE_RESPONSE, p->func, (uint8_t*)priv->modem, sizeof(*priv->modem));

	return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_close_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	if(!priv->modem)
		return(NULL);

	modem_close(priv->modem);

	return(rpc_create(TYPE_RESPONSE, p->func, NULL, 0));
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_conf_reload_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	if(!priv->modem)
		return(NULL);

	modem_conf_reload(priv->modem);

	return(rpc_create(TYPE_RESPONSE, p->func, NULL, 0));
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_imei_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	rpc_packet_t *res = NULL;
	char imei[0x100];

	if(!priv->modem)
		return(NULL);

	if(modem_get_imei(priv->modem, imei, sizeof(imei)))
		res = rpc_create(TYPE_RESPONSE, p->func, (uint8_t*)imei, strlen(imei));

	return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_imsi_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	rpc_packet_t *res = NULL;
	char imsi[0x100];

	if(!priv->modem)
		return(NULL);

	if(modem_get_imsi(priv->modem, imsi, sizeof(imsi)))
		res = rpc_create(TYPE_RESPONSE, p->func, (uint8_t*)imsi, strlen(imsi));

	return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_signal_quality_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	rpc_packet_t *res = NULL;
	modem_signal_quality_t sq;

	if(!priv->modem)
		return(NULL);

	/* if signal present */
	if(!modem_get_signal_quality(priv->modem, &sq))
		res = rpc_create(TYPE_RESPONSE, p->func, (uint8_t*)&sq, sizeof(sq));

	return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_network_time_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	rpc_packet_t *res = NULL;
	time_t t;

	if(!priv->modem)
		return(NULL);

	if((t = modem_get_network_time(priv->modem)))
		res = rpc_create(TYPE_RESPONSE, p->func, (uint8_t*)&t, sizeof(t));

	return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_operator_name_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	rpc_packet_t *res = NULL;
	char oper[0x100];

	if(!priv->modem)
		return(NULL);

	if(modem_get_operator_name(priv->modem, oper, sizeof(oper)))
		res = rpc_create(TYPE_RESPONSE, p->func,
			(uint8_t*)oper, strlen(oper)
		);

	return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_network_registration_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	modem_network_reg_t nr;;

	if(!priv->modem)
		return(NULL);

	nr = modem_network_registration(priv->modem);

	return(rpc_create(TYPE_RESPONSE, p->func, &nr, sizeof(nr)));
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_network_type_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	rpc_packet_t *res = NULL;
	char nt[0x100];

	if(!priv->modem)
		return(NULL);

	if(modem_get_network_type(priv->modem, nt, sizeof(nt)))
		res = rpc_create(TYPE_RESPONSE, p->func,
			(uint8_t*)nt, strlen(nt)
		);

	return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_change_pin_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	modem_change_pin_t *pc = (modem_change_pin_t*)p->data;
	rpc_packet_t *res = NULL;

	if(!priv->modem)
		return(NULL);

	if(p->hdr.data_len != sizeof(*pc))
		return(res);

	if(!modem_change_pin(priv->modem, pc->old_pin, pc->new_pin))
		res = rpc_create(TYPE_RESPONSE, p->func, (uint8_t*)pc, sizeof(*pc));

	return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_fw_version_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	rpc_packet_t *res = NULL;
	modem_fw_ver_t fw_ver;

	if(!priv->modem)
		return(NULL);

	if(modem_get_fw_version(priv->modem, &fw_ver))
		res = rpc_create(TYPE_RESPONSE, p->func,
			(uint8_t*)&fw_ver, sizeof(fw_ver));

	return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_info_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	rpc_packet_t *res = NULL;
	usb_device_info_t mi;

	if(!priv->modem)
		return(NULL);

	if(modem_get_info(priv->modem, &mi))
		res = rpc_create(TYPE_RESPONSE, p->func, (uint8_t*)&mi, sizeof(mi));

	return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_operator_scan_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	rpc_packet_t *res = NULL;
	modem_oper_t *opers;
	int nopers = 0;

	if(!priv->modem)
		return(NULL);

	if((nopers = modem_operator_scan(priv->modem, &opers)) > 0)
		res = rpc_create(TYPE_RESPONSE, p->func, (uint8_t*)opers, sizeof(modem_oper_t) * nopers);

	free(opers);

	return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_at_command_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	rpc_packet_t *res = NULL;
	char *reply;

	if(!priv->modem)
		return(NULL);

	if((reply = modem_at_command(priv->modem, (char*)p->data)))
		res = rpc_create(TYPE_RESPONSE, p->func, (uint8_t*)reply, strlen(reply));

	free(reply);

	return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_cell_id_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	rpc_packet_t *res = NULL;
	int32_t cell_id;

	if(!priv->modem)
		return(NULL);

	/* cutting Cell ID number from the reply */
	if((cell_id = modem_get_cell_id(priv->modem)))
		res = rpc_create(TYPE_RESPONSE, p->func, (uint8_t*)&cell_id, sizeof(cell_id));

	return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_operator_scan_start_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	rpc_packet_t* res = NULL;
	int scan_res;

	if(!priv->modem)
		return(NULL);

	scan_res = modem_operator_scan_start(priv->modem, (char*)p->data);
	res = rpc_create(TYPE_RESPONSE, p->func, (uint8_t*)&scan_res, sizeof(scan_res));

	return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_operator_scan_is_running_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	rpc_packet_t *res = NULL;
	int8_t scan_res;

	if(!priv->modem)
		return(NULL);

	scan_res = modem_operator_scan_is_running(priv->modem);

	res = rpc_create(TYPE_RESPONSE, p->func, (uint8_t*)&scan_res, sizeof(scan_res));

	return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_last_error_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	int32_t err;

	if(!priv->modem)
		return(NULL);

	err = modem_get_last_error(priv->modem);

	return
	(
		rpc_create(TYPE_RESPONSE, p->func, (uint8_t*)&err, sizeof(err))
	);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_set_wwan_profile_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	rpc_packet_t *res = NULL;

	if(!priv->modem)
		return(NULL);

	if(!modem_set_wwan_profile(priv->modem, (modem_data_profile_t*)p->data))
		res = rpc_create(TYPE_RESPONSE, p->func, (uint8_t*)p, sizeof(*p));

	return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_start_wwan_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	rpc_packet_t *res = NULL;
	int32_t result;

	if(!priv->modem)
		return(NULL);

	if(!(result = modem_start_wwan(priv->modem)))
		res = rpc_create(TYPE_RESPONSE, p->func, (uint8_t*)&result, sizeof(result));

	return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_stop_wwan_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	rpc_packet_t *res = NULL;
	int32_t result;

	if(!priv->modem)
		return(NULL);

	if(!(result = modem_stop_wwan(priv->modem)))
		res = rpc_create(TYPE_RESPONSE, p->func, (uint8_t*)&result, sizeof(result));

	return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_state_wwan_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	rpc_packet_t *res = NULL;
	modem_state_wwan_t state;

	if(!priv->modem)
		return(NULL);

	state = modem_state_wwan(priv->modem);
	res = rpc_create(TYPE_RESPONSE, p->func, (uint8_t*)&state, sizeof(state));

	return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_ussd_cmd_packet(modemd_client_thread_t* priv, rpc_packet_t* p)
{
	rpc_packet_t *res = NULL;
	char *reply;

	if(!priv->modem)
		return(NULL);

	if((reply = modem_ussd_cmd(priv->modem, (char*)p->data)))
		res = rpc_create(TYPE_RESPONSE, p->func, (uint8_t*)reply, strlen(reply));

	free(reply);

	return(res);
}

/*------------------------------------------------------------------------*/

const rpc_function_info_t rpc_functions[] = {
	{"modem_find_first", modem_find_first_packet},
	{"modem_find_next", modem_find_next_packet},
	{"modem_open_by_port", modem_open_by_port_packet},
	{"modem_close", modem_close_packet},
	{"modem_get_info", modem_get_info_packet},
	{"modem_get_last_error", modem_get_last_error_packet},
	{"modem_get_imei", modem_get_imei_packet},
	{"modem_change_pin", modem_change_pin_packet},
	{"modem_get_fw_version", modem_get_fw_version_packet},
	{"modem_network_registration", modem_network_registration_packet},
	{"modem_get_imsi", modem_get_imsi_packet},
	{"modem_operator_scan_start", modem_operator_scan_start_packet},
	{"modem_operator_scan_is_running", modem_operator_scan_is_running_packet},
	{"modem_get_signal_quality", modem_get_signal_quality_packet},
	{"modem_operator_scan", modem_operator_scan_packet},
	{"modem_at_command", modem_at_command_packet},
	{"modem_get_network_time", modem_get_network_time_packet},
	{"modem_get_operator_name", modem_get_operator_name_packet},
	{"modem_get_network_type", modem_get_network_type_packet},
	{"modem_get_cell_id", modem_get_cell_id_packet},
	{"modem_conf_reload", modem_conf_reload_packet},
	{"modem_set_wwan_profile", modem_set_wwan_profile_packet},
	{"modem_start_wwan", modem_start_wwan_packet},
	{"modem_stop_wwan", modem_stop_wwan_packet},
	{"modem_state_wwan", modem_state_wwan_packet},
	{"modem_ussd_cmd", modem_ussd_cmd_packet},
	{{0, 0, 0}},
};

/*------------------------------------------------------------------------*/

void* ThreadWrapper(void* prm)
{
	const rpc_function_info_t *rpc_func;
	modemd_client_thread_t* priv = prm;
	rpc_packet_t *p_in = NULL, *p_out;

	while(!priv->terminate && (p_in = rpc_recv(priv->sock)))
	{
		rpc_print(p_in);

		if(p_in->hdr.type != TYPE_QUERY)
		{
			rpc_free(p_in);
			continue;
		}

		rpc_func = rpc_functions;
		p_out = NULL;

		while(rpc_func->func)
		{
			if(strcmp(rpc_func->name, p_in->func) == 0)
			{
				/* execute function */
				p_out = rpc_func->func(priv, p_in);

				break;
			}

			++ rpc_func;
		}

		if(!p_out)
			/* function failed, create NULL result */
			p_out = rpc_create(TYPE_RESPONSE, p_in->func, NULL, 0);

		rpc_send(priv->sock, p_out);
		rpc_print(p_out);
		rpc_free(p_out);
		rpc_free(p_in);
	}

	/* cleanup resources */
	close(priv->sock);

	free(priv);

	return(NULL);
}
