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

#ifndef KICAD_SCH_EDIT_TOOL_H
#define KICAD_SCH_EDIT_TOOL_H

#include <tool/tool_interactive.h>
#include <tool/tool_menu.h>
#include <sch_base_frame.h>


class SCH_EDIT_FRAME;
class SCH_SELECTION_TOOL;


class SCH_EDIT_TOOL : public TOOL_INTERACTIVE
{
public:
    SCH_EDIT_TOOL();
    ~SCH_EDIT_TOOL();

    /// @copydoc TOOL_INTERACTIVE::Init()
    bool Init() override;

    /// @copydoc TOOL_INTERACTIVE::Reset()
    void Reset( RESET_REASON aReason ) override;

    ///> Get the SCH_DRAWING_TOOL top-level context menu
    inline TOOL_MENU& GetToolMenu()
    {
        return m_menu;
    }

    /**
     * Function Main()
     *
     * Runs an interactive move of the selected items, or the item under the cursor.
     */
    int Main( const TOOL_EVENT& aEvent );

    int Rotate( const TOOL_EVENT& aEvent );
    int Mirror( const TOOL_EVENT& aEvent );

    int Duplicate( const TOOL_EVENT& aEvent );
    int RepeatDrawItem( const TOOL_EVENT& aEvent );

    /**
     * Function Remove()
     *
     * Deletes the selected items, or the item under the cursor.
     */
    int Remove( const TOOL_EVENT& aEvent );

private:
    void moveItem( SCH_ITEM* aItem, VECTOR2I aDelta, bool isDrag );

    ///> Selects additional items for a drag operation.
    ///> Connected items with no wire are selected (as there is no wire to adjust for the drag).
    ///> Connected wires are selected with any un-connected ends flagged (STARTPOINT or ENDPOINT).
    void selectConnectedDragItems( SCH_ITEM* aSourceItem, wxPoint aPoint );

    ///> Returns the right modification point (e.g. for rotation), depending on the number of
    ///> selected items.
    bool updateModificationPoint( SELECTION& aSelection );

    ///> Similar to getView()->Update(), but handles items that are redrawn by their parents.
    void updateView( EDA_ITEM* );

    ///> Similar to m_frame->SaveCopyInUndoList(), but handles items that are owned by their
    ///> parents.
    void saveCopyInUndoList( SCH_ITEM*, UNDO_REDO_T aType, bool aAppend = false );

    ///> Sets up handlers for various events.
    void setTransitions() override;

private:
    SCH_SELECTION_TOOL*   m_selectionTool;
    KIGFX::SCH_VIEW*      m_view;
    KIGFX::VIEW_CONTROLS* m_controls;
    SCH_EDIT_FRAME*       m_frame;

    /// Menu model displayed by the tool.
    TOOL_MENU             m_menu;

    ///> Flag determining if anything is being dragged right now
    bool                  m_moveInProgress;

    ///> Last cursor position (needed for getModificationPoint() to avoid changes
    ///> of edit reference point).
    VECTOR2I              m_cursor;
};

#endif //KICAD_SCH_EDIT_TOOL_H
