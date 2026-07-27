/*
Copyright 2022 @Yowkees
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H

#include "quantum.h"

// Home-row mods: GUI / Alt / Ctrl / Shift / Hyper from outside in.
#define HM_N LGUI_T(KC_N)
#define HM_R LALT_T(KC_R)
#define HM_T LCTL_T(KC_T)
#define HM_S LSFT_T(KC_S)
#define HM_G ALL_T(KC_G)
#define HM_Y ALL_T(KC_Y)
#define HM_H LSFT_T(KC_H)
#define HM_A LCTL_T(KC_A)
#define HM_E LALT_T(KC_E)
#define HM_I LGUI_T(KC_I)

// Layer 1 clipboard keys with home-row mods.
#define HM_UNDO  LGUI_T(KC_UNDO)
#define HM_CUT   LALT_T(KC_CUT)
#define HM_COPY  LCTL_T(KC_COPY)
#define HM_PASTE LSFT_T(KC_PASTE)

// Tap Z; hold on layer 1 for 40% pointer and scrolling speed.
#define PR_Z LT(4, KC_Z)

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  // Layer 0 — Graphite alphas with home-row mods.
  [0] = LAYOUT_universal(
    KC_B     , KC_L     , KC_D     , KC_W     , KC_Z     ,                              KC_QUOT  , KC_F     , KC_O     , KC_U     , KC_J     ,
    HM_N     , HM_R     , HM_T     , HM_S     , HM_G     ,                              HM_Y     , HM_H     , HM_A     , HM_E     , HM_I     ,
    KC_Q     , KC_X     , KC_M     , KC_C     , KC_V     ,                              KC_K     , KC_P     , KC_COMM  , KC_DOT   , KC_SLSH  ,
    KBC_SAVE , CPI_D100 , CPI_I100 , LSFT_T(KC_ESC), LT(1,KC_SPC), LT(3,KC_TAB),  LT(3,KC_BSPC), LT(2,KC_ENT), KC_NO   , KC_NO    , KC_NO    , KC_NO
  ),

  // Layer 1 — Mouse, navigation, clipboard.
  [1] = LAYOUT_universal(
    SCRL_MO  , MS_BTN2  , MS_BTN1  , MS_BTN3  , PR_Z     ,                              _______  , _______  , _______  , _______  , _______  ,
    HM_UNDO  , HM_CUT   , HM_COPY  , HM_PASTE , KC_HYPR  ,                              _______  , KC_LEFT  , KC_DOWN  , KC_UP    , KC_RGHT  ,
    _______  , _______  , _______  , _______  , _______  ,                              _______  , KC_END   , KC_PGDN  , KC_PGUP  , KC_HOME  ,
    SSNP_FRE , SCRL_DVI , SCRL_DVD , _______  , _______  , _______  ,        _______  , KC_DEL   , _______  , _______  , _______  , _______
  ),

  // Layer 2 — Numpad and common symbols.
  [2] = LAYOUT_universal(
    S(KC_6)  , KC_7     , KC_8     , KC_9     , S(KC_5)  ,                              _______  , _______  , _______  , _______  , _______  ,
    KC_0     , KC_4     , KC_5     , KC_6     , S(KC_8)  ,                              _______  , _______  , _______  , _______  , _______  ,
    KC_EQL   , KC_1     , KC_2     , KC_3     , S(KC_1)  ,                              _______  , _______  , _______  , _______  , _______  ,
    _______  , _______  , _______  , KC_MINS  , _______  , S(KC_EQL),       _______   , _______  , _______  , _______  , _______  , _______
  ),

  // Layer 3 — Brackets and remaining punctuation.
  [3] = LAYOUT_universal(
    KC_GRV   , S(KC_BSLS), KC_BSLS  , KC_LBRC  , KC_RBRC  ,                              _______  , _______  , _______  , _______  , _______  ,
    S(KC_3)  , KC_SCLN, S(KC_SCLN)  , S(KC_9)  , S(KC_0)  ,                              _______  , _______  , _______  , _______  , _______  ,
    S(KC_4)  , S(KC_2)   , S(KC_7)  , S(KC_LBRC), S(KC_RBRC),                             _______  , _______  , _______  , _______  , _______  ,
    _______  , _______   , _______  , S(KC_MINS), _______ , S(KC_GRV) ,      _______   , _______  , _______  , _______  , _______  , _______
  ),

  // Layer 4 — Precision-pointer hold layer; all keys remain transparent.
  [4] = LAYOUT_universal(
    _______  , _______  , _______  , _______  , _______  ,                              _______  , _______  , _______  , _______  , _______  ,
    _______  , _______  , _______  , _______  , _______  ,                              _______  , _______  , _______  , _______  , _______  ,
    _______  , _______  , _______  , _______  , _______  ,                              _______  , _______  , _______  , _______  , _______  ,
    _______  , _______  , _______  , _______  , _______  , _______  ,        _______  , _______  , _______  , _______  , _______  , _______
  ),
};
// clang-format on

static bool    precision_mode    = false;
static int32_t precision_accum_x = 0;
static int32_t precision_accum_y = 0;
static int32_t precision_accum_h = 0;
static int32_t precision_accum_v = 0;

static int16_t precision_scale(int16_t motion, int32_t *accum) {
    *accum += motion * 2;
    int16_t scaled = *accum / 5;
    *accum -= scaled * 5;
    return scaled;
}

// Morph LCtrl+I into LCtrl+K (only when LCtrl is the sole active modifier).
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (keycode == PR_Z) {
        precision_mode = record->event.pressed;
        if (!precision_mode) {
            precision_accum_x = 0;
            precision_accum_y = 0;
            precision_accum_h = 0;
            precision_accum_v = 0;
        }
    }

    if (keycode == HM_I && record->tap.count > 0 && record->event.pressed) {
        if (get_mods() == MOD_BIT(KC_LCTL)) {
            tap_code(KC_K);
            return false;
        }
    }
    return true;
}

report_mouse_t pointing_device_task_user(report_mouse_t report) {
    if (!precision_mode) {
        return report;
    }

    // Scale the final report so Keyball's native scroll-divider logic remains intact.
    report.x = precision_scale(report.x, &precision_accum_x);
    report.y = precision_scale(report.y, &precision_accum_y);
    report.h = precision_scale(report.h, &precision_accum_h);
    report.v = precision_scale(report.v, &precision_accum_v);
    return report;
}
