#include "ListView.h"

#include <algorithm>
#include <vector>

bool CompareWithSortSpecs(ImGuiTableSortSpecs *sort_specs, const ListView::MyItem *a, const ListView::MyItem *b) {
    for (int n = 0; n < sort_specs->SpecsCount; n++) {
        // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
        // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is
        // simpler!
        const ImGuiTableColumnSortSpecs *sort_spec = &sort_specs->Specs[n];
        switch (sort_spec->ColumnUserID) {
        case ListView::MyItemColumnID::MyItemColumnID_ID:
            if (a->ID == b->ID) {
                break;
            }
            if (sort_spec->SortDirection == ImGuiSortDirection_Ascending)
                return a->ID < b->ID;
            else
                return a->ID > b->ID;
        case ListView::MyItemColumnID::MyItemColumnID_Name:
            if (a->Name == b->Name) {
                break;
            }
            if (sort_spec->SortDirection == ImGuiSortDirection_Ascending)
                return a->Name < b->Name;
            else
                return a->Name > b->Name;
            break;
        case ListView::MyItemColumnID::MyItemColumnID_Quantity:
            if (a->Quantity == b->Quantity) {
                break;
            }
            if (sort_spec->SortDirection == ImGuiSortDirection_Ascending)
                return a->Quantity < b->Quantity;
            else
                return a->Quantity > b->Quantity;
            break;
        case ListView::MyItemColumnID::MyItemColumnID_Description:
            if (a->Name == b->Name) {
                break;
            }
            if (sort_spec->SortDirection == ImGuiSortDirection_Ascending)
                return a->Name < b->Name;
            else
                return a->Name > b->Name;
            break;
        default:
            IM_ASSERT(0);
            break;
        }
    }

    return false;
}

void ListView::Render() {
    static const char *template_items_names[] = {u8"香蕉", u8"苹果", u8"樱桃",   u8"西瓜", u8"葡萄柚",
                                                 u8"草莓", u8"芒果", u8"猕猴桃", u8"橙子", u8"菠萝",
                                                 u8"蓝莓", u8"李子", u8"椰子",   u8"梨",   u8"杏"};

    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
    // Create item list
    static std::vector<MyItem> items;
    if (items.size() == 0) {
        items.resize(50, MyItem());
        for (int n = 0; n < items.size(); n++) {
            const int template_n = n % IM_ARRAYSIZE(template_items_names);
            MyItem &item = items[n];
            item.ID = n;
            item.Name = template_items_names[template_n];
            item.Quantity = (n * n - n) % 20; // Assign default quantities
        }
    }

    // Options
    static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
                                   ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti | ImGuiTableFlags_RowBg |
                                   ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
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
                                ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_WidthStretch, 0.0f,
                                MyItemColumnID_Quantity);
        ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
        ImGui::TableHeadersRow();

        // Sort our data if sort specs have been changed!
        if (ImGuiTableSortSpecs *sort_specs = ImGui::TableGetSortSpecs())
            if (sort_specs->SpecsDirty) {
                std::sort(items.begin(), items.end(), [sort_specs](const MyItem &lhs, const MyItem &rhs) -> bool {
                    return CompareWithSortSpecs(sort_specs, &lhs, &rhs);
                });
                sort_specs->SpecsDirty = false;
            }

        // Demonstrate using clipper for large vertical lists
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(items.size()));
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
                ImGui::TextUnformatted(item->Name.c_str());
                ImGui::TableNextColumn();
                ImGui::SmallButton("None");
                ImGui::TableNextColumn();
                ImGui::Text("%d", item->Quantity);
                ImGui::PopID();
            }
        ImGui::EndTable();
    }
}
