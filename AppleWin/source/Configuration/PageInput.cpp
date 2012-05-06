#include "StdAfx.h"
#include "PageInput.h"
#include "PropertySheetHelper.h"
#include "..\resource\resource.h"

CPageInput* CPageInput::ms_this = 0;	// reinit'd in ctor

const TCHAR CPageInput::m_szJoyChoice0[] = TEXT("Disabled\0");
const TCHAR CPageInput::m_szJoyChoice1[] = TEXT("PC Joystick #1\0");
const TCHAR CPageInput::m_szJoyChoice2[] = TEXT("PC Joystick #2\0");
const TCHAR CPageInput::m_szJoyChoice3[] = TEXT("Keyboard (standard)\0");
const TCHAR CPageInput::m_szJoyChoice4[] = TEXT("Keyboard (centering)\0");
const TCHAR CPageInput::m_szJoyChoice5[] = TEXT("Mouse\0");

const TCHAR* const CPageInput::m_pszJoy0Choices[J0C_MAX] = {
									CPageInput::m_szJoyChoice0,
									CPageInput::m_szJoyChoice1,	// PC Joystick #1
									CPageInput::m_szJoyChoice3,
									CPageInput::m_szJoyChoice4,
									CPageInput::m_szJoyChoice5 };

const TCHAR* const CPageInput::m_pszJoy1Choices[J1C_MAX] = {
									CPageInput::m_szJoyChoice0,
									CPageInput::m_szJoyChoice2,	// PC Joystick #2
									CPageInput::m_szJoyChoice3,
									CPageInput::m_szJoyChoice4,
									CPageInput::m_szJoyChoice5 };

const TCHAR CPageInput::m_szCPMSlotChoice_Slot4[] = TEXT("Slot 4\0");
const TCHAR CPageInput::m_szCPMSlotChoice_Slot5[] = TEXT("Slot 5\0");
const TCHAR CPageInput::m_szCPMSlotChoice_Unplugged[] = TEXT("Unplugged\0");
const TCHAR CPageInput::m_szCPMSlotChoice_Unavailable[] = TEXT("Unavailable\0");


BOOL CALLBACK CPageInput::DlgProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	// Switch from static func to our instance
	return CPageInput::ms_this->DlgProcInternal(hWnd, message, wparam, lparam);
}

BOOL CPageInput::DlgProcInternal(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
	case WM_NOTIFY:
		{
			// Property Sheet notifications

			switch (((LPPSHNOTIFY)lparam)->hdr.code)
			{
			case PSN_SETACTIVE:
				// About to become the active page
				m_PropertySheetHelper.SetLastPage(m_Page);
				InitOptions(hWnd);
				break;
			case PSN_KILLACTIVE:
				SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);			// Changes are valid
				break;
			case PSN_APPLY:
				DlgOK(hWnd);
				SetWindowLong(hWnd, DWL_MSGRESULT, PSNRET_NOERROR);	// Changes are valid
				break;
			case PSN_QUERYCANCEL:
				// Can use this to ask user to confirm cancel
				break;
			case PSN_RESET:
				DlgCANCEL(hWnd);
				break;

			/*		// Could use this to display PDL() value
			case UDN_DELTAPOS:
				LPNMUPDOWN lpnmud = (LPNMUPDOWN) lparam;
				if (lpnmud->hdr.idFrom == IDC_SPIN_XTRIM)
				{
					static int x = 0;
					x = lpnmud->iPos + lpnmud->iDelta;
					x = SendDlgItemMessage(hWnd, IDC_SPIN_XTRIM, UDM_GETPOS, 0, 0);
				}
				else if (lpnmud->hdr.idFrom == IDC_SPIN_YTRIM)
				{
					static int y = 0;
					y = lpnmud->iPos + lpnmud->iDelta;
					y = SendDlgItemMessage(hWnd, IDC_SPIN_YTRIM, UDM_GETPOS, 0, 0);
				}
				break;
			*/
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wparam))
		{
		case IDC_JOYSTICK0:
			if(HIWORD(wparam) == CBN_SELCHANGE)
			{
				DWORD newjoytype = (DWORD)SendDlgItemMessage(hWnd,IDC_JOYSTICK0,CB_GETCURSEL,0,0);
				JoySetEmulationType(hWnd,m_nJoy0ChoiceTranlationTbl[newjoytype],JN_JOYSTICK0);
				InitJoystickChoices(hWnd, JN_JOYSTICK1, IDC_JOYSTICK1);	// Re-init joy1 list
			}
			break;

		case IDC_JOYSTICK1:
			if(HIWORD(wparam) == CBN_SELCHANGE)
			{
				DWORD newjoytype = (DWORD)SendDlgItemMessage(hWnd,IDC_JOYSTICK1,CB_GETCURSEL,0,0);
				JoySetEmulationType(hWnd,m_nJoy1ChoiceTranlationTbl[newjoytype],JN_JOYSTICK1);
				InitJoystickChoices(hWnd, JN_JOYSTICK0, IDC_JOYSTICK0);	// Re-init joy0 list
			}
			break;

		case IDC_SCROLLLOCK_TOGGLE:
			m_uScrollLockToggle = IsDlgButtonChecked(hWnd, IDC_SCROLLLOCK_TOGGLE) ? 1 : 0;
			break;

		case IDC_MOUSE_IN_SLOT4:
			{
				const UINT uNewState = IsDlgButtonChecked(hWnd, IDC_MOUSE_IN_SLOT4) ? 1 : 0;
				m_PropertySheetHelper.GetConfigNew().m_Slot[4] = uNewState ? CT_MouseInterface : CT_Empty;

				InitOptions(hWnd);	// re-init
			}
			break;

		case IDC_CPM_CONFIG:
			if(HIWORD(wparam) == CBN_SELCHANGE)
			{
				const DWORD NewCPMChoiceItem = (DWORD) SendDlgItemMessage(hWnd, IDC_CPM_CONFIG, CB_GETCURSEL, 0, 0);
				const CPMCHOICE NewCPMChoice = m_CPMComboItemToChoice[NewCPMChoiceItem];
				if (NewCPMChoice == m_CPMChoice)
					break;

				// Whatever has changed, the old slot will now be empty
				const SS_CARDTYPE Slot4 = m_PropertySheetHelper.GetConfigNew().m_Slot[4];
				const SS_CARDTYPE Slot5 = m_PropertySheetHelper.GetConfigNew().m_Slot[5];
				if (Slot4 == CT_Z80)
					m_PropertySheetHelper.GetConfigNew().m_Slot[4] = CT_Empty;
				else if (Slot5 == CT_Z80)
					m_PropertySheetHelper.GetConfigNew().m_Slot[5] = CT_Empty;

				// Insert CP/M card into new slot (or leave slot empty)
				if (NewCPMChoice == CPM_SLOT4)
					m_PropertySheetHelper.GetConfigNew().m_Slot[4] = CT_Z80;
				else if (NewCPMChoice == CPM_SLOT5)
					m_PropertySheetHelper.GetConfigNew().m_Slot[5] = CT_Z80;

				InitOptions(hWnd);	// re-init
			}
			break;

		case IDC_PASTE_FROM_CLIPBOARD:
			ClipboardInitiatePaste();
			break;
		}
		break;

	case WM_INITDIALOG:
		{
			InitJoystickChoices(hWnd, JN_JOYSTICK0, IDC_JOYSTICK0);
			InitJoystickChoices(hWnd, JN_JOYSTICK1, IDC_JOYSTICK1);

			SendDlgItemMessage(hWnd, IDC_SPIN_XTRIM, UDM_SETRANGE, 0, MAKELONG(128,-127));
			SendDlgItemMessage(hWnd, IDC_SPIN_YTRIM, UDM_SETRANGE, 0, MAKELONG(128,-127));

			SendDlgItemMessage(hWnd, IDC_SPIN_XTRIM, UDM_SETPOS, 0, MAKELONG(JoyGetTrim(true),0));
			SendDlgItemMessage(hWnd, IDC_SPIN_YTRIM, UDM_SETPOS, 0, MAKELONG(JoyGetTrim(false),0));

			CheckDlgButton(hWnd, IDC_SCROLLLOCK_TOGGLE, m_uScrollLockToggle ? BST_CHECKED : BST_UNCHECKED);

			InitOptions(hWnd);

			break;
		}
	}

	return FALSE;
}

void CPageInput::DlgOK(HWND hWnd)
{
	const UINT uNewJoyType0 = SendDlgItemMessage(hWnd, IDC_JOYSTICK0, CB_GETCURSEL, 0, 0);
	const UINT uNewJoyType1 = SendDlgItemMessage(hWnd, IDC_JOYSTICK1, CB_GETCURSEL, 0, 0);

	if (JoySetEmulationType(hWnd, m_nJoy0ChoiceTranlationTbl[uNewJoyType0], JN_JOYSTICK0))
	{
		REGSAVE(TEXT("Joystick 0 Emulation"), joytype[0]);
	}
	
	if (JoySetEmulationType(hWnd, m_nJoy1ChoiceTranlationTbl[uNewJoyType1], JN_JOYSTICK1))
	{
		REGSAVE(TEXT("Joystick 1 Emulation"), joytype[1]);
	}

	JoySetTrim((short)SendDlgItemMessage(hWnd, IDC_SPIN_XTRIM, UDM_GETPOS, 0, 0), true);
	JoySetTrim((short)SendDlgItemMessage(hWnd, IDC_SPIN_YTRIM, UDM_GETPOS, 0, 0), false);

	m_uMouseShowCrosshair = IsDlgButtonChecked(hWnd, IDC_MOUSE_CROSSHAIR) ? 1 : 0;
	m_uMouseRestrictToWindow = IsDlgButtonChecked(hWnd, IDC_MOUSE_RESTRICT_TO_WINDOW) ? 1 : 0;

	REGSAVE(TEXT(REGVALUE_PDL_XTRIM), JoyGetTrim(true));
	REGSAVE(TEXT(REGVALUE_PDL_YTRIM), JoyGetTrim(false));
	REGSAVE(TEXT(REGVALUE_SCROLLLOCK_TOGGLE), m_uScrollLockToggle);
	REGSAVE(TEXT(REGVALUE_MOUSE_CROSSHAIR), m_uMouseShowCrosshair);
	REGSAVE(TEXT(REGVALUE_MOUSE_RESTRICT_TO_WINDOW), m_uMouseRestrictToWindow);

	m_PropertySheetHelper.PostMsgAfterClose(hWnd, m_Page);
}

void CPageInput::InitOptions(HWND hWnd)
{
	InitSlotOptions(hWnd);
}

void CPageInput::InitJoystickChoices(HWND hWnd, int nJoyNum, int nIdcValue)
{
	TCHAR* pnzJoystickChoices;
	int *pnJoyTranslationTbl;
	int nJoyTranslationTblSize;
	unsigned short nJC_DISABLED, nJC_JOYSTICK, nJC_KEYBD_STANDARD, nJC_KEYBD_CENTERING, nJC_MAX;
	TCHAR** ppszJoyChoices;
	int nOtherJoyNum = nJoyNum == JN_JOYSTICK0 ? JN_JOYSTICK1 : JN_JOYSTICK0;

	if(nJoyNum == JN_JOYSTICK0)
	{
		pnzJoystickChoices = m_joystick0choices;
		pnJoyTranslationTbl = m_nJoy0ChoiceTranlationTbl;
		nJoyTranslationTblSize = sizeof(m_nJoy0ChoiceTranlationTbl);
		nJC_DISABLED = J0C_DISABLED;
		nJC_JOYSTICK = J0C_JOYSTICK1;
		nJC_KEYBD_STANDARD = J0C_KEYBD_STANDARD;
		nJC_KEYBD_CENTERING = J0C_KEYBD_CENTERING;
		nJC_MAX = J0C_MAX;
		ppszJoyChoices = (TCHAR**) m_pszJoy0Choices;
	}
	else
	{
		pnzJoystickChoices = m_joystick1choices;
		pnJoyTranslationTbl = m_nJoy1ChoiceTranlationTbl;
		nJoyTranslationTblSize = sizeof(m_nJoy1ChoiceTranlationTbl);
		nJC_DISABLED = J1C_DISABLED;
		nJC_JOYSTICK = J1C_JOYSTICK2;
		nJC_KEYBD_STANDARD = J1C_KEYBD_STANDARD;
		nJC_KEYBD_CENTERING = J1C_KEYBD_CENTERING;
		nJC_MAX = J1C_MAX;
		ppszJoyChoices = (TCHAR**) m_pszJoy1Choices;
	}

	TCHAR* pszMem = pnzJoystickChoices;
	int nIdx = 0;
	memset(pnJoyTranslationTbl, -1, nJoyTranslationTblSize);

	// Build the Joystick choices string list. These first 2 are always selectable.
	memcpy(pszMem, ppszJoyChoices[nJC_DISABLED], strlen(ppszJoyChoices[nJC_DISABLED])+1);
	pszMem += strlen(ppszJoyChoices[nJC_DISABLED])+1;
	pnJoyTranslationTbl[nIdx++] = nJC_DISABLED;

	memcpy(pszMem, ppszJoyChoices[nJC_JOYSTICK], strlen(ppszJoyChoices[nJC_JOYSTICK])+1);
	pszMem += strlen(ppszJoyChoices[nJC_JOYSTICK])+1;
	pnJoyTranslationTbl[nIdx++] = nJC_JOYSTICK;

	// Now exclude the other Joystick type (if it exists) from this new list
	for(UINT i=nJC_KEYBD_STANDARD; i<nJC_MAX; i++)
	{
		if( ( (i == nJC_KEYBD_STANDARD) || (i == nJC_KEYBD_CENTERING) ) &&
			( (joytype[nOtherJoyNum] == nJC_KEYBD_STANDARD) || (joytype[nOtherJoyNum] == nJC_KEYBD_CENTERING) )
		  )
		{
			continue;
		}

		if(joytype[nOtherJoyNum] != i)
		{
			memcpy(pszMem, ppszJoyChoices[i], strlen(ppszJoyChoices[i])+1);
			pszMem += strlen(ppszJoyChoices[i])+1;
			pnJoyTranslationTbl[nIdx++] = i;
		}
	}

	*pszMem = 0x00;	// Doubly null terminated

	m_PropertySheetHelper.FillComboBox(hWnd, nIdcValue, pnzJoystickChoices, joytype[nJoyNum]);
}

void CPageInput::InitSlotOptions(HWND hWnd)
{
	const SS_CARDTYPE Slot4 = m_PropertySheetHelper.GetConfigNew().m_Slot[4];

	const bool bIsSlot4Mouse = Slot4 == CT_MouseInterface;
	CheckDlgButton(hWnd, IDC_MOUSE_IN_SLOT4, bIsSlot4Mouse ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd, IDC_MOUSE_CROSSHAIR, m_uMouseShowCrosshair ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd, IDC_MOUSE_RESTRICT_TO_WINDOW, m_uMouseRestrictToWindow ? BST_CHECKED : BST_UNCHECKED);

	const bool bIsSlot4Empty = Slot4 == CT_Empty;
	EnableWindow(GetDlgItem(hWnd, IDC_MOUSE_IN_SLOT4), (bIsSlot4Mouse || bIsSlot4Empty) ? TRUE : FALSE);
	EnableWindow(GetDlgItem(hWnd, IDC_MOUSE_CROSSHAIR), bIsSlot4Mouse ? TRUE : FALSE);
	EnableWindow(GetDlgItem(hWnd, IDC_MOUSE_RESTRICT_TO_WINDOW), bIsSlot4Mouse ? TRUE : FALSE);

	InitCPMChoices(hWnd);
}

void CPageInput::InitCPMChoices(HWND hWnd)
{
	const SS_CARDTYPE Slot4 = m_PropertySheetHelper.GetConfigNew().m_Slot[4];
	const SS_CARDTYPE Slot5 = m_PropertySheetHelper.GetConfigNew().m_Slot[5];

	if (Slot4 == CT_Z80)		m_CPMChoice = CPM_SLOT4;
	else if (Slot5 == CT_Z80)	m_CPMChoice = CPM_SLOT5;
	else						m_CPMChoice = CPM_UNPLUGGED;

	for (UINT i=0; i<_CPM_MAX_CHOICES; i++)
		m_CPMComboItemToChoice[i] = CPM_UNAVAILABLE;

	UINT uStringOffset = 0;
	UINT uComboItemIdx = 0;

	const bool bIsSlot4Empty = Slot4 == CT_Empty;
	const bool bIsSlot4CPM   = Slot4 == CT_Z80;
	const bool bIsSlot5Empty = Slot5 == CT_Empty;
	const bool bIsSlot5CPM   = Slot5 == CT_Z80;

	if (bIsSlot4Empty || bIsSlot4CPM)
	{
		const UINT uStrLen = strlen(m_szCPMSlotChoice_Slot4)+1;
		memcpy(&m_szCPMSlotChoices[uStringOffset], m_szCPMSlotChoice_Slot4, uStrLen);
		uStringOffset += uStrLen;

		m_CPMComboItemToChoice[uComboItemIdx++] = CPM_SLOT4;
	}

	if (bIsSlot5Empty || bIsSlot5CPM)
	{
		const UINT uStrLen = strlen(m_szCPMSlotChoice_Slot5)+1;
		memcpy(&m_szCPMSlotChoices[uStringOffset], m_szCPMSlotChoice_Slot5, uStrLen);
		uStringOffset += uStrLen;

		m_CPMComboItemToChoice[uComboItemIdx++] = CPM_SLOT5;
	}

	if (uStringOffset)
	{
		const UINT uStrLen = strlen(m_szCPMSlotChoice_Unplugged)+1;
		memcpy(&m_szCPMSlotChoices[uStringOffset], m_szCPMSlotChoice_Unplugged, uStrLen);
		uStringOffset += uStrLen;

		m_CPMComboItemToChoice[uComboItemIdx] = CPM_UNPLUGGED;
	}
	else
	{
		const UINT uStrLen = strlen(m_szCPMSlotChoice_Unavailable)+1;
		memcpy(&m_szCPMSlotChoices[uStringOffset], m_szCPMSlotChoice_Unavailable, uStrLen);
		uStringOffset += uStrLen;

		m_CPMChoice = CPM_UNAVAILABLE;	// Force this
		m_CPMComboItemToChoice[uComboItemIdx] = CPM_UNAVAILABLE;
	}

	m_szCPMSlotChoices[uStringOffset] = 0;	// Doubly null terminated

	// 

	UINT uCurrentChoice = uComboItemIdx;	// Default to last item (either UNPLUGGED or UNAVAILABLE)
	for (UINT i=0; i<=uComboItemIdx; i++)
	{
		if (m_CPMComboItemToChoice[i] == m_CPMChoice)
		{
			uCurrentChoice = i;
			break;
		}
	}

	m_PropertySheetHelper.FillComboBox(hWnd, IDC_CPM_CONFIG, m_szCPMSlotChoices, uCurrentChoice);
}