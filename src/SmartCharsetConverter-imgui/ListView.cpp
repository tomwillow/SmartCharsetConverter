#include "ListView.h"

#include <algorithm>
#include <vector>

/**
 * @return <return_value, should_break>
 */
template <std::size_t ItemIndex>
std::tuple<bool, bool> Compare(const ListView::MyItem *a, const ListView::MyItem *b, bool ascending) {
    if (std::get<ItemIndex>(a->AsTuple()) == std::get<ItemIndex>(b->AsTuple())) {
        return {false, true};
    }

    bool isLessThan = std::get<ItemIndex>(a->AsTuple()) < std::get<ItemIndex>(b->AsTuple());
    return {ascending ? isLessThan : !isLessThan, false};
}

bool CompareWithSortSpecs(ImGuiTableSortSpecs *sort_specs, const ListView::MyItem *a, const ListView::MyItem *b) {
    for (int n = 0; n < sort_specs->SpecsCount; n++) {
        // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
        // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is
        // simpler!
        const ImGuiTableColumnSortSpecs *sort_spec = &sort_specs->Specs[n];

        switch (sort_spec->ColumnUserID) {
        case 0: {
            auto [returnValue, shouldBreak] =
                Compare<0>(a, b, sort_spec->SortDirection == ImGuiSortDirection_Ascending);
            if (shouldBreak) {
                break;
            }
            return returnValue;
        }
        case 1: {
            auto [returnValue, shouldBreak] =
                Compare<1>(a, b, sort_spec->SortDirection == ImGuiSortDirection_Ascending);
            if (shouldBreak) {
                break;
            }
            return returnValue;
        }
        case 2: {
            auto [returnValue, shouldBreak] =
                Compare<2>(a, b, sort_spec->SortDirection == ImGuiSortDirection_Ascending);
            if (shouldBreak) {
                break;
            }
            return returnValue;
        }
        case 3: {
            auto [returnValue, shouldBreak] =
                Compare<3>(a, b, sort_spec->SortDirection == ImGuiSortDirection_Ascending);
            if (shouldBreak) {
                break;
            }
            return returnValue;
        }
        case 4: {
            auto [returnValue, shouldBreak] =
                Compare<4>(a, b, sort_spec->SortDirection == ImGuiSortDirection_Ascending);
            if (shouldBreak) {
                break;
            }
            return returnValue;
        }
        case 5: {
            auto [returnValue, shouldBreak] =
                Compare<5>(a, b, sort_spec->SortDirection == ImGuiSortDirection_Ascending);
            if (shouldBreak) {
                break;
            }
            return returnValue;
        }
        default:
            IM_ASSERT(0);
            break;
        }
    }

    return false;
}

ListView::ListView(LanguageService &languageService) noexcept : languageService(languageService) {

    static const char *template_items_names[] = {u8"香蕉", u8"苹果", u8"樱桃",   u8"西瓜", u8"葡萄柚",
                                                 u8"草莓", u8"芒果", u8"猕猴桃", u8"橙子", u8"菠萝",
                                                 u8"蓝莓", u8"李子", u8"椰子",   u8"梨",   u8"杏"};

    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
    // Create item list
    if (items.size() == 0) {
        items.resize(50, MyItem());
        for (int n = 0; n < items.size(); n++) {
            const int template_n = n % IM_ARRAYSIZE(template_items_names);
            MyItem &item = items[n];
            item.index = n;
            item.fileName = template_items_names[template_n];
            item.fileSize = (n * n - n) % 20; // Assign default quantities
        }
    }
}

void ListView::Render() {
    std::vector<MyItem> itemsTemp;
    {
        std::unique_lock ul(itemsLock);
        itemsTemp.swap(itemsQueue);
    }
    items.insert(items.end(), itemsTemp.begin(), itemsTemp.end());

    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

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

    if (ImGui::BeginTable("table_sorting", static_cast<int>(ListViewColumn::TEXT_PIECE) + 1, flags,
                          ImVec2(0.0f, TEXT_BASE_HEIGHT * 15), 0.0f)) {
        // Declare columns
        // We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be stored in the
        // sort specifications. This is so our sort function can identify a column given our own identifier. We
        // could also identify them based on their index! Demonstrate using a mixture of flags among available
        // sort-related flags:
        // - ImGuiTableColumnFlags_DefaultSort
        // - ImGuiTableColumnFlags_NoSort / ImGuiTableColumnFlags_NoSortAscending /
        // ImGuiTableColumnFlags_NoSortDescending
        // - ImGuiTableColumnFlags_PreferSortAscending / ImGuiTableColumnFlags_PreferSortDescending
        ImGui::TableSetupColumn(languageService.GetUtf8String(v0_2::StringId::INDEX).c_str(),
                                ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 0.0f,
                                static_cast<ImGuiID>(ListViewColumn::INDEX));
        ImGui::TableSetupColumn(languageService.GetUtf8String(v0_2::StringId::FILENAME).c_str(),
                                ImGuiTableColumnFlags_WidthFixed, 0.0f, static_cast<ImGuiID>(ListViewColumn::FILENAME));
        ImGui::TableSetupColumn(languageService.GetUtf8String(v0_2::StringId::SIZE).c_str(),
                                ImGuiTableColumnFlags_WidthFixed, 0.0f, static_cast<ImGuiID>(ListViewColumn::FILESIZE));
        ImGui::TableSetupColumn(languageService.GetUtf8String(v0_2::StringId::ENCODING).c_str(),
                                ImGuiTableColumnFlags_WidthFixed, 0.0f, static_cast<ImGuiID>(ListViewColumn::ENCODING));
        ImGui::TableSetupColumn(languageService.GetUtf8String(v0_2::StringId::LINE_BREAKS).c_str(),
                                ImGuiTableColumnFlags_WidthFixed, 0.0f,
                                static_cast<ImGuiID>(ListViewColumn::LINE_BREAK));
        ImGui::TableSetupColumn(languageService.GetUtf8String(v0_2::StringId::TEXT_PIECE).c_str(),
                                ImGuiTableColumnFlags_WidthStretch, 0.0f,
                                static_cast<ImGuiID>(ListViewColumn::TEXT_PIECE));
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
                ImGui::PushID(item->index);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                // ImGui::Text("%04d", item->ID);

                {
                    static ImVector<int> selection;
                    const bool item_is_selected = selection.contains(item->index);
                    std::string label = fmt::format("{}", item->index);

                    ImGuiSelectableFlags selectable_flags =
                        ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap;
                    if (ImGui::Selectable(label.c_str(), item_is_selected, selectable_flags, ImVec2(0, 0))) {
                        if (ImGui::GetIO().KeyCtrl) {
                            if (item_is_selected)
                                selection.find_erase_unsorted(item->index);
                            else
                                selection.push_back(item->index);
                        } else {
                            selection.clear();
                            selection.push_back(item->index);
                        }
                    }
                }

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(item->fileName.c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%d", item->fileSize);
                ImGui::TableNextColumn();
                ImGui::PopID();
            }

        // if in this frame there are new items to add, to scroll table to bottom
        if (!itemsTemp.empty()) {
            ImGui::SetScrollHereY(1.0);
        }

        ImGui::EndTable();
    }
}

void ListView::AddItem(MyItem myItem) {
    std::unique_lock ul(itemsLock);
    itemsQueue.push_back(std::move(myItem));
}
