#include "Todo.h"

#include <cstdio>
#include <iterator>
#include <unordered_map>
#include "imgui.h"
#include "Utils.h"
#include "sqlite3.h"
#include "Audio.h"

const char *UiData::ui_heading = "Todos";

void UiData::Render() {
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;
    
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(ImVec2(viewport->Size[0]/3, viewport->Size[1]));
    ImGui::Begin("Todo List", nullptr, window_flags);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

    if (ImGui::Button("Add Todo")) {
        snd_general->Play();
        ImGui::OpenPopup("Add Todo Popup");
    }

    ImGui::Checkbox("Show Completed Todos", &show_completed_todos);

    Ui_AddTodoPopup();
    
    ImGui::NewLine();
    ImGui::SeparatorText("Todo List");

    Ui_TodoList();

    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + viewport->Size[0]/3, viewport->Pos.y));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size[0]*2/3, viewport->Size[1]));
    ImGui::Begin("Todo Editor", nullptr, window_flags);
    
    Ui_EditorWindow();

    ImGui::End();
    ImGui::PopStyleColor(1);
    ImGui::End();
}

void UiData::Ui_EditorWindow() {
    ImGui::SeparatorText("Todo Editor");
    if (ui_selected_todo != nullptr) {
        ImGui::TextDisabled("%s", ui_selected_todo->heading.data);
        ImGui::NewLine();

        if (ImGui::Button("Edit Heading")) {
            snd_general->Play();
            ImGui::OpenPopup("Edit Heading Popup");
        }

        Ui_EditHeadingPopup();

        ImGui::NewLine();

        ImGui::TextDisabled("Created on: %s (%ld days ago)", ui_selected_todo->created.data, days_since(ui_selected_todo->created.data));
        
        ImGui::TextDisabled("Status: ");
        ImGui::SameLine();
        if (ui_selected_todo->completed) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Completed");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Pending");
        }
        ImGui::NewLine();

        ImGui::BeginDisabled(ui_selected_todo->completed);
        if (ImGui::Button("Add Description")) {
            snd_general->Play();
            ImGui::OpenPopup("Add Description Popup");
        }
        ImGui::EndDisabled();

        Ui_AppendDescriptionPopup();

        ImGui::SameLine();

        if (ImGui::Button("Toggle Completion")) {
            ui_selected_todo->completed = !ui_selected_todo->completed;
            if (ui_selected_todo->completed) {
                snd_add->Play();
                pending_todos.remove(ui_selected_todo);
                completed_todos.push_front(ui_selected_todo);
            } else {
                snd_cancel->Play();
                completed_todos.remove(ui_selected_todo);
                pending_todos.push_front(ui_selected_todo);
            }
        }

        ImGui::NewLine();
        Ui_ListDescription();
    } else {
        ImGui::Text("No todo selected");
    }
}

void UiData::Ui_EditHeadingPopup() {
    if (ImGui::BeginPopupModal("Edit Heading Popup")) {
        ImGui::PushItemWidth(-FLT_MIN);
        ImGui::Text("%s", "Edit Heading");
        if (ImGui::IsWindowAppearing()) {
            ImGui::SetKeyboardFocusHere(0);
            memset(ui_last_header_entry.data, 0, ui_last_header_entry.size);
            strncpy(ui_last_header_entry.data, ui_selected_todo->heading.data, ui_last_header_entry.size);
        }
        ImGui::InputText(
                "##edit_todo_header",
                ui_last_header_entry.data,
                ui_last_header_entry.size);
        ImGui::PopItemWidth();

        if (ImGui::Button("Save") && strlen(ui_last_header_entry.data) > 0) {
            snd_add->Play();
            strncpy(ui_selected_todo->heading.data, ui_last_header_entry.data, ui_selected_todo->heading.size);
            memset(ui_last_header_entry.data, 0, ui_last_header_entry.size);
            
            ImGui::CloseCurrentPopup();
        }

        if (ImGui::Button("Cancel")) {
            snd_cancel->Play();
            memset(ui_last_header_entry.data, 0, ui_last_header_entry.size);
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void UiData::Ui_AppendDescriptionPopup() {
    if (ImGui::BeginPopupModal("Add Description Popup")) {
        ImGui::PushItemWidth(-FLT_MIN);
        ImGui::Text("%s", "Add Description");
        if (ImGui::IsWindowAppearing()) {
            ImGui::SetKeyboardFocusHere(0);
        }
        ImGui::InputTextMultiline(
            "##new_description",
            ui_last_description_entry.data,
            ui_last_description_entry.size,
            ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16),
            ImGuiInputTextFlags_AllowTabInput);
        ImGui::PopItemWidth();

        if (ImGui::Button("Append Description") && strlen(ui_last_description_entry.data) > 0) {
            snd_add->Play();
            Info *new_info = new Info();
            new_info->added.data = (char *)calloc(DATE_MAX_SIZE, sizeof(char));
            new_info->added.size = DATE_MAX_SIZE;
            get_current_date(new_info->added.data, new_info->added.size);
            new_info->description.data = (char *) calloc(DESCRIPTION_MAX_SIZE, sizeof(char));
            new_info->description.size = DESCRIPTION_MAX_SIZE;
            strncpy(new_info->description.data, ui_last_description_entry.data, DESCRIPTION_MAX_SIZE);
            ui_selected_todo->info.push_front(new_info);
            memset(ui_last_description_entry.data, 0, DESCRIPTION_MAX_SIZE);
            
            ImGui::CloseCurrentPopup();
        }

        if (ImGui::Button("Cancel")) {
            snd_cancel->Play();
            memset(ui_last_description_entry.data, 0, DESCRIPTION_MAX_SIZE);
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void UiData::Ui_ListDescription() {
    ImGui::SeparatorText("Descriptions");
    for (auto itr = ui_selected_todo->info.begin();
            itr != ui_selected_todo->info.end();
            itr++)
    {
        ImGui::TextDisabled("Added on: %s (%ld days ago)", (*itr)->added.data, days_since((*itr)->added.data));
        ImGui::NewLine();
        ImGui::PushID((*itr)->description.data);

        size_t lines = 0;
        for (size_t i = 0; i < (*itr)->description.size; i++) {
            if ((*itr)->description.data[i] == '\n') {
                lines++;
            }
        }
        lines += 3;
        assert((*itr)->description.data != nullptr);
        assert(strlen((*itr)->description.data) > 0);
        ImGui::InputTextMultiline(
            "##read_only_description",
            (*itr)->description.data,
            (*itr)->description.size,
            ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * lines),
            ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_WordWrap
        );
        
        ImGui::PopID();

        ImGui::Separator();
    }
}

void UiData::Ui_TodoList() {
    if (show_completed_todos) {
        for (Todo* t : completed_todos) {
            assert(t != nullptr);
            assert(t->heading.data != nullptr);
            assert(strlen(t->heading.data) > 0);
            if (ImGui::Selectable(t->heading.data, ui_selected_todo == t, ImGuiSelectableFlags_None, ImVec2(0, ImGui::GetTextLineHeight() * 1.5))) {
                ui_selected_todo = t;
            }
            ImGui::NewLine();
        }
    } else {
        for (auto i = pending_todos.begin(); i != pending_todos.end(); i++) {
            assert(*i != nullptr);
            assert((*i)->heading.data != nullptr);
            assert(strlen((*i)->heading.data) > 0);
            if (ImGui::Selectable((*i)->heading.data, ui_selected_todo == *i, ImGuiSelectableFlags_None, ImVec2(0, ImGui::GetTextLineHeight() * 1.5))) {
                ui_selected_todo = *i;
            }
            ImGui::NewLine();
        }
    }
}

void UiData::Ui_AddTodoPopup() {
    if (ImGui::BeginPopupModal("Add Todo Popup")) {
        ImGui::PushItemWidth(-FLT_MIN);
        ImGui::Text("%s", "Add Todo");
        if (ImGui::IsWindowAppearing()) {
            ImGui::SetKeyboardFocusHere(0);
        }
        ImGui::InputText(
                "##new_todo_header",
                ui_last_header_entry.data,
                ui_last_header_entry.size);
        ImGui::InputTextMultiline(
            "##new_todo_description",
            ui_last_description_entry.data,
            ui_last_description_entry.size,
            ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16),
            ImGuiInputTextFlags_AllowTabInput);
        ImGui::PopItemWidth();

        if (ImGui::Button("Add") && strlen(ui_last_header_entry.data) > 0) {
            snd_add->Play();
            Todo *new_todo = new Todo();
            new_todo->completed = false;
            new_todo->created.data = (char *)calloc(DATE_MAX_SIZE, sizeof(char));
            new_todo->created.size = DATE_MAX_SIZE;
            get_current_date(new_todo->created.data, new_todo->created.size);

            if (strlen(ui_last_description_entry.data) > 0) {
                Info *new_info = new Info();
                new_info->added.data = (char *)calloc(DATE_MAX_SIZE, sizeof(char));
                new_info->added.size = DATE_MAX_SIZE;
                get_current_date(new_info->added.data, new_info->added.size);
                new_info->description.data = (char *) calloc(DESCRIPTION_MAX_SIZE, sizeof(char));
                new_info->description.size = DESCRIPTION_MAX_SIZE;
                strncpy(new_info->description.data, ui_last_description_entry.data, DESCRIPTION_MAX_SIZE);
                new_todo->info.push_front(new_info);
                memset(ui_last_description_entry.data, 0, DESCRIPTION_MAX_SIZE);
            }

            new_todo->heading.data = (char *)calloc(HEADING_MAX_SIZE, sizeof(char));
            new_todo->heading.size = HEADING_MAX_SIZE;
            strncpy(new_todo->heading.data, ui_last_header_entry.data, HEADING_MAX_SIZE);
            
            ui_selected_todo = new_todo;
            
            pending_todos.push_back(new_todo);
            memset(ui_last_header_entry.data, 0, HEADING_MAX_SIZE);
            
            ImGui::CloseCurrentPopup();
        }

        if (ImGui::Button("Cancel")) {
            snd_cancel->Play();
            memset(ui_last_header_entry.data, 0, HEADING_MAX_SIZE);
            memset(ui_last_description_entry.data, 0, DESCRIPTION_MAX_SIZE);
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void UiData::LoadTodos(sqlite3 *db) {
    sqlite3_stmt *stmt;
    std::unordered_map<int, Todo*> pending_map;
    std::unordered_map<int, Todo*> completed_map;

    const char *sql = "SELECT id, heading, description, created, completed FROM pending;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Todo *new_todo = new Todo();
        new_todo->completed = false;

        int todo_id = sqlite3_column_int(stmt, 0);

        const unsigned char *heading_text = sqlite3_column_text(stmt, 1);
        new_todo->heading.data = (char *)calloc(HEADING_MAX_SIZE, sizeof(char));
        new_todo->heading.size = HEADING_MAX_SIZE;
        if (heading_text) {
            strncpy(new_todo->heading.data, (const char *)heading_text, HEADING_MAX_SIZE - 1);
        }

        const unsigned char *created_text = sqlite3_column_text(stmt, 3);
        new_todo->created.data = (char *)calloc(DATE_MAX_SIZE, sizeof(char));
        new_todo->created.size = DATE_MAX_SIZE;
        if (created_text) {
            strncpy(new_todo->created.data, (const char *)created_text, DATE_MAX_SIZE - 1);
        }

        pending_todos.push_back(new_todo);
        pending_map[todo_id] = new_todo;
    }

    sqlite3_finalize(stmt);

    sql = "SELECT id, heading, description, created FROM completed;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Todo *new_todo = new Todo();
        new_todo->completed = true;

        int todo_id = sqlite3_column_int(stmt, 0);

        const unsigned char *heading_text = sqlite3_column_text(stmt, 1);
        new_todo->heading.data = (char *)calloc(HEADING_MAX_SIZE, sizeof(char));
        new_todo->heading.size = HEADING_MAX_SIZE;
        if (heading_text) {
            strncpy(new_todo->heading.data, (const char *)heading_text, HEADING_MAX_SIZE - 1);
        }

        const unsigned char *created_text = sqlite3_column_text(stmt, 3);
        new_todo->created.data = (char *)calloc(DATE_MAX_SIZE, sizeof(char));
        new_todo->created.size = DATE_MAX_SIZE;
        if (created_text) {
            strncpy(new_todo->created.data, (const char *)created_text, DATE_MAX_SIZE - 1);
        }

        completed_todos.push_back(new_todo);
        completed_map[todo_id] = new_todo;
    }

    sqlite3_finalize(stmt);

    // Load descriptions for both pending and completed todos
    sql = "SELECT todo_id, description, added, completed FROM descriptions ORDER BY rowid;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int todo_id = sqlite3_column_int(stmt, 0);
        const unsigned char *description_text = sqlite3_column_text(stmt, 1);
        const unsigned char *added_text = sqlite3_column_text(stmt, 2);
        bool is_completed = sqlite3_column_int(stmt, 3) != 0;

        Todo* target = nullptr;
        if (is_completed) {
            auto it = completed_map.find(todo_id);
            if (it != completed_map.end()) target = it->second;
        } else {
            auto it = pending_map.find(todo_id);
            if (it != pending_map.end()) target = it->second;
        }
        if (!target) continue;

        Info *new_info = new Info();
        new_info->added.data = (char *)calloc(DATE_MAX_SIZE, sizeof(char));
        new_info->added.size = DATE_MAX_SIZE;
        if (added_text) {
            strncpy(new_info->added.data, (const char *)added_text, DATE_MAX_SIZE - 1);
        }
        new_info->description.data = (char *) calloc(DESCRIPTION_MAX_SIZE, sizeof(char));
        new_info->description.size = DESCRIPTION_MAX_SIZE;
        if (description_text) {
            strncpy(new_info->description.data, (const char *)description_text, DESCRIPTION_MAX_SIZE - 1);
        }
        target->info.push_back(new_info);
    }

    sqlite3_finalize(stmt);
}

void UiData::SaveTodos(sqlite3 *db) {
    char *err_msg = nullptr;
    int rc;
    int pending_index = 0;
    int completed_index = 0;

    const char *delete_pending_sql = "DELETE FROM pending;";
    const char *delete_completed_sql = "DELETE FROM completed;";
    const char *delete_descriptions_sql = "DELETE FROM descriptions;";

    rc = sqlite3_exec(db, "BEGIN IMMEDIATE TRANSACTION;", nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return;
    }

    rc = sqlite3_exec(db, delete_pending_sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        goto rollback;
    }

    rc = sqlite3_exec(db, delete_completed_sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        goto rollback;
    }

    rc = sqlite3_exec(db, delete_descriptions_sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        goto rollback;
    }

    for (Todo* t : pending_todos) {
        const char *insert_sql = "INSERT INTO pending (id, heading, description, created, completed) VALUES (?, ?, ?, ?, ?);";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr) != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
            continue;
        }

        sqlite3_bind_int(stmt, 1, pending_index++);
        sqlite3_bind_text(stmt, 2, t->heading.data, -1, SQLITE_STATIC);

        // Keep description column null; real descriptions live in descriptions table
        sqlite3_bind_null(stmt, 3);

        sqlite3_bind_text(stmt, 4, t->created.data, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 5, t->completed ? 1 : 0);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        }

        sqlite3_finalize(stmt);

        // Insert all descriptions for this todo into descriptions table
        sqlite3_stmt* desc_stmt = nullptr;
        const char *insert_desc_sql = "INSERT INTO descriptions (todo_id, description, added, completed) VALUES (?, ?, ?, ?);";
        if (sqlite3_prepare_v2(db, insert_desc_sql, -1, &desc_stmt, nullptr) != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare description statement: %s\n", sqlite3_errmsg(db));
            continue;
        }
        for (Info* info : t->info) {
            sqlite3_bind_int(desc_stmt, 1, pending_index - 1);
            sqlite3_bind_text(desc_stmt, 2, info->description.data, -1, SQLITE_STATIC);
            sqlite3_bind_text(desc_stmt, 3, info->added.data, -1, SQLITE_STATIC);
            sqlite3_bind_int(desc_stmt, 4, 0);
            if (sqlite3_step(desc_stmt) != SQLITE_DONE) {
                fprintf(stderr, "Failed to insert description: %s\n", sqlite3_errmsg(db));
            }
            sqlite3_reset(desc_stmt);
            sqlite3_clear_bindings(desc_stmt);
        }
        sqlite3_finalize(desc_stmt);
    }

    for (Todo* t : completed_todos) {
        const char *insert_sql = "INSERT INTO completed (id, heading, description, created, completed) VALUES (?, ?, ?, ?, ?);";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr) != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
            continue;
        }

        sqlite3_bind_int(stmt, 1, completed_index++);
        sqlite3_bind_text(stmt, 2, t->heading.data, -1, SQLITE_STATIC);

        sqlite3_bind_null(stmt, 3);

        sqlite3_bind_text(stmt, 4, t->created.data, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 5, t->completed ? 1 : 0);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        }

        sqlite3_finalize(stmt);

        sqlite3_stmt* desc_stmt = nullptr;
        const char *insert_desc_sql = "INSERT INTO descriptions (todo_id, description, added, completed) VALUES (?, ?, ?, ?);";
        if (sqlite3_prepare_v2(db, insert_desc_sql, -1, &desc_stmt, nullptr) != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare description statement: %s\n", sqlite3_errmsg(db));
            continue;
        }
        for (Info* info : t->info) {
            sqlite3_bind_int(desc_stmt, 1, completed_index - 1);
            sqlite3_bind_text(desc_stmt, 2, info->description.data, -1, SQLITE_STATIC);
            sqlite3_bind_text(desc_stmt, 3, info->added.data, -1, SQLITE_STATIC);
            sqlite3_bind_int(desc_stmt, 4, 1);
            if (sqlite3_step(desc_stmt) != SQLITE_DONE) {
                fprintf(stderr, "Failed to insert description: %s\n", sqlite3_errmsg(db));
            }
            sqlite3_reset(desc_stmt);
            sqlite3_clear_bindings(desc_stmt);
        }
        sqlite3_finalize(desc_stmt);
    }

    rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
    return;

rollback:
    sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
}