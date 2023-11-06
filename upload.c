#include "upload.h"

#define BUF_SIZE	1000

bool string_isset(const char *str) {
	return str != NULL && str[0] != '\0';
}

bool send_gmcmap(int timeout, const char *user, const char *device, int cpm) {
	char *req;
	int sock;
	char buf[BUF_SIZE] = { 0 };

	if (asprintf(&req, "GET /log2.asp?AID=%s&GID=%s&CPM=%d HTTP/1.1\r\nHost: www.gmcmap.com\r\n\r\n", user, device, cpm) == -1)
		return false;

	if ((sock = tcp_connect("www.gmcmap.com", "80", timeout)) != -1) {
		tcp_send(sock, req);
		tcp_receive(sock, buf, BUF_SIZE);
		tcp_close(sock);
	}

	free(req);

	return sock != -1 && strstr(buf, "OK.") != NULL;
}

bool send_netc(int timeout, const char *id, int cpm) {
	char *req;
	int sock;
	char buf[BUF_SIZE] = { 0 };

	if (asprintf(&req, "GET /push.php?id=%s&v=w32_1.1.3.1085&c=%d HTTP/1.1\r\nHost: radiation.netc.com\r\n\r\n", id, cpm) == -1)
		return false;

	if ((sock = tcp_connect("radiation.netc.com", "80", timeout)) != -1) {
		tcp_send(sock, req);
		tcp_receive(sock, buf, BUF_SIZE);
		tcp_close(sock);
	}

	free(req);

	return sock != -1 && strstr(buf, "Ok.") != NULL;
}

bool send_radmon(int timeout, const char *user, const char *pass, int cpm, const struct tm *tm) {
	char ch[22], *req;
	int sock;
	char buf[BUF_SIZE] = { 0 };

	strftime(ch, 22, "%Y-%m-%d%%20%H:%M:%S", tm);
	if (asprintf(&req, "GET /radmon.php?user=%s&password=%s&function=submit&datetime=%s&value=%d&unit=CPM HTTP/1.1\r\nHost: www.radmon.org\r\n\r\n", user, pass, ch, cpm) == -1)
		return false;

	if ((sock = tcp_connect("www.radmon.org", "80", timeout)) != -1) {
		tcp_send(sock, req);
		tcp_receive(sock, buf, BUF_SIZE);
		tcp_close(sock);
	}

	free(req);

	return sock != -1 && strstr(buf, "Incorrect login.") == NULL;
}

bool send_safecast(int timeout, const char *key, unsigned int dev, int cpm, const struct tm *tm, float lat, float lng, const char *loc) {
	char ch[21], *pld, *req;
	int sock;
	char buf[BUF_SIZE] = { 0 };

	strftime(ch, 21, "%Y-%m-%dT%H:%M:%SZ", tm);
	asprintf(&pld, "{\"latitude\":\"%.4f\",\"longitude\":\"%.4f\",\"location_name\":\"%s\",\"device_id\":\"%u\",\"captured_at\":\"%s\",\"value\":\"%d\",\"unit\":\"cpm\"}", lat, lng, loc, dev, ch, cpm);
	asprintf(&req, "POST /measurements.json?api_key=%s HTTP/1.1\r\nHost: api.safecast.org\r\nContent-Type: application/json\r\nContent-Length: %zu\r\n\r\n%s", key, strlen(pld), pld);

	if ((sock = tcp_connect("api.safecast.org", "80", timeout)) != -1) {
		tcp_send(sock, req);
		tcp_receive(sock, buf, BUF_SIZE);
		tcp_close(sock);
	}

	free(pld);
	free(req);

	return sock != -1 && strstr(buf, "201 Created") != NULL;
}

int upload(Settings *cfg, int cpm, struct tm tm) {
	int num_errors = 0;

	if (string_isset(cfg->netc_id)) {
		if (!send_netc(cfg->upload_timeout, cfg->netc_id, cpm)) {
			log_warn("Upload to netc.com failed.");
			num_errors += 1;
		}
	}

	if (string_isset(cfg->radmon_user) && string_isset(cfg->radmon_pass)) {
		if (!send_radmon(cfg->upload_timeout, cfg->radmon_user, cfg->radmon_pass, cpm, &tm)) {
			log_warn("Upload to radmon.org failed.");
			num_errors += 1;
		}
	}

	if (string_isset(cfg->safecast_key) && string_isset(cfg->location)) {
		if (!send_safecast(cfg->upload_timeout, cfg->safecast_key, cfg->safecast_device, cpm, &tm, cfg->latitude,
		                   cfg->longitude, cfg->location)) {
			log_warn("Upload to safecast.org failed.");
			num_errors += 1;
		}
	}

	if (string_isset(cfg->gmcmap_user) && string_isset(cfg->gmcmap_device)) {
		if (!send_gmcmap(cfg->upload_timeout, cfg->gmcmap_user, cfg->gmcmap_device, cpm)) {
			log_warn("Upload to gmcmap.com failed.");
			num_errors += 1;
		}
	}

	return num_errors;
}

typedef struct {
	Settings *cfg;
	int cpm;
	struct tm tm;
} upload_threaded_args;

void _upload_threaded_wrapper(upload_threaded_args *args) {
	upload(args->cfg, args->cpm, args->tm);
	free(args);
}

void upload_threaded(Settings *cfg, int cpm, struct tm tm) {
	upload_threaded_args *args = malloc(sizeof(upload_threaded_args));
	args->cfg = cfg;
	args->cpm = cpm;
	args->tm = tm;

	thrd_t thread;
	thrd_create(&thread, (thrd_start_t) _upload_threaded_wrapper, args);
	thrd_detach(thread);
}
