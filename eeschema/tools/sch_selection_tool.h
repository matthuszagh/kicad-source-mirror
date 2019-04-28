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

#ifndef KICAD_SCH_SELECTION_TOOL_H
#define KICAD_SCH_SELECTION_TOOL_H

#include <tool/tool_interactive.h>
#include <tool/context_menu.h>
#include <tool/selection.h>
#include <tool/tool_menu.h>
#include <sch_collectors.h>

class SCH_BASE_FRAME;
class SCH_ITEM;
class SCH_COLLECTOR;

namespace KIGFX
{
    class GAL;
}


class SCH_SELECTION_TOOL : public TOOL_INTERACTIVE
{
public:
    SCH_SELECTION_TOOL();
    ~SCH_SELECTION_TOOL();

    /// @copydoc TOOL_BASE::Init()
    bool Init() override;

    /// @copydoc TOOL_BASE::Reset()
    void Reset( RESET_REASON aReason ) override;

    /**
     * Function Main()
     *
     * The main loop.
     */
    int Main( const TOOL_EVENT& aEvent );

    /**
     * Function GetSelection()
     *
     * Returns the set of currently selected items.
     */
    SELECTION& GetSelection();

    /**
     * Function RequestSelection()
     *
     * Returns either an existing selection (filtered), or the selection at the current
     * cursor if the existing selection is empty.
     */
    SELECTION& RequestSelection( const KICAD_T* aFilterList = SCH_COLLECTOR::AllItems );

    /**
     * Function selectPoint()
     * Selects an item pointed by the parameter aWhere. If there is more than one item at that
     * place, there is a menu displayed that allows one to choose the item.
     *
     * @param aWhere is the place where the item should be selected.
     * @param aSelectionCancelledFlag allows the function to inform its caller that a selection
     * was cancelled (for instance, by clicking outside of the disambiguation menu).
     * @param aCheckLocked indicates if locked items should be excluded
     */
    SCH_ITEM* SelectPoint( const VECTOR2I& aWhere,
                           const KICAD_T* aFilterList = SCH_COLLECTOR::AllItems,
                           bool* aSelectionCancelledFlag = NULL, bool aCheckLocked = false );

    ///> Item selection event handler.
    int SelectItem( const TOOL_EVENT& aEvent );

    ///> Multiple item selection event handler
    int SelectItems( const TOOL_EVENT& aEvent );

    ///> Item unselection event handler.
    int UnselectItem( const TOOL_EVENT& aEvent );

    ///> Multiple item unselection event handler
    int UnselectItems( const TOOL_EVENT& aEvent );

    ///> Clear current selection event handler.
    int ClearSelection( const TOOL_EVENT& aEvent );

    /**
     * Function SelectionMenu()
     * Shows a popup menu to trim the COLLECTOR passed as aEvent's parameter down to a single
     * item.
     *
     * NOTE: this routine DOES NOT modify the selection.
     */
    int SelectionMenu( const TOOL_EVENT& aEvent );

private:
    /**
     * Function selectCursor()
     * Selects an item under the cursor unless there is something already selected or aForceSelect
     * is true.
     * @param aForceSelect forces to select an item even if there is an item already selected.
     * @param aClientFilter allows the client to perform tool- or action-specific filtering.
     * @return true if eventually there is an item selected, false otherwise.
     */
    bool selectCursor( const KICAD_T aFilterList[], bool aForceSelect = false );

    /**
     * Function selectMultiple()
     * Handles drawing a selection box that allows one to select many items at
     * the same time.
     *
     * @return true if the function was cancelled (i.e. CancelEvent was received).
     */
    bool selectMultiple();

    /**
     * Apply heuristics to try and determine a single object when multiple are found under the
     * cursor.
     */
    void guessSelectionCandidates( SCH_COLLECTOR& collector, const VECTOR2I& aWhere );

    /**
     * Allows the selection of a single item from a list via pop-up menu.  The items are
     * highlighted on the canvas when hovered in the menu.  The collector is trimmed to
     * the picked item.
     * @return true if an item was picked
     */
    bool doSelectionMenu( SCH_COLLECTOR* aItems );

    /**
     * Function clearSelection()
     * Clears the current selection.
     */
    void clearSelection();

    /**
     * Function toggleSelection()
     * Changes selection status of a given item.
     *
     * @param aItem is the item to have selection status changed.
     * @param aForce causes the toggle to happen without checking selectability
     */
    void toggleSelection( SCH_ITEM* aItem, bool aForce = false );

    /**
     * Function selectable()
     * Checks conditions for an item to be selected.
     *
     * @return True if the item fulfills conditions to be selected.
     */
    bool selectable( const SCH_ITEM* aItem, bool checkVisibilityOnly = false ) const;

    /**
     * Function select()
     * Takes necessary action mark an item as selected.
     *
     * @param aItem is an item to be selected.
     */
    void select( SCH_ITEM* aItem );

    /**
     * Function unselect()
     * Takes necessary action mark an item as unselected.
     *
     * @param aItem is an item to be unselected.
     */
    void unselect( SCH_ITEM* aItem );

    /**
     * Function highlight()
     * Highlights the item visually.
     * @param aItem is an item to be be highlighted.
     * @param aHighlightMode should be either SELECTED or BRIGHTENED
     * @param aGroup is the group to add the item to in the BRIGHTENED mode.
     */
    void highlight( SCH_ITEM* aItem, int aHighlightMode, SELECTION* aGroup = nullptr );

    /**
     * Function unhighlight()
     * Unhighlights the item visually.
     * @param aItem is an item to be be highlighted.
     * @param aHighlightMode should be either SELECTED or BRIGHTENED
     * @param aGroup is the group to remove the item from.
     */
    void unhighlight( SCH_ITEM* aItem, int aHighlightMode, SELECTION* aGroup = nullptr );

    /**
     * Function selectionContains()
     * Checks if the given point is placed within any of selected items' bounding box.
     *
     * @return True if the given point is contained in any of selected items' bouding box.
     */
    bool selectionContains( const VECTOR2I& aPoint ) const;

    ///> Sets up handlers for various events.
    void setTransitions() override;

private:
    SCH_BASE_FRAME* m_frame;    // Pointer to the parent frame
    SELECTION m_selection;      // Current state of selection

    bool m_additive;            // Items should be added to selection (instead of replacing)
    bool m_subtractive;         // Items should be removed from selection
    bool m_multiple;            // Multiple selection mode is active
    bool m_skip_heuristics;     // Heuristics are not allowed when choosing item under cursor

    TOOL_MENU m_menu;
};

#endif //KICAD_SCH_SELECTION_TOOL_H
