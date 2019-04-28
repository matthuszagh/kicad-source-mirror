/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2019 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <common.h>
#include <eeschema_id.h>
#include <tool/tool_manager.h>
#include <tool/common_tools.h>
#include <tools/sch_editor_control.h>
#include <tools/sch_picker_tool.h>
#include <tools/sch_drawing_tool.h>
#include <tools/sch_selection_tool.h>
#include <tools/sch_actions.h>
#include <tools/sch_edit_tool.h>
#include <tool/zoom_tool.h>

OPT<TOOL_EVENT> SCH_ACTIONS::TranslateLegacyId( int aId )
{
    switch( aId )
    {
    case ID_NO_TOOL_SELECTED:
        return SCH_ACTIONS::selectionActivate.MakeEvent();

    case ID_CANCEL_CURRENT_COMMAND:
        return ACTIONS::cancelInteractive.MakeEvent();

    case ID_ZOOM_REDRAW:
    case ID_POPUP_ZOOM_REDRAW:
    case ID_VIEWER_ZOOM_REDRAW:
        return ACTIONS::zoomRedraw.MakeEvent();

    case ID_POPUP_ZOOM_IN:
        return ACTIONS::zoomIn.MakeEvent();

    case ID_ZOOM_IN:
    case ID_VIEWER_ZOOM_IN:
        return ACTIONS::zoomInCenter.MakeEvent();

    case ID_POPUP_ZOOM_OUT:
        return ACTIONS::zoomOut.MakeEvent();

    case ID_ZOOM_OUT:
    case ID_VIEWER_ZOOM_OUT:
        return ACTIONS::zoomOutCenter.MakeEvent();

    case ID_POPUP_ZOOM_PAGE:
    case ID_ZOOM_PAGE:      // toolbar button "Fit on Screen"
    case ID_VIEWER_ZOOM_PAGE:
        return ACTIONS::zoomFitScreen.MakeEvent();

    case ID_POPUP_ZOOM_CENTER:
        return ACTIONS::zoomCenter.MakeEvent();

    case ID_ZOOM_SELECTION:
        return ACTIONS::zoomTool.MakeEvent();

    case ID_POPUP_GRID_NEXT:
        return ACTIONS::gridNext.MakeEvent();

    case ID_POPUP_GRID_PREV:
        return ACTIONS::gridPrev.MakeEvent();

    case ID_HIGHLIGHT_BUTT:
        return SCH_ACTIONS::highlightNetCursor.MakeEvent();

    case ID_HIGHLIGHT_NET:
        return SCH_ACTIONS::highlightNet.MakeEvent();

    case ID_MENU_PLACE_COMPONENT:
    case ID_SCH_PLACE_COMPONENT:
        return SCH_ACTIONS::placeSymbol.MakeEvent();

    case ID_MENU_PLACE_POWER_BUTT:
    case ID_PLACE_POWER_BUTT:
        return SCH_ACTIONS::placePower.MakeEvent();

    case ID_POPUP_SCH_BEGIN_WIRE:
        return SCH_ACTIONS::startWire.MakeEvent();

    case ID_MENU_WIRE_BUTT:
    case ID_WIRE_BUTT:
        return SCH_ACTIONS::drawWire.MakeEvent();

    case ID_POPUP_SCH_BEGIN_BUS:
        return SCH_ACTIONS::startBus.MakeEvent();

    case ID_MENU_BUS_BUTT:
    case ID_BUS_BUTT:
        return SCH_ACTIONS::drawBus.MakeEvent();

    case ID_MENU_NOCONN_BUTT:
    case ID_NOCONN_BUTT:
        return SCH_ACTIONS::placeNoConnect.MakeEvent();

    case ID_POPUP_SCH_ADD_JUNCTION:
        return SCH_ACTIONS::addJunction.MakeEvent();

    case ID_MENU_JUNCTION_BUTT:
    case ID_JUNCTION_BUTT:
        return SCH_ACTIONS::placeJunction.MakeEvent();

    case ID_MENU_WIRETOBUS_ENTRY_BUTT:
    case ID_WIRETOBUS_ENTRY_BUTT:
        return SCH_ACTIONS::placeBusWireEntry.MakeEvent();

    case ID_MENU_BUSTOBUS_ENTRY_BUTT:
    case ID_BUSTOBUS_ENTRY_BUTT:
        return SCH_ACTIONS::placeBusBusEntry.MakeEvent();

    case ID_POPUP_SCH_ADD_LABEL:
        return SCH_ACTIONS::addLabel.MakeEvent();

    case ID_MENU_LABEL_BUTT:
    case ID_LABEL_BUTT:
        return SCH_ACTIONS::placeLabel.MakeEvent();

    case ID_POPUP_SCH_ADD_GLABEL:
        return SCH_ACTIONS::addGlobalLabel.MakeEvent();

    case ID_MENU_GLABEL_BUTT:
    case ID_GLOBALLABEL_BUTT:
        return SCH_ACTIONS::placeGlobalLabel.MakeEvent();

    case ID_MENU_HIERLABEL_BUTT:
    case ID_HIERLABEL_BUTT:
        return SCH_ACTIONS::placeHierarchicalLabel.MakeEvent();

    case ID_MENU_SHEET_PIN_BUTT:
    case ID_SHEET_PIN_BUTT:
        return SCH_ACTIONS::placeSheetPin.MakeEvent();

    case ID_POPUP_IMPORT_HLABEL_TO_SHEETPIN:
    case ID_MENU_IMPORT_HLABEL_BUTT:
    case ID_IMPORT_HLABEL_BUTT:
        return SCH_ACTIONS::importSheetPin.MakeEvent();

    case ID_MENU_SHEET_SYMBOL_BUTT:
    case ID_SHEET_SYMBOL_BUTT:
        return SCH_ACTIONS::drawSheet.MakeEvent();

    case ID_POPUP_SCH_RESIZE_SHEET:
        return SCH_ACTIONS::resizeSheet.MakeEvent();

    case ID_MENU_TEXT_COMMENT_BUTT:
    case ID_TEXT_COMMENT_BUTT:
        return SCH_ACTIONS::placeSchematicText.MakeEvent();

    case ID_MENU_LINE_COMMENT_BUTT:
    case ID_LINE_COMMENT_BUTT:
        return SCH_ACTIONS::drawLines.MakeEvent();

    case ID_MENU_ADD_IMAGE_BUTT:
    case ID_ADD_IMAGE_BUTT:
        return SCH_ACTIONS::placeImage.MakeEvent();

    case ID_POPUP_END_LINE:
        return SCH_ACTIONS::finishDrawing.MakeEvent();

    case ID_MENU_DELETE_ITEM_BUTT:
    case ID_SCHEMATIC_DELETE_ITEM_BUTT:
        return SCH_ACTIONS::deleteItemCursor.MakeEvent();

    case ID_SCH_MOVE:
        return SCH_ACTIONS::move.MakeEvent();

    case ID_SCH_DRAG:
        return SCH_ACTIONS::drag.MakeEvent();

    case ID_SCH_DELETE:
        return SCH_ACTIONS::remove.MakeEvent();

    case ID_SIM_PROBE:
        return SCH_ACTIONS::simProbe.MakeEvent();

    case ID_SIM_TUNE:
        return SCH_ACTIONS::simTune.MakeEvent();

    case ID_SCH_ROTATE_CLOCKWISE:
        return SCH_ACTIONS::rotateCW.MakeEvent();

    case ID_SCH_ROTATE_COUNTERCLOCKWISE:
        return SCH_ACTIONS::rotateCCW.MakeEvent();

    case ID_SCH_MIRROR_X:
        return SCH_ACTIONS::mirrorX.MakeEvent();

    case ID_SCH_MIRROR_Y:
        return SCH_ACTIONS::mirrorY.MakeEvent();

    case ID_SCH_DUPLICATE:
        return SCH_ACTIONS::duplicate.MakeEvent();

    case ID_REPEAT_BUTT:
        return SCH_ACTIONS::repeatDrawItem.MakeEvent();

    case ID_SCH_EDIT_ITEM:
        return SCH_ACTIONS::properties.MakeEvent();

    case ID_SCH_EDIT_COMPONENT_REFERENCE:
        return SCH_ACTIONS::editReference.MakeEvent();

    case ID_SCH_EDIT_COMPONENT_VALUE:
        return SCH_ACTIONS::editValue.MakeEvent();

    case ID_SCH_EDIT_COMPONENT_FOOTPRINT:
        return SCH_ACTIONS::editFootprint.MakeEvent();
    }

    return OPT<TOOL_EVENT>();
}


void SCH_ACTIONS::RegisterAllTools( TOOL_MANAGER* aToolManager )
{
    aToolManager->RegisterTool( new COMMON_TOOLS );
    aToolManager->RegisterTool( new ZOOM_TOOL );
    aToolManager->RegisterTool( new SCH_SELECTION_TOOL );
    aToolManager->RegisterTool( new SCH_EDITOR_CONTROL );
    aToolManager->RegisterTool( new SCH_PICKER_TOOL );
    aToolManager->RegisterTool( new SCH_DRAWING_TOOL );
    aToolManager->RegisterTool( new SCH_EDIT_TOOL );
}
