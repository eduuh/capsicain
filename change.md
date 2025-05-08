# Capsicain config file

# This is the (very complex) config that I use myself.
# Check capsicain.example.ini for something simpler.
# Config 1+2 use my "KingCon / King Configuration"; its aim is that no finger must move more than one key (like the King in Chess)
# Requires that the Windows keyboard layout is set to standard US-English

#.ini versions:
#v35: new map for alt chars, new tappedAlt chars
#v36: KEY_MODIFIER_IFTAPPED_MAPPING
#v37: add LControlBlocksAlphaMapping, ProcessOnlyFirstKeyboard
#v38: add Workman Std, Colemak, Dvorak. Now with layerName
#v38b: rename to LControlLWinBlocksAlphaMapping
#v38c: add special chars ╔═╗║║╚╝╠╣╩╦ ┌─┐││└┘├┤┴┬ º¹²³
#v39: comment char is now #
#V40: changed key labels from 'DOT' to '.' and all similar cases. '[' was a pain...
#v41: introduced ALPHA_FROM-ALPHA_TO-ALPHA_END.
#v42: cleanup
#v43: changed some NumPad key labels, put ┌─┐╚═╝ on NumPad
#v44: enabled layer-specific sections (undone with v45...)
#v45: modular; tagged lines with INCLUDE, GLOBAL/OPTION/REWIRE/COMBO/ALPHA_FROM-ALPHA_TO-ALPHA_END
#v46: sequence() function. Redefine mod state to &^T. Rename ShiftShiftToShiftLock
#v46b: rules for capsAlt-H,S,D,F and caps-QWER, YXCVB
#v47: can map key to ESC. REWIRE for non-modifiers too. map A B LOCK
#v47b: remove LOCKs. Too many accidents. Experimental section that reuses the number row.
#v48: cleanup. Can now map GRAVE to ESC (eXit and layer change require original ESC key). 
#v49: removed REWIRE A B LOCK. Lock with ESC+modifier, unlock with ESC. Much more reliable.
#v49b: suppress Win tapped
#v50: alt+space=return
#v51: Tab+Number=Function key
#v52: undo alt+space=return. Too many fast-typing accidents.
#v52b: documentation on new SC_0XNN labels
#v52c: cleaned old references to tag "MODIFIER"
#v52d: new layer 8 for testing sequences
#v53: add 3 new GLOBAL for startup options
#v54: replace ShiftShiftToShiftLock with COMBOs
#v55: add GLOBAL StartAHK
#v56: new TapHold+Alt style
#v57: replace PAUSE:1 with SLEEP:100 (now milliseconds)
#v58: remove '^' defs from moddedKey(). Now all modifiers are released, then restored after the moddedKey send.
#v59: rewire LWin MOD11 // LWIN
#v59: add dead keys and diacritics àèìòùÀÈÌÒÙ áéíóúÁÉÍÓÚ âêîôûÂÊÎÔÛ çÇ / äëïöüÿÄËÏÖÜŸ ãñõÃÑÕ øØ åÅ æÆœŒ šŠžŽ / §™±
#v60: add my windows shortcuts
#v61: remove obsolete GLOBAL delayOnStartup
#v62: change all AHK triggers to F14/F15
#v63: add capsicainEnableDisableKey
#v64: change name to capsicainOnOffKey
#i65: Test CAPS+F1..F10 for layer switch. CAPS+F12 for previous layer
#i66: Remove reference sections. It's all in the wiki now https://github.com/cajhin/capsicain/wiki
#i67: Find&Replace all "Layer">"Config", and "layer">"config"
#i68: WinAlt D -> F15+u = wsl.exe
#i69: define combos for macros
#i70: allow ALT+NumPad combo for virtual numpad (e.g. with Alt+Tab+M=NP1) 
#i71: record macros with Ctrl+Caps+Number, play with Ctrl+TapCaps+Number
#i72: new GLOBAL DontTranslateMessyKeys
#i72: new GLOBAL DontProtectConsole
#i73: ctrl+both shift for Caps toggle (needed in case of Linux VM with grabbed keyboard)
#i74: new GLOBAL deactivateWinkeyStartmenu