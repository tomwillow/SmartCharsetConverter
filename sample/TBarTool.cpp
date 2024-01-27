#pragma once
#include "TBarTool.h"

#include "..\Control\TTreeViewContent.h"
#include "..\Element\TShape.h"
#include "..\Element\TBar.h"
#include "..\Element\TConstraintCoincide.h"

TBarTool::TBarTool()
{
	myElementType = ELEMENT_BAR;

	sType = TEXT("连杆");
	bCanBuildCoincide = true;
}


TBarTool::~TBarTool()
{
}

TElement * TBarTool::AddIntoShape(TRealLine &RealLine)
{
	RealLine.vecDpt.push_back(RealLine.GetRelativePointByIndex(0));
	RealLine.vecDpt.push_back(RealLine.GetRelativePointByIndex(1));

	RealLine.eType = myElementType;
	return pShape->AddElement((TBar *)&RealLine);
}

void TBarTool::AddIntoTreeViewContent(TElement *Element, int id)
{
	if (Element->eType == ELEMENT_REALLINE)
	{
		Element->eType = myElementType;
		TBar Bar;
		Bar= *(TRealLine*)Element;
		pTreeViewContent->AddItem(&Bar, pShape->iNextId);
	}
	else
		TLineTool::AddIntoTreeViewContent(Element, id);
}

//void TBarTool::AddCoincide(TConstraintCoincide *pCoincide, int id, TConfiguration *pConfig)
//{
//	AddIntoTreeViewContent(pCoincide, id);
//	pShape->AddElement(pCoincide);
//}