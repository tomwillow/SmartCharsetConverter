#pragma once

#include "UnicodeTable.h"

#include <imgui.h>
#include <fmt/format.h>

class MainWindow {
public:
    MainWindow() {
        uni_table::InitUtf8Table();
    }

    void Render() {

        ImGui::StyleColorsLight();

        {
            ImGuiStyle &style = ImGui::GetStyle();
            style.FrameBorderSize = 1.0f;
            style.ScrollbarSize = 20.0f;
            style.ScrollbarRounding = 0.0f;
        }

        {
            bool open = true;
            static bool use_work_area = true;
            static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;

            // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
            // Based on your use case you may want one or the other.
            const ImGuiViewport *viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
            ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

            if (ImGui::Begin("Example: Simple layout", &open, flags)) {
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("File")) {
                        if (ImGui::MenuItem("Close", "Ctrl+W")) {
                            //*p_open = false;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Left
                static int selected = 0;
                {
                    ImGui::BeginGroup();
                    ImGui::BeginChild("left pane", ImVec2(150, 0), ImGuiChildFlags_ResizeX);
                    ImGui::SeparatorText(u8"文件列表");
                    {
                        ImGui::BeginChild("left pane", ImVec2(0, 0), ImGuiChildFlags_Border);
                        // for (int i = 0; i < 100; i++) {
                        //     // FIXME: Good candidate to use ImGuiSelectableFlags_SelectOnNav
                        //     char label[128];
                        //     sprintf(label, "MyObject %d", i);
                        //     if (ImGui::Selectable(label, selected == i))
                        //         selected = i;
                        // }
                        DrawListView();
                        ImGui::EndChild();
                    }
                    ImGui::EndChild();
                    ImGui::EndGroup();
                }
                ImGui::SameLine();

                // Right
                {
                    ImGui::BeginGroup();
                    ImGui::BeginChild("item view",
                                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
                    ImGui::Text("MyObject: %d", selected);
                    ImGui::Separator();
                    if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None)) {
                        if (ImGui::BeginTabItem("Description")) {
                            ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do "
                                               "eiusmod tempor incididunt ut labore et dolore magna aliqua. ");
                            ImGui::EndTabItem();
                        }
                        if (ImGui::BeginTabItem("Details")) {
                            ImGui::Text("ID: 0123456789");
                            ImGui::EndTabItem();
                        }
                        ImGui::EndTabBar();
                    }
                    ImGui::EndChild();
                    if (ImGui::Button("Revert")) {}
                    ImGui::SameLine();
                    if (ImGui::Button("Save")) {}
                    ImGui::EndGroup();
                }
            }
            ImGui::End();
        }

        {
            static uni_table::ExampleAssetsBrowser brower;
            bool open = true;
            brower.Draw("aa", &open);
        }

        // 在这里放置你所有的 UI 控件
        static bool show_demo_window = true;
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);
    }

    // We are passing our own identifier to TableSetupColumn() to facilitate identifying columns in the sorting code.
    // This identifier will be passed down into ImGuiTableSortSpec::ColumnUserID.
    // But it is possible to omit the user id parameter of TableSetupColumn() and just use the column index instead!
    // (ImGuiTableSortSpec::ColumnIndex) If you don't use sorting, you will generally never care about giving column an
    // ID!
    enum MyItemColumnID {
        MyItemColumnID_ID,
        MyItemColumnID_Name,
        MyItemColumnID_Action,
        MyItemColumnID_Quantity,
        MyItemColumnID_Description
    };

    struct MyItem {
        int ID;
        const char *Name;
        int Quantity;

        // We have a problem which is affecting _only this demo_ and should not affect your code:
        // As we don't rely on std:: or other third-party library to compile dear imgui, we only have reliable access to
        // qsort(), however qsort doesn't allow passing user data to comparing function. As a workaround, we are storing
        // the sort specs in a static/global for the comparing function to access. In your own use case you would
        // probably pass the sort specs to your sorting/comparing functions directly and not use a global. We could
        // technically call ImGui::TableGetSortSpecs() in CompareWithSortSpecs(), but considering that this function is
        // called very often by the sorting algorithm it would be a little wasteful.
        static const ImGuiTableSortSpecs *s_current_sort_specs;

        static void SortWithSortSpecs(ImGuiTableSortSpecs *sort_specs, MyItem *items, int items_count) {
            s_current_sort_specs = sort_specs; // Store in variable accessible by the sort function.
            if (items_count > 1)
                qsort(items, (size_t)items_count, sizeof(items[0]), MyItem::CompareWithSortSpecs);
            s_current_sort_specs = NULL;
        }

        // Compare function to be used by qsort()
        static int IMGUI_CDECL CompareWithSortSpecs(const void *lhs, const void *rhs) {
            const MyItem *a = (const MyItem *)lhs;
            const MyItem *b = (const MyItem *)rhs;
            for (int n = 0; n < s_current_sort_specs->SpecsCount; n++) {
                // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
                // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is
                // simpler!
                const ImGuiTableColumnSortSpecs *sort_spec = &s_current_sort_specs->Specs[n];
                int delta = 0;
                switch (sort_spec->ColumnUserID) {
                case MyItemColumnID_ID:
                    delta = (a->ID - b->ID);
                    break;
                case MyItemColumnID_Name:
                    delta = (strcmp(a->Name, b->Name));
                    break;
                case MyItemColumnID_Quantity:
                    delta = (a->Quantity - b->Quantity);
                    break;
                case MyItemColumnID_Description:
                    delta = (strcmp(a->Name, b->Name));
                    break;
                default:
                    IM_ASSERT(0);
                    break;
                }
                if (delta > 0)
                    return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
                if (delta < 0)
                    return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
            }

            // qsort() is instable so always return a way to differenciate items.
            // Your own compare function may want to avoid fallback on implicit sort specs.
            // e.g. a Name compare if it wasn't already part of the sort specs.
            return (a->ID - b->ID);
        }
    };

    void DrawListView() {
        static const char *template_items_names[] = {"Banana",     "Apple", "Cherry",  "Watermelon", "Grapefruit",
                                                     "Strawberry", "Mango", "Kiwi",    "Orange",     "Pineapple",
                                                     "Blueberry",  "Plum",  "Coconut", "Pear",       "Apricot"};

        const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
        // Create item list
        static ImVector<MyItem> items;
        if (items.Size == 0) {
            items.resize(50, MyItem());
            for (int n = 0; n < items.Size; n++) {
                const int template_n = n % IM_ARRAYSIZE(template_items_names);
                MyItem &item = items[n];
                item.ID = n;
                item.Name = template_items_names[template_n];
                item.Quantity = (n * n - n) % 20; // Assign default quantities
            }
        }

        // Options
        static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
                                       ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti |
                                       ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
                                       ImGuiTableFlags_ScrollY; //  ImGuiTableFlags_NoBordersInBody |
        // PushStyleCompact();
        // ImGui::CheckboxFlags("ImGuiTableFlags_SortMulti", &flags, ImGuiTableFlags_SortMulti);
        // ImGui::SameLine();
        // HelpMarker("When sorting is enabled: hold shift when clicking headers to sort on multiple column. "
        //            "TableGetSortSpecs() may return specs where (SpecsCount > 1).");
        // ImGui::CheckboxFlags("ImGuiTableFlags_SortTristate", &flags, ImGuiTableFlags_SortTristate);
        // ImGui::SameLine();
        // HelpMarker("When sorting is enabled: allow no sorting, disable default sorting. TableGetSortSpecs() may "
        //            "return specs where (SpecsCount == 0).");
        // PopStyleCompact();

        if (ImGui::BeginTable("table_sorting", 4, flags, ImVec2(0.0f, TEXT_BASE_HEIGHT * 15), 0.0f)) {
            // Declare columns
            // We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be stored in the
            // sort specifications. This is so our sort function can identify a column given our own identifier. We
            // could also identify them based on their index! Demonstrate using a mixture of flags among available
            // sort-related flags:
            // - ImGuiTableColumnFlags_DefaultSort
            // - ImGuiTableColumnFlags_NoSort / ImGuiTableColumnFlags_NoSortAscending /
            // ImGuiTableColumnFlags_NoSortDescending
            // - ImGuiTableColumnFlags_PreferSortAscending / ImGuiTableColumnFlags_PreferSortDescending
            ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 0.0f,
                                    MyItemColumnID_ID);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 0.0f, MyItemColumnID_Name);
            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, 0.0f,
                                    MyItemColumnID_Action);
            ImGui::TableSetupColumn("Quantity",
                                    ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_WidthStretch,
                                    0.0f, MyItemColumnID_Quantity);
            ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
            ImGui::TableHeadersRow();

            // Sort our data if sort specs have been changed!
            if (ImGuiTableSortSpecs *sort_specs = ImGui::TableGetSortSpecs())
                if (sort_specs->SpecsDirty) {
                    MyItem::SortWithSortSpecs(sort_specs, items.Data, items.Size);
                    sort_specs->SpecsDirty = false;
                }

            // Demonstrate using clipper for large vertical lists
            ImGuiListClipper clipper;
            clipper.Begin(items.Size);
            while (clipper.Step())
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                    // Display a data item
                    MyItem *item = &items[row_n];
                    ImGui::PushID(item->ID);
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    // ImGui::Text("%04d", item->ID);

                    {
                        static ImVector<int> selection;
                        const bool item_is_selected = selection.contains(item->ID);
                        std::string label = fmt::format("{}", item->ID);

                        ImGuiSelectableFlags selectable_flags =
                            ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap;
                        if (ImGui::Selectable(label.c_str(), item_is_selected, selectable_flags, ImVec2(0, 0))) {
                            if (ImGui::GetIO().KeyCtrl) {
                                if (item_is_selected)
                                    selection.find_erase_unsorted(item->ID);
                                else
                                    selection.push_back(item->ID);
                            } else {
                                selection.clear();
                                selection.push_back(item->ID);
                            }
                        }
                    }

                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(item->Name);
                    ImGui::TableNextColumn();
                    ImGui::SmallButton("None");
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", item->Quantity);
                    ImGui::PopID();
                }
            ImGui::EndTable();
        }
    }
};
const ImGuiTableSortSpecs *MainWindow::MyItem::s_current_sort_specs = NULL;