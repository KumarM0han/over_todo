#include "Todo.h"

#include "imgui.h"
#include <cstdio>

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

    Ui_AddTodoPopup();
    
    ImGui::NewLine();
    ImGui::SeparatorText("Todo List");

    Ui_TodoList();

    ImGui::PopStyleColor(1);
    ImGui::End();
}

void UiData::Ui_TodoList() {
    size_t l = 0;
    for (auto i = pending_todos.begin(); i != pending_todos.end(); i++) {
        assert(*i != nullptr);
        assert((*i)->heading.data != nullptr);
        assert(strlen((*i)->heading.data) > 0);
        if (ImGui::Selectable((*i)->heading.data, ui_selected_todo == *i)) {
            ui_selected_todo = *i;
        }
        l++;
    }
}

void UiData::Ui_AddTodoPopup() {
    if (ImGui::BeginPopupModal("Add Todo Popup")) {
        ImGui::PushItemWidth(-FLT_MIN);
        ImGui::Text("%s", "Add Todo");
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
            new_todo->created.data = (char *)calloc(DATE_MAX_SIZE, sizeof(char));
            new_todo->created.size = DATE_MAX_SIZE;
            new_todo->heading.data = (char *)calloc(HEADING_MAX_SIZE, sizeof(char));
            new_todo->heading.size = HEADING_MAX_SIZE;

            if (strlen(ui_last_description_entry.data) > 0) {
                Info *new_info = new Info();
                new_info->added.data = (char *)calloc(DATE_MAX_SIZE, sizeof(char));
                new_info->added.size = DATE_MAX_SIZE;
                new_info->description.data = (char *) calloc(DESCRIPTION_MAX_SIZE, sizeof(char));
                new_info->description.size = DESCRIPTION_MAX_SIZE;
                strncpy(new_info->description.data, ui_last_description_entry.data, DESCRIPTION_MAX_SIZE);
                new_todo->info.push_front(new_info);
                memset(ui_last_description_entry.data, 0, DESCRIPTION_MAX_SIZE);
            }
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