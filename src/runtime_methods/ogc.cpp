/*
    CrossLang is a dynamically-typed scripting language built on
   TessesFramework, named in honor of Jesus's sacrifice.

    Copyright (C) 2026 Mike Nolan
    SPDX-License-Identifier: GPL-3.0-or-later WITH TessesFramework-Exception-1.0

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "CrossLang.hpp"

#if defined(GEKKO)
#include <gccore.h>
#include <ogc/pad.h>
#include <ogcsys.h>
#if defined(HW_RVL)
#include <wiiuse/wpad.h>
#endif
#endif
namespace Tesses::CrossLang {
#if defined(GEKKO)
#if defined(HW_RVL)
static TObject OGC_WPAD_ScanPads(GCList &ls, std::vector<TObject> args) {
    return (int64_t)WPAD_ScanPads();
}
static TObject OGC_WPAD_ButtonsUp(GCList &ls, std::vector<TObject> args) {
    int64_t chan;
    if (GetArgument(args, 0, chan))
        return (int64_t)WPAD_ButtonsUp((int)chan);
    return 0;
}
static TObject OGC_WPAD_ButtonsDown(GCList &ls, std::vector<TObject> args) {
    int64_t chan;
    if (GetArgument(args, 0, chan))
        return (int64_t)WPAD_ButtonsDown((int)chan);
    return 0;
}
static TObject OGC_WPAD_ButtonsHeld(GCList &ls, std::vector<TObject> args) {
    int64_t chan;
    if (GetArgument(args, 0, chan))
        return (int64_t)WPAD_ButtonsDown((int)chan);
    return 0;
}
static TObject OGC_WPAD_BatteryLevel(GCList &ls, std::vector<TObject> args) {
    int64_t chan;
    if (GetArgument(args, 0, chan))
        return (int64_t)WPAD_BatteryLevel((int)chan);
    return 0;
}
#endif
#endif
void TStd::RegisterOGC(std::shared_ptr<GC> gc, TRootEnvironment *env) {
    GCList ls(gc);
#if defined(GEKKO)

    gc->BarrierBegin();

    TDictionary *dict_ogc_pad = TDictionary::Create(ls);
#if defined(HW_RVL)
    TDictionary *dict_rvl_wpad = TDictionary::Create(ls);
    dict_rvl_wpad->DeclareFunction(gc, "ScanPads", "Scan wiimotes", {},
                                   OGC_WPAD_ScanPads);
    dict_rvl_wpad->DeclareFunction(gc, "ButtonsDown", "Is button down", {"pad"},
                                   OGC_WPAD_ButtonsDown);
    dict_rvl_wpad->SetValue("BUTTON_A", (int64_t)WPAD_BUTTON_A);
    dict_rvl_wpad->SetValue("BUTTON_B", (int64_t)WPAD_BUTTON_B);
    dict_rvl_wpad->SetValue("BUTTON_HOME", (int64_t)WPAD_BUTTON_HOME);
    dict_rvl_wpad->SetValue("BUTTON_UP", (int64_t)WPAD_BUTTON_UP);
    dict_rvl_wpad->SetValue("BUTTON_DOWN", (int64_t)WPAD_BUTTON_DOWN);
    dict_rvl_wpad->SetValue("BUTTON_LEFT", (int64_t)WPAD_BUTTON_LEFT);
    dict_rvl_wpad->SetValue("BUTTON_RIGHT", (int64_t)WPAD_BUTTON_RIGHT);
    dict_rvl_wpad->SetValue("BUTTON_1", (int64_t)WPAD_BUTTON_1);
    dict_rvl_wpad->SetValue("BUTTON_2", (int64_t)WPAD_BUTTON_2);
    dict_rvl_wpad->SetValue("BUTTON_PLUS", (int64_t)WPAD_BUTTON_PLUS);
    dict_rvl_wpad->SetValue("BUTTON_MINUS", (int64_t)WPAD_BUTTON_MINUS);
    env->DeclareVariable("WPAD", dict_rvl_wpad);

#endif
    env->DeclareVariable("PAD", dict_ogc_pad);
    gc->BarrierEnd();
#endif
    env->permissions.canRegisterOGC = true;
}
} // namespace Tesses::CrossLang