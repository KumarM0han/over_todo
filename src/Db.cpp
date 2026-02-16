#include "Db.h"
#include <cstdio>

#ifndef DB_FILE_PATH
#define DB_FILE_PATH "todos.db"
#endif

bool init_db(sqlite3 **db, char **err_msg, int *rc) {
    *rc = sqlite3_open(DB_FILE_PATH, db);
    if (*rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(*db));
        sqlite3_close(*db);
        return false;
    }

    const char *sql = "CREATE TABLE IF NOT EXISTS pending (id INTEGER PRIMARY KEY, heading TEXT NOT NULL, description TEXT, created DATE NOT NULL, completed BOOLEAN NOT NULL);";
    
    *rc = sqlite3_exec(*db, sql, 0, 0, err_msg);
    if (*rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", *err_msg);
        sqlite3_free(*err_msg);
        sqlite3_close(*db);
        return false;
    }

    sql = "CREATE TABLE IF NOT EXISTS completed (id INTEGER PRIMARY KEY, heading TEXT NOT NULL, description TEXT, created DATE NOT NULL, completed BOOLEAN NOT NULL);";
    *rc = sqlite3_exec(*db, sql, 0, 0, err_msg);
    if (*rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", *err_msg);
        sqlite3_free(*err_msg);
        sqlite3_close(*db);
        return false;
    }

    sql = "CREATE TABLE IF NOT EXISTS descriptions (todo_id INTEGER NOT NULL, description TEXT NOT NULL, added DATE NOT NULL, completed BOOLEAN NOT NULL);";
    *rc = sqlite3_exec(*db, sql, 0, 0, err_msg);
    if (*rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", *err_msg);
        sqlite3_free(*err_msg);
        sqlite3_close(*db);
        return false;
    }

    return true;
}