#pragma once
#include <imgui.h>
#include <fmt/format.h>

class ListView {
public:
    enum MyItemColumnID {
        MyItemColumnID_ID,
        MyItemColumnID_Name,
        MyItemColumnID_Action,
        MyItemColumnID_Quantity,
        MyItemColumnID_Description
    };

    struct MyItem {
        int ID;
        std::string Name;
        int Quantity;
    };

    void Render();
};