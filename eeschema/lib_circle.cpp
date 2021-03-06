/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2017 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2004-2019 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file lib_circle.cpp
 * @brief LIB_CIRCLE class implementation.
 */

#include <fctsys.h>
#include <gr_basic.h>
#include <macros.h>
#include <sch_draw_panel.h>
#include <plotter.h>
#include <trigo.h>
#include <base_units.h>
#include <msgpanel.h>
#include <bitmaps.h>

#include <general.h>
#include <lib_circle.h>
#include <transform.h>


LIB_CIRCLE::LIB_CIRCLE( LIB_PART*      aParent ) :
    LIB_ITEM( LIB_CIRCLE_T, aParent )
{
    m_Radius     = 0;
    m_Width      = 0;
    m_Fill       = NO_FILL;
    m_isFillable = true;
}


bool LIB_CIRCLE::HitTest( const wxPoint& aPosRef ) const
{
    int mindist = GetPenSize() / 2;

    // Have a minimal tolerance for hit test
    if( mindist < MINIMUM_SELECTION_DISTANCE )
        mindist = MINIMUM_SELECTION_DISTANCE;

    return HitTest( aPosRef, mindist, DefaultTransform );
}


bool LIB_CIRCLE::HitTest( const wxPoint &aPosRef, int aThreshold, const TRANSFORM& aTransform ) const
{
    if( aThreshold < 0 )
        aThreshold = GetPenSize() / 2;

    int dist = KiROUND( GetLineLength( aPosRef, aTransform.TransformCoordinate( m_Pos ) ) );

    if( abs( dist - m_Radius ) <= aThreshold )
        return true;
    return false;
}


EDA_ITEM* LIB_CIRCLE::Clone() const
{
    return new LIB_CIRCLE( *this );
}


int LIB_CIRCLE::compare( const LIB_ITEM& aOther ) const
{
    wxASSERT( aOther.Type() == LIB_CIRCLE_T );

    const LIB_CIRCLE* tmp = ( LIB_CIRCLE* ) &aOther;

    if( m_Pos.x != tmp->m_Pos.x )
        return m_Pos.x - tmp->m_Pos.x;

    if( m_Pos.y != tmp->m_Pos.y )
        return m_Pos.y - tmp->m_Pos.y;

    if( m_Radius != tmp->m_Radius )
        return m_Radius - tmp->m_Radius;

    return 0;
}


void LIB_CIRCLE::SetOffset( const wxPoint& aOffset )
{
    m_Pos += aOffset;
}


bool LIB_CIRCLE::Inside( EDA_RECT& aRect ) const
{
    wxPoint center(m_Pos.x, -m_Pos.y);
    return aRect.IntersectsCircle( center, m_Radius );
}


void LIB_CIRCLE::Move( const wxPoint& aPosition )
{
    m_Pos = aPosition;
}


void LIB_CIRCLE::MirrorHorizontal( const wxPoint& aCenter )
{
    m_Pos.x -= aCenter.x;
    m_Pos.x *= -1;
    m_Pos.x += aCenter.x;
}


void LIB_CIRCLE::MirrorVertical( const wxPoint& aCenter )
{
    m_Pos.y -= aCenter.y;
    m_Pos.y *= -1;
    m_Pos.y += aCenter.y;
}


void LIB_CIRCLE::Rotate( const wxPoint& aCenter, bool aRotateCCW )
{
    int rot_angle = aRotateCCW ? -900 : 900;

    RotatePoint( &m_Pos, aCenter, rot_angle );
}


void LIB_CIRCLE::Plot( PLOTTER* aPlotter, const wxPoint& aOffset, bool aFill,
                       const TRANSFORM& aTransform )
{
    wxPoint pos = aTransform.TransformCoordinate( m_Pos ) + aOffset;

    if( aFill && m_Fill == FILLED_WITH_BG_BODYCOLOR )
    {
        aPlotter->SetColor( GetLayerColor( LAYER_DEVICE_BACKGROUND ) );
        aPlotter->Circle( pos, m_Radius * 2, FILLED_WITH_BG_BODYCOLOR, 0 );
    }

    bool already_filled = m_Fill == FILLED_WITH_BG_BODYCOLOR;
    auto pen_size = GetPenSize();

    if( !already_filled || pen_size > 0 )
    {
        pen_size = std::max( 0, pen_size );
        aPlotter->SetColor( GetLayerColor( LAYER_DEVICE ) );
        aPlotter->Circle( pos, m_Radius * 2, already_filled ? NO_FILL : m_Fill, GetPenSize() );
    }
}


int LIB_CIRCLE::GetPenSize() const
{
    if( m_Width > 0 )
        return m_Width;

    if( m_Width == 0 )
       return GetDefaultLineThickness();

    return -1;   // a value to use a minimal pen size
}


void LIB_CIRCLE::drawGraphic( EDA_DRAW_PANEL* aPanel, wxDC* aDC, const wxPoint& aOffset,
                              void* aData, const TRANSFORM& aTransform )
{
    wxPoint pos1    = aTransform.TransformCoordinate( m_Pos ) + aOffset;
    COLOR4D color   = GetLayerColor( LAYER_DEVICE );
    COLOR4D bgColor = GetLayerColor( LAYER_DEVICE_BACKGROUND );
    FILL_T  fill    = aData ? NO_FILL : m_Fill;

    EDA_RECT* const clipbox  = aPanel? aPanel->GetClipBox() : NULL;

    if( fill == FILLED_WITH_BG_BODYCOLOR )
        GRFilledCircle( clipbox, aDC, pos1.x, pos1.y, m_Radius, GetPenSize(), bgColor, bgColor );
    else if( fill == FILLED_SHAPE )
        GRFilledCircle( clipbox, aDC, pos1.x, pos1.y, m_Radius, 0, color, color );
    else
        GRCircle( clipbox, aDC, pos1.x, pos1.y, m_Radius, GetPenSize(), color );
}


const EDA_RECT LIB_CIRCLE::GetBoundingBox() const
{
    EDA_RECT rect;

    rect.SetOrigin( m_Pos.x - m_Radius, m_Pos.y - m_Radius );
    rect.SetEnd( m_Pos.x + m_Radius, m_Pos.y + m_Radius );
    rect.Inflate( ( GetPenSize()+1 ) / 2 );

    rect.RevertYAxis();

    return rect;
}


void LIB_CIRCLE::GetMsgPanelInfo( EDA_UNITS_T aUnits, MSG_PANEL_ITEMS& aList )
{
    wxString msg;
    EDA_RECT bBox = GetBoundingBox();

    LIB_ITEM::GetMsgPanelInfo( aUnits, aList );

    msg = MessageTextFromValue( aUnits, m_Width, true );

    aList.push_back( MSG_PANEL_ITEM(  _( "Line Width" ), msg, BLUE ) );

    msg = MessageTextFromValue( aUnits, m_Radius, true );
    aList.push_back( MSG_PANEL_ITEM( _( "Radius" ), msg, RED ) );

    msg.Printf( wxT( "(%d, %d, %d, %d)" ), bBox.GetOrigin().x,
                bBox.GetOrigin().y, bBox.GetEnd().x, bBox.GetEnd().y );

    aList.push_back( MSG_PANEL_ITEM( _( "Bounding Box" ), msg, BROWN ) );
}


wxString LIB_CIRCLE::GetSelectMenuText( EDA_UNITS_T aUnits ) const
{
    return wxString::Format( _( "Circle center (%s, %s), radius %s" ),
                             MessageTextFromValue( aUnits, m_Pos.x ),
                             MessageTextFromValue( aUnits, m_Pos.y ),
                             MessageTextFromValue( aUnits, m_Radius ) );
}


BITMAP_DEF LIB_CIRCLE::GetMenuImage() const
{
    return add_circle_xpm;
}


void LIB_CIRCLE::BeginEdit( STATUS_FLAGS aEditMode, const wxPoint aPosition )
{
    wxCHECK_RET( ( aEditMode & ( IS_NEW | IS_MOVED | IS_RESIZED ) ) != 0,
                 wxT( "Invalid edit mode for LIB_CIRCLE object." ) );

    if( aEditMode == IS_NEW )
    {
        m_Pos = m_initialPos = aPosition;
    }
    else if( aEditMode == IS_MOVED )
    {
        m_initialPos = m_Pos;
        m_initialCursorPos = aPosition;
    }

    m_Flags = aEditMode;
}


bool LIB_CIRCLE::ContinueEdit( const wxPoint aPosition )
{
    wxCHECK_MSG( ( m_Flags & ( IS_NEW | IS_MOVED | IS_RESIZED ) ) != 0, false,
                   wxT( "Bad call to ContinueEdit().  LIB_CIRCLE is not being edited." ) );

    return false;
}


void LIB_CIRCLE::EndEdit( const wxPoint& aPosition, bool aAbort )
{
    wxCHECK_RET( ( m_Flags & ( IS_NEW | IS_MOVED | IS_RESIZED ) ) != 0,
                   wxT( "Bad call to EndEdit().  LIB_CIRCLE is not being edited." ) );

    m_Flags = 0;
}


void LIB_CIRCLE::CalcEdit( const wxPoint& aPosition )
{
    if( m_Flags == IS_NEW || m_Flags == IS_RESIZED )
    {
        m_Radius = KiROUND( GetLineLength( m_Pos, aPosition ) );
    }
    else
    {
        Move( m_initialPos + aPosition - m_initialCursorPos );
    }
}
