#include "pch.h"

DataStore gConfig("MapEditorSA", true);

Hotkey newObjKey(VK_KEY_N, VK_KEY_N);
Hotkey copyKey(VK_LCONTROL, VK_KEY_C);
Hotkey pasteKey(VK_LCONTROL, VK_KEY_V);
Hotkey deleteKey(VK_DELETE, VK_DELETE);
Hotkey snapKey(VK_KEY_Z, VK_KEY_Z);
Hotkey editorOpenKey(VK_LCONTROL, VK_TAB);
Hotkey viewportSwitchKey(VK_KEY_E, VK_KEY_E);
Hotkey toggleUIKey(VK_LCONTROL, VK_KEY_P);
Hotkey copyHoveredObjName(VK_LSHIFT, VK_KEY_C);
