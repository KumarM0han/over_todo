#pragma once

#include <list>
#include <cstring>

#define HEADING_MAX_SIZE 2048
#define DESCRIPTION_MAX_SIZE 8192
#define DATE_MAX_SIZE 11

template <typename T>
using list = std::list<T>;

struct FixedSizeHeapString {
    char *data;
    size_t size;
};

typedef FixedSizeHeapString Heading;
typedef FixedSizeHeapString Date;
typedef FixedSizeHeapString Description;

struct Info {
    Date added;
    Description description;

    ~Info() {
        delete[] description.data;
    }
};

struct Todo {
    Date created;
    Heading heading;
    
    list<Info*> info;

    ~Todo() {
        for (Info* i : info) {
            delete i;
        }
    }
};

struct UiData {
    list<Todo*> pending_todos;
    list<Todo*> completed_todos;

    static const char *ui_heading;
    Todo* ui_selected_todo;
    Heading ui_last_header_entry;
    Description ui_last_description_entry;

    UiData() : 
        ui_selected_todo(NULL),
        ui_last_header_entry{new char[HEADING_MAX_SIZE]{}, HEADING_MAX_SIZE},
        ui_last_description_entry{new char[DESCRIPTION_MAX_SIZE]{}, DESCRIPTION_MAX_SIZE}
    {}
    void Render();

    ~UiData() {
        delete[] ui_last_header_entry.data;
        delete[] ui_last_description_entry.data;
    }

private:
    void Ui_AddTodoPopup();
    void Ui_TodoList();
};