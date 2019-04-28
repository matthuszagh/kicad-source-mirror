/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2014 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2008 Wayne Stambaugh <stambaughw@gmail.com>
 * Copyright (C) 2004-2018 KiCad Developers, see AUTHORS.txt for contributors.
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

/**
 * @file eeschema/onrightclick.cpp
 */

#include <fctsys.h>
#include <eeschema_id.h>
#include <sch_draw_panel.h>
#include <confirm.h>
#include <sch_edit_frame.h>
#include <menus_helpers.h>

#include <advanced_config.h>
#include <class_library.h>
#include <general.h>
#include <hotkeys.h>
#include <netlist_object.h>
#include <sch_bus_entry.h>
#include <sch_marker.h>
#include <sch_text.h>
#include <sch_junction.h>
#include <sch_component.h>
#include <sch_line.h>
#include <sch_no_connect.h>
#include <sch_sheet.h>
#include <sch_sheet_path.h>
#include <sch_bitmap.h>
#include <symbol_lib_table.h>
#include <sch_connection.h>
#include <sch_view.h>

#include <iostream>
#include <tool/tool_manager.h>
#include <tools/sch_actions.h>
#include <tools/sch_selection_tool.h>

static void AddMenusForBlock( wxMenu* PopMenu, SCH_EDIT_FRAME* frame );
static void AddMenusForWire( wxMenu* PopMenu, SCH_LINE* Wire, SCH_EDIT_FRAME* frame );
static void AddMenusForBus( wxMenu* PopMenu, SCH_LINE* Bus, SCH_EDIT_FRAME* frame );
static void AddMenusForHierchicalSheet( wxMenu* PopMenu, SCH_SHEET* Sheet );
static void AddMenusForSheetPin( wxMenu* PopMenu, SCH_SHEET_PIN* PinSheet );
static void AddMenusForText( wxMenu* PopMenu, SCH_TEXT* Text );
static void AddMenusForLabel( wxMenu* PopMenu, SCH_LABEL* Label );
static void AddMenusForGLabel( wxMenu* PopMenu, SCH_GLOBALLABEL* GLabel );
static void AddMenusForHLabel( wxMenu* PopMenu, SCH_HIERLABEL* GLabel );
static void AddMenusForEditComponent( wxMenu* PopMenu, SCH_COMPONENT* Component,
                                      SYMBOL_LIB_TABLE* aLibs );
static void AddMenusForComponent( wxMenu* PopMenu, SCH_COMPONENT* Component,
                                  SYMBOL_LIB_TABLE* aLibs );
static void AddMenusForComponentField( wxMenu* PopMenu, SCH_FIELD* Field );
static void AddMenusForMarkers( wxMenu* aPopMenu, SCH_MARKER* aMarker, SCH_EDIT_FRAME* aFrame );
static void AddMenusForBitmap( wxMenu* aPopMenu, SCH_BITMAP * aBitmap );
static void AddMenusForBusEntry( wxMenu* aPopMenu, SCH_BUS_ENTRY_BASE * aBusEntry );


bool SCH_EDIT_FRAME::OnRightClick( const wxPoint& aPosition, wxMenu* PopMenu )
{
    SCH_SELECTION_TOOL* selTool = GetToolManager()->GetTool<SCH_SELECTION_TOOL>();
    SCH_ITEM*           item = GetScreen()->GetCurItem();
    bool                blockActive = GetScreen()->IsBlockActive();
    wxString            msg;

    // Ugly hack, clear any highligthed symbol, because the HIGHLIGHT flag create issues when creating menus
    // Will be fixed later
    GetCanvas()->GetView()->HighlightItem( nullptr, nullptr );

    // Do not start a block command on context menu.
    m_canvas->SetCanStartBlock( -1 );

    // Try to locate items at cursor position.
    if( item == NULL || item->GetEditFlags() == 0 )
    {
        bool actionCancelled = false;
        item = selTool->SelectPoint( aPosition, SCH_COLLECTOR::AllItemsButPins, &actionCancelled );

        // If the clarify item selection context menu is aborted, don't show the context menu.
        if( item == NULL && actionCancelled )
            return false;
    }

    // If a command is in progress: add "cancel" and "end tool" menu
    if( GetToolId() != ID_NO_TOOL_SELECTED )
    {
        if( item && item->GetEditFlags() )
        {
            AddMenuItem( PopMenu, ID_CANCEL_CURRENT_COMMAND, _( "Cancel" ),
                         KiBitmap( cancel_xpm ) );
        }
        else
        {
            AddMenuItem( PopMenu, ID_CANCEL_CURRENT_COMMAND, _( "End Tool" ),
                         KiBitmap( cursor_xpm ) );
        }

        PopMenu->AppendSeparator();

        switch( GetToolId() )
        {
        case ID_WIRE_BUTT:
            AddMenusForWire( PopMenu, NULL, this );
            if( item == NULL )
                PopMenu->AppendSeparator();
            break;

        case ID_BUS_BUTT:
            AddMenusForBus( PopMenu, NULL, this );
            if( item == NULL )
                PopMenu->AppendSeparator();
            break;

        default:
            break;
        }
    }
    else
    {
        if( item && item->GetEditFlags() )
        {
            AddMenuItem( PopMenu, ID_CANCEL_CURRENT_COMMAND, _( "Cancel" ),
                         KiBitmap( cancel_xpm ) );
            PopMenu->AppendSeparator();
        }
    }

    if( item == NULL )
    {
        if( g_CurrentSheet->Last() != g_RootSheet )
        {
            msg = AddHotkeyName( _( "Leave Sheet" ), g_Schematic_Hotkeys_Descr, HK_LEAVE_SHEET );
            AddMenuItem( PopMenu, ID_POPUP_SCH_LEAVE_SHEET, msg, KiBitmap( leave_sheet_xpm ) );
            PopMenu->AppendSeparator();
        }

        return true;
    }

    bool is_new = item->IsNew();

    switch( item->Type() )
    {
    case SCH_NO_CONNECT_T:
        AddMenuItem( PopMenu, ID_SCH_DELETE, _( "Delete No Connect" ),
                     KiBitmap( delete_xpm ) );
        break;

    case SCH_JUNCTION_T:
        addJunctionMenuEntries( PopMenu, (SCH_JUNCTION*) item );
        break;

    case SCH_BUS_BUS_ENTRY_T:
    case SCH_BUS_WIRE_ENTRY_T:
        AddMenusForBusEntry( PopMenu, static_cast<SCH_BUS_ENTRY_BASE*>( item ) );
        break;

    case SCH_MARKER_T:
        AddMenusForMarkers( PopMenu, (SCH_MARKER*) item, this );
        break;

    case SCH_TEXT_T:
        AddMenusForText( PopMenu, (SCH_TEXT*) item );
        break;

    case SCH_LABEL_T:
        AddMenusForLabel( PopMenu, (SCH_LABEL*) item );
        break;

    case SCH_GLOBAL_LABEL_T:
        AddMenusForGLabel( PopMenu, (SCH_GLOBALLABEL*) item );
        break;

    case SCH_HIER_LABEL_T:
        AddMenusForHLabel( PopMenu, (SCH_HIERLABEL*) item );
        break;

    case SCH_FIELD_T:
        AddMenusForComponentField( PopMenu, (SCH_FIELD*) item );
        break;

    case SCH_COMPONENT_T:
        AddMenusForComponent( PopMenu, (SCH_COMPONENT*) item, Prj().SchSymbolLibTable() );
        break;

    case SCH_BITMAP_T:
        AddMenusForBitmap( PopMenu, (SCH_BITMAP*) item );
        break;

    case SCH_LINE_T:
        switch( item->GetLayer() )
        {
        case LAYER_WIRE:
            AddMenusForWire( PopMenu, (SCH_LINE*) item, this );
            break;

        case LAYER_BUS:
            AddMenusForBus( PopMenu, (SCH_LINE*) item, this );
            break;

        default:
            if( is_new )
            AddMenuItem( PopMenu, ID_SCH_EDIT_ITEM, _( "Edit..." ),
                         KiBitmap( edit_xpm ) );
            AddMenuItem( PopMenu, ID_SCH_DELETE, _( "Delete Drawing" ),
                         KiBitmap( delete_xpm ) );
            break;
        }
        break;

    case SCH_SHEET_T:
        AddMenusForHierchicalSheet( PopMenu, (SCH_SHEET*) item );
        break;

    case SCH_SHEET_PIN_T:
        AddMenusForSheetPin( PopMenu, (SCH_SHEET_PIN*) item );
        break;

    default:
        wxFAIL_MSG( wxString::Format( wxT( "Cannot create context menu for unknown type %d" ),
                                      item->Type() ) );
        break;
    }

    PopMenu->AppendSeparator();
    return true;
}


void AddMenusForComponentField( wxMenu* PopMenu, SCH_FIELD* Field )
{
    wxString msg, name;

    if( !Field->GetEditFlags() )
    {
        switch( Field->GetId() )
        {
        case REFERENCE: name = _( "Move Reference" ); break;
        case VALUE:     name = _( "Move Value" ); break;
        case FOOTPRINT: name = _( "Move Footprint Field" ); break;
        default:        name = _( "Move Field" ); break;
        }

        msg = AddHotkeyName( name, g_Schematic_Hotkeys_Descr, HK_MOVE_COMPONENT_OR_ITEM );
        AddMenuItem( PopMenu, ID_SCH_MOVE, msg, KiBitmap( move_xpm ) );
    }

    switch( Field->GetId() )
    {
    case REFERENCE: name = _( "Rotate Reference" ); break;
    case VALUE:     name = _( "Rotate Value" ); break;
    case FOOTPRINT: name = _( "Rotate Footprint Field" ); break;
    default:        name = _( "Rotate Field" ); break;
    }

    msg = AddHotkeyName( name, g_Schematic_Hotkeys_Descr, HK_ROTATE );
    AddMenuItem( PopMenu, ID_SCH_ROTATE_CLOCKWISE, msg, KiBitmap( rotate_cw_xpm ) );
}


void AddMenusForComponent( wxMenu* PopMenu, SCH_COMPONENT* Component, SYMBOL_LIB_TABLE* aLibs )
{
    if( Component->Type() != SCH_COMPONENT_T )
    {
        wxASSERT( 0 );
        return;
    }

    wxString       msg;

    if( !Component->GetEditFlags() )
    {
        msg.Printf( _( "Move %s" ), Component->GetField( REFERENCE )->GetText() );
        msg = AddHotkeyName( msg, g_Schematic_Hotkeys_Descr, HK_MOVE_COMPONENT_OR_ITEM );
        AddMenuItem( PopMenu, ID_SCH_MOVE, msg, KiBitmap( move_xpm ) );
        msg = AddHotkeyName( _( "Drag" ), g_Schematic_Hotkeys_Descr, HK_DRAG );
        AddMenuItem( PopMenu, ID_SCH_DRAG, msg, KiBitmap( drag_xpm ) );
    }

    wxMenu* orientmenu = new wxMenu;
    msg = AddHotkeyName( _( "Rotate Clockwise" ), g_Schematic_Hotkeys_Descr, HK_ROTATE );
    AddMenuItem( orientmenu, ID_SCH_ROTATE_CLOCKWISE, msg, KiBitmap( rotate_cw_xpm ) );
    AddMenuItem( orientmenu, ID_SCH_ROTATE_COUNTERCLOCKWISE, _( "Rotate Counterclockwise" ),
                 KiBitmap( rotate_ccw_xpm ) );
    msg = AddHotkeyName( _( "Mirror Around Horizontal(X) Axis" ), g_Schematic_Hotkeys_Descr,
                         HK_MIRROR_X );
    AddMenuItem( orientmenu, ID_SCH_MIRROR_X, msg, KiBitmap( mirror_v_xpm ) );
    msg = AddHotkeyName( _( "Mirror Around Vertical(Y) Axis" ), g_Schematic_Hotkeys_Descr,
                         HK_MIRROR_Y );
    AddMenuItem( orientmenu, ID_SCH_MIRROR_Y, msg, KiBitmap( mirror_h_xpm ) );
    msg = AddHotkeyName( _( "Reset to Default" ), g_Schematic_Hotkeys_Descr,
                         HK_ORIENT_NORMAL_COMPONENT );
    AddMenuItem( PopMenu, orientmenu, ID_POPUP_SCH_GENERIC_ORIENT_CMP,
                 _( "Orientation" ), KiBitmap( orient_xpm ) );

    AddMenusForEditComponent( PopMenu, Component, aLibs );

    if( !Component->GetEditFlags() )
    {
        msg = AddHotkeyName( _( "Duplicate" ), g_Schematic_Hotkeys_Descr,
                             HK_DUPLICATE );
        AddMenuItem( PopMenu, ID_SCH_DUPLICATE, msg, KiBitmap( duplicate_xpm ) );
        msg = AddHotkeyName( _( "Delete" ), g_Schematic_Hotkeys_Descr, HK_DELETE );
        AddMenuItem( PopMenu, ID_SCH_DELETE, msg, KiBitmap( delete_xpm ) );
    }

    msg = AddHotkeyName( _( "Autoplace Fields" ), g_Schematic_Hotkeys_Descr, HK_AUTOPLACE_FIELDS );
    AddMenuItem( PopMenu, ID_AUTOPLACE_FIELDS, msg, KiBitmap( autoplace_fields_xpm ) );

    if( !Component->GetField( DATASHEET )->GetFullyQualifiedText().IsEmpty() )
        AddMenuItem( PopMenu, ID_POPUP_SCH_DISPLAYDOC_CMP, _( "Open Documentation" ),
                     KiBitmap( datasheet_xpm ) );
}


void AddMenusForEditComponent( wxMenu* PopMenu, SCH_COMPONENT* Component, SYMBOL_LIB_TABLE* aLibs )
{
    if( Component->Type() != SCH_COMPONENT_T )
    {
        wxASSERT( 0 );
        return;
    }

    wxString    msg;
    LIB_PART*   part = NULL;
    LIB_ALIAS*  alias = NULL;

    try
    {
        alias = aLibs->LoadSymbol( Component->GetLibId() );
    }
    catch( ... )
    {
    }

    if( alias )
        part = alias->GetPart();

    wxMenu* editmenu = new wxMenu;

    if( part && part->HasConversion() )
        AddMenuItem( editmenu, ID_POPUP_SCH_EDIT_CONVERT_CMP, _( "Convert" ),
                     KiBitmap( component_select_alternate_shape_xpm ) );

    if( part && part->GetUnitCount() >= 2 )
    {
        wxMenu* sel_unit_menu = new wxMenu; int ii;

        for( ii = 0; ii < part->GetUnitCount(); ii++ )
        {
            wxString num_unit;
            int unit = Component->GetUnit();
            num_unit.Printf( _( "Unit %s" ), GetChars( LIB_PART::SubReference(  ii + 1, false ) ) );
            wxMenuItem * item = sel_unit_menu->Append( ID_POPUP_SCH_SELECT_UNIT1 + ii,
                                                       num_unit, wxEmptyString,
                                                       wxITEM_CHECK );
            if( unit == ii + 1 )
                item->Check(true);

            // The ID max for these submenus is ID_POPUP_SCH_SELECT_UNIT_CMP_MAX
            // See eeschema_id to modify this value.
            if( ii >= (ID_POPUP_SCH_SELECT_UNIT_CMP_MAX - ID_POPUP_SCH_SELECT_UNIT1) )
                break;      // We have used all IDs for these submenus
        }

        AddMenuItem( editmenu, sel_unit_menu, ID_POPUP_SCH_SELECT_UNIT_CMP,
                     _( "Unit" ), KiBitmap( component_select_unit_xpm ) );
    }

    if( !Component->GetEditFlags() )
    {
        msg = AddHotkeyName( _( "Edit with Library Editor" ), g_Schematic_Hotkeys_Descr,
                             HK_EDIT_COMPONENT_WITH_LIBEDIT );
        AddMenuItem( editmenu, ID_POPUP_SCH_CALL_LIBEDIT_AND_LOAD_CMP,
                     msg, KiBitmap( libedit_xpm ) );
    }
}


void AddMenusForGLabel( wxMenu* PopMenu, SCH_GLOBALLABEL* GLabel )
{
    wxMenu*  menu_change_type = new wxMenu;
    wxString msg;

    // add menu change type text (to label, glabel, text):
    AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_HLABEL,
                 _( "Change to Hierarchical Label" ), KiBitmap( label2glabel_xpm ) );
    AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_LABEL,
                 _( "Change to Label" ), KiBitmap( glabel2label_xpm ) );
    AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_COMMENT,
                 _( "Change to Text" ), KiBitmap( glabel2text_xpm ) );
    AddMenuItem( PopMenu, menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT,
                 _( "Change Type" ), KiBitmap( gl_change_xpm ) );
}


void AddMenusForHLabel( wxMenu* PopMenu, SCH_HIERLABEL* HLabel )
{
    wxMenu*  menu_change_type = new wxMenu;
    wxString msg;

    // add menu change type text (to label, glabel, text):
    AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_LABEL,
                 _( "Change to Label" ), KiBitmap( glabel2label_xpm ) );
    AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_COMMENT,
                 _( "Change to Text" ), KiBitmap( glabel2text_xpm ) );
    AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_GLABEL,
                 _( "Change to Global Label" ), KiBitmap( label2glabel_xpm ) );
    AddMenuItem( PopMenu, menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT,
                 _( "Change Type" ), KiBitmap( gl_change_xpm ) );
}


void AddMenusForLabel( wxMenu* PopMenu, SCH_LABEL* Label )
{
    wxMenu*  menu_change_type = new wxMenu;
    wxString msg;

    // add menu change type text (to label, glabel, text):
    AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_HLABEL,
                 _( "Change to Hierarchical Label" ), KiBitmap( label2glabel_xpm ) );
    AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_COMMENT,
                 _( "Change to Text" ), KiBitmap( label2text_xpm ) );
    AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_GLABEL,
                 _( "Change to Global Label" ), KiBitmap( label2glabel_xpm ) );
    AddMenuItem( PopMenu, menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT,
                 _( "Change Type" ), KiBitmap( gl_change_xpm ) );
}


void AddMenusForText( wxMenu* PopMenu, SCH_TEXT* Text )
{
    wxString msg;
    wxMenu*  menu_change_type = new wxMenu;

    /* add menu change type text (to label, glabel, text),
     * but only if this is a single line text
     */
    if( Text->GetText().Find( wxT( "\n" ) ) ==  wxNOT_FOUND )
    {
        AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_LABEL,
                     _( "Change to Label" ), KiBitmap( label2text_xpm ) );
        AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_HLABEL,
                     _( "Change to Hierarchical Label" ), KiBitmap( label2glabel_xpm ) );
        AddMenuItem( menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT_TO_GLABEL,
                     _( "Change to Global Label" ), KiBitmap( label2glabel_xpm ) );
        AddMenuItem( PopMenu, menu_change_type, ID_POPUP_SCH_CHANGE_TYPE_TEXT,
                     _( "Change Type" ), KiBitmap( gl_change_xpm ) );
    }
}


void SCH_EDIT_FRAME::addJunctionMenuEntries( wxMenu* aMenu, SCH_JUNCTION* aJunction )
{
    wxString msg;
    SCH_SCREEN* screen = GetScreen();

    msg = AddHotkeyName( _( "Delete Junction" ), g_Schematic_Hotkeys_Descr, HK_DELETE );
    AddMenuItem( aMenu, ID_SCH_DELETE, msg, KiBitmap( delete_xpm ) );

    if( !aJunction->IsNew() )
    {
        if( m_collectedItems.IsDraggableJunction() )
            AddMenuItem( aMenu, ID_SCH_DRAG, _( "Drag Junction" ), KiBitmap( drag_xpm ) );

        if( screen->GetWire( aJunction->GetPosition(), EXCLUDE_END_POINTS_T ) )
            AddMenuItem( aMenu, ID_POPUP_SCH_BREAK_WIRE, _( "Break Wire" ),
                         KiBitmap( break_line_xpm ) );
    }

    if( screen->GetWireOrBus( aJunction->GetPosition() ) )
    {
        AddMenuItem( aMenu, ID_POPUP_SCH_DELETE_NODE, _( "Delete Node" ),
                     KiBitmap( delete_node_xpm ) );
        AddMenuItem( aMenu, ID_POPUP_SCH_DELETE_CONNECTION, _( "Delete Connection" ),
                     KiBitmap( delete_connection_xpm ) );
    }
}


void AddMenusForWire( wxMenu* PopMenu, SCH_LINE* Wire, SCH_EDIT_FRAME* frame )
{
    SCH_SCREEN* screen = frame->GetScreen();
    wxPoint     pos    = frame->GetCrossHairPosition();
    wxString    msg;

    if( Wire == NULL )
    {
        msg = AddHotkeyName( _( "Begin Wire" ), g_Schematic_Hotkeys_Descr, HK_BEGIN_WIRE );
        AddMenuItem( PopMenu, ID_POPUP_SCH_BEGIN_WIRE, msg, KiBitmap( add_line_xpm ) );
        return;
    }

    msg = AddHotkeyName( _( "Drag Wire" ), g_Schematic_Hotkeys_Descr, HK_DRAG );
    AddMenuItem( PopMenu, ID_SCH_DRAG, msg, KiBitmap( drag_xpm ) );
    PopMenu->AppendSeparator();
    msg = AddHotkeyName( _( "Delete Wire" ), g_Schematic_Hotkeys_Descr, HK_DELETE );
    AddMenuItem( PopMenu, ID_SCH_DELETE, msg, KiBitmap( delete_xpm ) );
    AddMenuItem( PopMenu, ID_POPUP_SCH_DELETE_NODE, _( "Delete Node" ),
                 KiBitmap( delete_node_xpm ) );
    AddMenuItem( PopMenu, ID_POPUP_SCH_DELETE_CONNECTION, _( "Delete Connection" ),
                 KiBitmap( delete_connection_xpm ) );

    SCH_LINE* line = screen->GetWireOrBus( frame->GetCrossHairPosition() );

    if( line && !line->IsEndPoint( frame->GetCrossHairPosition() ) )
        AddMenuItem( PopMenu, ID_POPUP_SCH_BREAK_WIRE, _( "Break Wire" ),
                     KiBitmap( break_line_xpm ) );

    PopMenu->AppendSeparator();

    msg = AddHotkeyName( _( "Add Junction" ), g_Schematic_Hotkeys_Descr, HK_ADD_JUNCTION );
    AddMenuItem( PopMenu, ID_POPUP_SCH_ADD_JUNCTION, msg, KiBitmap( add_junction_xpm ) );
    msg = AddHotkeyName( _( "Add Label..." ), g_Schematic_Hotkeys_Descr, HK_ADD_LABEL );
    AddMenuItem( PopMenu, ID_POPUP_SCH_ADD_LABEL, msg, KiBitmap( add_line_label_xpm ) );

    // Add global label command only if the cursor is over one end of the wire.
    if( Wire->IsEndPoint( pos ) )
        AddMenuItem( PopMenu, ID_POPUP_SCH_ADD_GLABEL, _( "Add Global Label..." ),
                     KiBitmap( add_glabel_xpm ) );
}


void AddMenusForBus( wxMenu* PopMenu, SCH_LINE* Bus, SCH_EDIT_FRAME* frame )
{
    SCH_SELECTION_TOOL* selTool = frame->GetToolManager()->GetTool<SCH_SELECTION_TOOL>();
    wxPoint             pos = frame->GetCrossHairPosition();
    wxString            msg;

    if( Bus == NULL )
    {
        msg = AddHotkeyName( _( "Begin Bus" ), g_Schematic_Hotkeys_Descr, HK_BEGIN_BUS );
        AddMenuItem( PopMenu, ID_POPUP_SCH_BEGIN_BUS, msg, KiBitmap( add_bus_xpm ) );
        return;
    }

    msg = AddHotkeyName( _( "Delete Bus" ), g_Schematic_Hotkeys_Descr, HK_DELETE );
    AddMenuItem( PopMenu, ID_SCH_DELETE, msg, KiBitmap( delete_bus_xpm ) );

    AddMenuItem( PopMenu, ID_POPUP_SCH_BREAK_WIRE, _( "Break Bus" ), KiBitmap( break_bus_xpm ) );

    // TODO(JE) remove once real-time is enabled
    if( !ADVANCED_CFG::GetCfg().m_realTimeConnectivity )
    {
        frame->RecalculateConnections();

        // Have to pick up the pointer again because it may have been changed by SchematicCleanUp
        bool actionCancelled = false;
        Bus = dynamic_cast<SCH_LINE*>( selTool->SelectPoint( pos, SCH_COLLECTOR::AllItemsButPins,
                                                             &actionCancelled ) );
        wxASSERT( Bus );
    }

    // Bus unfolding menu (only available if bus is properly defined)
    auto connection = Bus->Connection( *g_CurrentSheet );

    if( connection && connection->IsBus() && connection->Members().size() > 0 )
    {
        int idx = 0;
        wxMenu* bus_unfolding_menu = new wxMenu;

        PopMenu->AppendSubMenu( bus_unfolding_menu, _( "Unfold Bus" ) );

        for( const auto& member : connection->Members() )
        {
            int id = ID_POPUP_SCH_UNFOLD_BUS + ( idx++ );
            auto name = member->Name( true );

            if( member->Type() == CONNECTION_BUS )
            {
                wxMenu* submenu = new wxMenu;
                bus_unfolding_menu->AppendSubMenu( submenu, _( name ) );

                for( const auto& sub_member : member->Members() )
                {
                    id = ID_POPUP_SCH_UNFOLD_BUS + ( idx++ );

                    submenu->Append( id, sub_member->Name( true ), wxEmptyString );

                    // See comment in else clause below
                    auto sub_item_clone = new wxMenuItem();
                    sub_item_clone->SetItemLabel( sub_member->Name( true ) );

                    frame->Bind( wxEVT_COMMAND_MENU_SELECTED, &SCH_EDIT_FRAME::OnUnfoldBus,
                                 frame, id, id, sub_item_clone );
                }
            }
            else
            {
                bus_unfolding_menu->Append( id, name, wxEmptyString );

                // Because Bind() takes ownership of the user data item, we
                // make a new menu item here and set its label.  Why create a
                // menu item instead of just a wxString or something? Because
                // Bind() requires a pointer to wxObject rather than a void
                // pointer.  Maybe at some point I'll think of a better way...
                auto item_clone = new wxMenuItem();
                item_clone->SetItemLabel( name );

                frame->Bind( wxEVT_COMMAND_MENU_SELECTED, &SCH_EDIT_FRAME::OnUnfoldBus,
                             frame, id, id, item_clone );
            }
        }
    }

    PopMenu->AppendSeparator();
    msg = AddHotkeyName( _( "Add Junction" ), g_Schematic_Hotkeys_Descr, HK_ADD_JUNCTION );
    AddMenuItem( PopMenu, ID_POPUP_SCH_ADD_JUNCTION, msg, KiBitmap( add_junction_xpm ) );
    msg = AddHotkeyName( _( "Add Label..." ), g_Schematic_Hotkeys_Descr, HK_ADD_LABEL );
    AddMenuItem( PopMenu, ID_POPUP_SCH_ADD_LABEL, msg, KiBitmap( add_line_label_xpm ) );

    // Add global label command only if the cursor is over one end of the bus.
    if( Bus->IsEndPoint( pos ) )
        AddMenuItem( PopMenu, ID_POPUP_SCH_ADD_GLABEL, _( "Add Global Label..." ),
                     KiBitmap( add_glabel_xpm ) );
}


void AddMenusForHierchicalSheet( wxMenu* PopMenu, SCH_SHEET* Sheet )
{
    wxString msg;

    if( !Sheet->GetEditFlags() )
    {
        AddMenuItem( PopMenu, ID_POPUP_SCH_ENTER_SHEET, _( "Enter Sheet" ),
                     KiBitmap( enter_sheet_xpm ) );
        PopMenu->AppendSeparator();
        msg = AddHotkeyName( _( "Move" ), g_Schematic_Hotkeys_Descr,
                             HK_MOVE_COMPONENT_OR_ITEM );
        AddMenuItem( PopMenu, ID_SCH_MOVE, msg, KiBitmap( move_xpm ) );

        msg = AddHotkeyName( _( "Drag" ), g_Schematic_Hotkeys_Descr, HK_DRAG );
        AddMenuItem( PopMenu, ID_SCH_DRAG, msg, KiBitmap( drag_xpm ) );

        PopMenu->AppendSeparator();
        msg = AddHotkeyName( _( "Select Items On PCB" ), g_Schematic_Hotkeys_Descr,
                             HK_SELECT_ITEMS_ON_PCB );
        AddMenuItem( PopMenu, ID_POPUP_SCH_SELECT_ON_PCB, msg, KiBitmap( select_same_sheet_xpm ) );
        PopMenu->AppendSeparator();

        wxMenu* orientmenu = new wxMenu;
        msg = AddHotkeyName( _( "Rotate Clockwise" ), g_Schematic_Hotkeys_Descr, HK_ROTATE );
        AddMenuItem( orientmenu, ID_SCH_ROTATE_CLOCKWISE, msg, KiBitmap( rotate_cw_xpm ) );

        AddMenuItem( orientmenu, ID_SCH_ROTATE_COUNTERCLOCKWISE, _( "Rotate Counterclockwise" ),
                     KiBitmap( rotate_ccw_xpm ) );

        msg = AddHotkeyName( _( "Mirror Around Horizontal(X) Axis" ), g_Schematic_Hotkeys_Descr,
                             HK_MIRROR_X );
        AddMenuItem( orientmenu, ID_SCH_MIRROR_X, msg, KiBitmap( mirror_v_xpm ) );
        msg = AddHotkeyName( _( "Mirror Around Vertical(Y) Axis" ), g_Schematic_Hotkeys_Descr,
                             HK_MIRROR_Y );
        AddMenuItem( orientmenu, ID_SCH_MIRROR_Y, msg, KiBitmap( mirror_h_xpm ) );

        AddMenuItem( PopMenu, orientmenu, ID_POPUP_SCH_GENERIC_ORIENT_CMP,
                 _( "Orientation" ), KiBitmap( orient_xpm ) );
    }

    if( Sheet->GetEditFlags() )
    {
        AddMenuItem( PopMenu, ID_POPUP_SCH_END_SHEET, _( "Place" ), KiBitmap( checked_ok_xpm ) );
    }
    else
    {
        msg = AddHotkeyName( _( "Edit..." ), g_Schematic_Hotkeys_Descr, HK_EDIT );
        AddMenuItem( PopMenu, ID_SCH_EDIT_ITEM, msg, KiBitmap( editor_xpm ) );

        AddMenuItem( PopMenu, ID_POPUP_SCH_RESIZE_SHEET, _( "Resize" ),
                     KiBitmap( resize_sheet_xpm ) );
        PopMenu->AppendSeparator();
        AddMenuItem( PopMenu, ID_POPUP_IMPORT_HLABEL_TO_SHEETPIN, _( "Import Sheet Pins" ),
                     KiBitmap( import_hierarchical_label_xpm ) );

        if( Sheet->HasUndefinedPins() )  // Sheet has pin labels, and can be cleaned
            AddMenuItem( PopMenu, ID_POPUP_SCH_CLEANUP_SHEET, _( "Cleanup Sheet Pins" ),
                         KiBitmap( options_pinsheet_xpm ) );

        PopMenu->AppendSeparator();
        msg = AddHotkeyName( _( "Delete" ), g_Schematic_Hotkeys_Descr, HK_DELETE );
        AddMenuItem( PopMenu, ID_SCH_DELETE, msg, KiBitmap( delete_sheet_xpm ) );
    }
}


void AddMenusForSheetPin( wxMenu* PopMenu, SCH_SHEET_PIN* PinSheet )
{
    wxString msg;

    if( !PinSheet->GetEditFlags() )
    {
        msg = AddHotkeyName( _( "Move" ), g_Schematic_Hotkeys_Descr, HK_MOVE_COMPONENT_OR_ITEM );
        AddMenuItem( PopMenu, ID_SCH_MOVE, msg, KiBitmap( move_xpm ) );
    }

    AddMenuItem( PopMenu, ID_SCH_EDIT_ITEM, _( "Edit..." ), KiBitmap( edit_xpm ) );

    if( !PinSheet->GetEditFlags() )
        AddMenuItem( PopMenu, ID_SCH_DELETE, _( "Delete" ), KiBitmap( delete_xpm ) );
}


void AddMenusForMarkers( wxMenu* aPopMenu, SCH_MARKER* aMarker, SCH_EDIT_FRAME* aFrame )
{
    AddMenuItem( aPopMenu, ID_SCH_DELETE, _( "Delete Marker" ), KiBitmap( delete_xpm ) );
    AddMenuItem( aPopMenu, ID_POPUP_SCH_GETINFO_MARKER, _( "Marker Error Info" ),
                 KiBitmap( info_xpm ) );
}


void AddMenusForBitmap( wxMenu* aPopMenu, SCH_BITMAP * aBitmap )
{
    wxString msg;

    if( aBitmap->GetEditFlags() == 0 )
    {
        msg = AddHotkeyName( _( "Move" ), g_Schematic_Hotkeys_Descr,
                             HK_MOVE_COMPONENT_OR_ITEM );
        AddMenuItem( aPopMenu, ID_SCH_MOVE, msg, KiBitmap( move_xpm ) );
    }

    msg = AddHotkeyName( _( "Rotate Counterclockwise" ), g_Schematic_Hotkeys_Descr, HK_ROTATE );
    AddMenuItem( aPopMenu, ID_SCH_ROTATE_CLOCKWISE, msg, KiBitmap( rotate_ccw_xpm ) );
    msg = AddHotkeyName( _( "Mirror Around Horizontal(X) Axis" ), g_Schematic_Hotkeys_Descr,
                         HK_MIRROR_X );
    AddMenuItem( aPopMenu, ID_SCH_MIRROR_X, msg, KiBitmap( mirror_v_xpm ) );
    msg = AddHotkeyName( _( "Mirror Around Vertical(Y) Axis" ), g_Schematic_Hotkeys_Descr,
                         HK_MIRROR_Y );
    AddMenuItem( aPopMenu, ID_SCH_MIRROR_Y, msg, KiBitmap( mirror_h_xpm ) );
    msg = AddHotkeyName( _( "Edit Image..." ), g_Schematic_Hotkeys_Descr, HK_EDIT );
    AddMenuItem( aPopMenu, ID_SCH_EDIT_ITEM, msg, KiBitmap( image_xpm ) );

    if( aBitmap->GetEditFlags() == 0 )
    {
        aPopMenu->AppendSeparator();
        msg = AddHotkeyName( _( "Delete" ), g_Schematic_Hotkeys_Descr, HK_DELETE );
        AddMenuItem( aPopMenu, ID_SCH_DELETE, msg, KiBitmap( delete_xpm ) );
    }
}


void AddMenusForBusEntry( wxMenu* aPopMenu, SCH_BUS_ENTRY_BASE* aBusEntry )
{
    wxString msg;

    if( !aBusEntry->GetEditFlags() )
    {
        msg = AddHotkeyName( _( "Move Bus Entry" ), g_Schematic_Hotkeys_Descr,
                                      HK_MOVE_COMPONENT_OR_ITEM );
        AddMenuItem( aPopMenu, ID_SCH_MOVE, msg, KiBitmap( move_xpm ) );
    }

    if( aBusEntry->GetBusEntryShape() == '\\' )
        AddMenuItem( aPopMenu, ID_POPUP_SCH_ENTRY_SELECT_SLASH,
                     _( "Set Bus Entry Shape /" ), KiBitmap( change_entry_orient_xpm ) );
    else
        AddMenuItem( aPopMenu, ID_POPUP_SCH_ENTRY_SELECT_ANTISLASH,
                     _( "Set Bus Entry Shape \\" ), KiBitmap( change_entry_orient_xpm ) );

    msg = AddHotkeyName( _( "Delete Bus Entry" ), g_Schematic_Hotkeys_Descr, HK_DELETE );
    AddMenuItem( aPopMenu, ID_SCH_DELETE, msg, KiBitmap( delete_xpm ) );
}
