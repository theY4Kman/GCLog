#ifndef _GCLOG_H
#define _GCLOG_H

#include <termios.h>

typedef enum { SIM, DIY, GQ } Geiger;

typedef struct {
	unsigned int interval;
	Geiger device_type;
	char *device_port;
	speed_t device_baudrate;
	unsigned int device_conn_attempts;
	int device_reconnect_after_errors;
	float latitude, longitude;
	int upload_timeout;
	char *location;
	char *netc_id;
	char *radmon_user, *radmon_pass;
	char *safecast_key;
	unsigned int safecast_device;
	char *gmcmap_user, *gmcmap_device;
} Settings;

#endif //_GCLOG_H
