#pragma once
#include <vector>
#include <Windows.h>
#include "..\Common\String.h"

class TElement;
namespace DialogAnimation
{
	enum enumListBoxItemType{ D, V, A };
	String GetTypeName(enumListBoxItemType type);

	enum enumListBoxItemValueType{ X, Y, PHI };
	String GetUnitName(enumListBoxItemType type, enumListBoxItemValueType value_type);

	struct TListBoxItem
	{
		int id;
		int index_of_point;
		enumListBoxItemType type;//P,V,A
		enumListBoxItemValueType value_type;//X,Y,PHI
		String s;//ÏîÄ¿Ãû³Æ
		String sUnit;
		std::vector<double> data;
		TElement *pElement;
	};

	INT_PTR CALLBACK DlgAnimationProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

}