#include "Todo.h"

#include <cstdio>
#include "imgui.h"
#include "Utils.h"

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
            ImGui::OpenPopup("Add Description Popup");
        }
        ImGui::EndDisabled();

        Ui_AppendDescriptionPopup();

        ImGui::SameLine();

        if (ImGui::Button("Toggle Completion")) {
            ui_selected_todo->completed = !ui_selected_todo->completed;
            if (ui_selected_todo->completed) {
                pending_todos.remove(ui_selected_todo);
                completed_todos.push_front(ui_selected_todo);
            } else {
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
            memset(ui_last_header_entry.data, 0, HEADING_MAX_SIZE);
            memset(ui_last_description_entry.data, 0, DESCRIPTION_MAX_SIZE);
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}