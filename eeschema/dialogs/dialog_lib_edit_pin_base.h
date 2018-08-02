///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Aug  2 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __DIALOG_LIB_EDIT_PIN_BASE_H__
#define __DIALOG_LIB_EDIT_PIN_BASE_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
class PinShapeComboBox;
class PinTypeComboBox;
class wxBitmapComboBox;

#include "dialog_shim.h"
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/combobox.h>
#include <wx/gbsizer.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/panel.h>
#include <wx/statline.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class DIALOG_LIB_EDIT_PIN_BASE
///////////////////////////////////////////////////////////////////////////////
class DIALOG_LIB_EDIT_PIN_BASE : public DIALOG_SHIM
{
	private:
	
	protected:
		enum
		{
			ID_M_TEXTPINNAME = 1000,
			ID_M_STATICTEXTPADNAME,
			ID_M_TEXTPADNAME,
			ID_M_STATICTEXTPINLEN,
			ID_M_TEXTLENGTH,
			ID_M_STATICLENGTHUNITS,
			ID_M_STATICTEXTNAMESIZE,
			ID_M_TEXTPINNAMETEXTSIZE,
			ID_M_STATICNAMETEXTSIZEUNITS,
			ID_M_STATICTEXTPADNAMESIZE,
			ID_M_TEXTPADNAMETEXTSIZE,
			ID_M_STATICNUMBERTEXTSIZEUNITS
		};
		
		wxStaticText* m_pinNameLabel;
		wxTextCtrl* m_textPinName;
		wxStaticText* m_pinNumberLabel;
		wxTextCtrl* m_textPinNumber;
		wxStaticText* m_staticTextEType;
		PinTypeComboBox* m_choiceElectricalType;
		wxStaticText* m_staticTextGstyle;
		PinShapeComboBox* m_choiceStyle;
		wxStaticText* m_posXLabel;
		wxTextCtrl* m_posXCtrl;
		wxStaticText* m_posXUnits;
		wxStaticText* m_posYLabel;
		wxTextCtrl* m_posYCtrl;
		wxStaticText* m_posYUnits;
		wxStaticText* m_staticTextOrient;
		wxBitmapComboBox* m_choiceOrientation;
		wxStaticText* m_pinLengthLabel;
		wxTextCtrl* m_pinLengthCtrl;
		wxStaticText* m_pinLengthUnits;
		wxStaticText* m_nameSizeLabel;
		wxTextCtrl* m_nameSizeCtrl;
		wxStaticText* m_nameSizeUnits;
		wxStaticText* m_numberSizeLabel;
		wxTextCtrl* m_numberSizeCtrl;
		wxStaticText* m_numberSizeUnits;
		wxCheckBox* m_checkApplyToAllParts;
		wxCheckBox* m_checkApplyToAllConversions;
		wxCheckBox* m_checkShow;
		wxPanel* m_panelShowPin;
		wxStaticLine* m_staticline1;
		wxStdDialogButtonSizer* m_sdbSizerButtons;
		wxButton* m_sdbSizerButtonsOK;
		wxButton* m_sdbSizerButtonsCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnPropertiesChange( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnPaintShowPanel( wxPaintEvent& event ) { event.Skip(); }
		
	
	public:
		
		DIALOG_LIB_EDIT_PIN_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Pin Properties"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER ); 
		~DIALOG_LIB_EDIT_PIN_BASE();
	
};

#endif //__DIALOG_LIB_EDIT_PIN_BASE_H__
