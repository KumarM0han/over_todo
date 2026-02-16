#pragma once

#include "sqlite3.h"

bool init_db(sqlite3 **db, char **err_msg, int *rc);