#pragma once
#include <imgui.h>
#include <fmt/format.h>

class ListView {
public:
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

        // Compare function to be used by qsort()
        static int __cdecl CompareWithSortSpecs(const void *lhs, const void *rhs);

        static void SortWithSortSpecs(ImGuiTableSortSpecs *sort_specs, MyItem *items, int items_count);
    };

    void Render();
};