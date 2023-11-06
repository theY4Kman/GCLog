#ifndef _UPLOAD_H_
#define _UPLOAD_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include "gclog.h"
#include "logger.h"
#include "tcpcli.h"

int upload(Settings *cfg, int cpm, struct tm tm);
void upload_threaded(Settings *cfg, int cpm, struct tm tm);

#endif
