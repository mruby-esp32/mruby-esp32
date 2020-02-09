/*
 Created by Katsuhiko KAGEYAMA(@kishima) - <https://silentworlds.info>
 Copyright (c) 2019-2020 Katsuhiko KAGEYAMA.
 All rights reserved.

 This file is part of Family mruby.

 Family mruby is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Family mruby is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Family mruby.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

enum class FmrbVkey : int{
  VK_NONE=0,            /**< No character (marks the first virtual key) */

  VK_SPACE,           /**< Space */
  VK_0,               /**< Number 0 */
  VK_1,               /**< Number 1 */
  VK_2,               /**< Number 2 */
  VK_3,               /**< Number 3 */
  VK_4,               /**< Number 4 */
  VK_5,               /**< Number 5 */
  VK_6,               /**< Number 6 */
  VK_7,               /**< Number 7 */
  VK_8,               /**< Number 8 */
  VK_9,               /**< Number 9 */
  VK_KP_0,            /**< Keypad number 0 */
  VK_KP_1,            /**< Keypad number 1 */
  VK_KP_2,            /**< Keypad number 2 */
  VK_KP_3,            /**< Keypad number 3 */
  VK_KP_4,            /**< Keypad number 4 */
  VK_KP_5,            /**< Keypad number 5 */
  VK_KP_6,            /**< Keypad number 6 */
  VK_KP_7,            /**< Keypad number 7 */
  VK_KP_8,            /**< Keypad number 8 */
  VK_KP_9,            /**< Keypad number 9 */
  VK_a,               /**< Lower case letter 'a' */
  VK_b,               /**< Lower case letter 'b' */
  VK_c,               /**< Lower case letter 'c' */
  VK_d,               /**< Lower case letter 'd' */
  VK_e,               /**< Lower case letter 'e' */
  VK_f,               /**< Lower case letter 'f' */
  VK_g,               /**< Lower case letter 'g' */
  VK_h,               /**< Lower case letter 'h' */
  VK_i,               /**< Lower case letter 'i' */
  VK_j,               /**< Lower case letter 'j' */
  VK_k,               /**< Lower case letter 'k' */
  VK_l,               /**< Lower case letter 'l' */
  VK_m,               /**< Lower case letter 'm' */
  VK_n,               /**< Lower case letter 'n' */
  VK_o,               /**< Lower case letter 'o' */
  VK_p,               /**< Lower case letter 'p' */
  VK_q,               /**< Lower case letter 'q' */
  VK_r,               /**< Lower case letter 'r' */
  VK_s,               /**< Lower case letter 's' */
  VK_t,               /**< Lower case letter 't' */
  VK_u,               /**< Lower case letter 'u' */
  VK_v,               /**< Lower case letter 'v' */
  VK_w,               /**< Lower case letter 'w' */
  VK_x,               /**< Lower case letter 'x' */
  VK_y,               /**< Lower case letter 'y' */
  VK_z,               /**< Lower case letter 'z' */
  VK_A,               /**< Upper case letter 'A' */
  VK_B,               /**< Upper case letter 'B' */
  VK_C,               /**< Upper case letter 'C' */
  VK_D,               /**< Upper case letter 'D' */
  VK_E,               /**< Upper case letter 'E' */
  VK_F,               /**< Upper case letter 'F' */
  VK_G,               /**< Upper case letter 'G' */
  VK_H,               /**< Upper case letter 'H' */
  VK_I,               /**< Upper case letter 'I' */
  VK_J,               /**< Upper case letter 'J' */
  VK_K,               /**< Upper case letter 'K' */
  VK_L,               /**< Upper case letter 'L' */
  VK_M,               /**< Upper case letter 'M' */
  VK_N,               /**< Upper case letter 'N' */
  VK_O,               /**< Upper case letter 'O' */
  VK_P,               /**< Upper case letter 'P' */
  VK_Q,               /**< Upper case letter 'Q' */
  VK_R,               /**< Upper case letter 'R' */
  VK_S,               /**< Upper case letter 'S' */
  VK_T,               /**< Upper case letter 'T' */
  VK_U,               /**< Upper case letter 'U' */
  VK_V,               /**< Upper case letter 'V' */
  VK_W,               /**< Upper case letter 'W' */
  VK_X,               /**< Upper case letter 'X' */
  VK_Y,               /**< Upper case letter 'Y' */
  VK_Z,               /**< Upper case letter 'Z' */
  VK_GRAVEACCENT,     /**< Grave accent: ` */
  VK_ACUTEACCENT,     /**< Acute accent: ´ */
  VK_QUOTE,           /**< Quote: ' */
  VK_QUOTEDBL,        /**< Double quote: " */
  VK_EQUALS,          /**< Equals: = */
  VK_MINUS,           /**< Minus: - */
  VK_KP_MINUS,        /**< Keypad minus: - */
  VK_PLUS,            /**< Plus: + */
  VK_KP_PLUS,         /**< Keypad plus: + */
  VK_KP_MULTIPLY,     /**< Keypad multiply: * */
  VK_ASTERISK,        /**< Asterisk: * */
  VK_BACKSLASH,       /**< Backslash: \ */
  VK_KP_DIVIDE,       /**< Keypad divide: / */
  VK_SLASH,           /**< Slash: / */
  VK_KP_PERIOD,       /**< Keypad period: . */
  VK_PERIOD,          /**< Period: . */
  VK_COLON,           /**< Colon: : */
  VK_COMMA,           /**< Comma: , */
  VK_SEMICOLON,       /**< Semicolon: ; */
  VK_AMPERSAND,       /**< Ampersand: & */
  VK_VERTICALBAR,     /**< Vertical bar: | */
  VK_HASH,            /**< Hash: # */
  VK_AT,              /**< At: @ */
  VK_CARET,           /**< Caret: ^ */
  VK_DOLLAR,          /**< Dollar: $ */
  VK_POUND,           /**< Pound: £ */
  VK_EURO,            /**< Euro: € */
  VK_PERCENT,         /**< Percent: % */
  VK_EXCLAIM,         /**< Exclamation mark: ! */
  VK_QUESTION,        /**< Question mark: ? */
  VK_LEFTBRACE,       /**< Left brace: { */
  VK_RIGHTBRACE,      /**< Right brace: } */
  VK_LEFTBRACKET,     /**< Left bracket: [ */
  VK_RIGHTBRACKET,    /**< Right bracket: ] */
  VK_LEFTPAREN,       /**< Left parenthesis: ( */
  VK_RIGHTPAREN,      /**< Right parenthesis: ) */
  VK_LESS,            /**< Less: < */
  VK_GREATER,         /**< Greater: > */
  VK_UNDERSCORE,      /**< Underscore: _ */
  VK_DEGREE,          /**< Degree: ° */
  VK_SECTION,         /**< Section: § */
  VK_TILDE,           /**< Tilde: ~ */
  VK_NEGATION,        /**< Negation: ¬ */
  VK_GRAVE_a,         /**< Grave 'a': à */
  VK_GRAVE_e,         /**< Grave 'e': è */
  VK_ACUTE_e,         /**< Acute 'e': é */
  VK_GRAVE_i,         /**< Grave 'i': ì */
  VK_GRAVE_o,         /**< Grave 'o': ò */
  VK_GRAVE_u,         /**< Grave 'u': ù */
  VK_CEDILLA_c,       /**< Cedilla 'c': ç */
  VK_ESZETT,          /**< Eszett: ß */
  VK_UMLAUT_u,        /**< Umlaut 'u': ü */
  VK_UMLAUT_o,        /**< Umlaut 'o': ö */
  VK_UMLAUT_a,        /**< Umlaut 'a': ä */

  VK_VISIBLE_MAX,
  //invisible
  VK_LSHIFT,          /**< Left SHIFT */
  VK_RSHIFT,          /**< Right SHIFT */
  VK_LALT,            /**< Left ALT */
  VK_RALT,            /**< Right ALT */
  VK_LCTRL,           /**< Left CTRL */
  VK_RCTRL,           /**< Right CTRL */
  VK_LGUI,            /**< Left GUI */
  VK_RGUI,            /**< Right GUI */
  VK_ESCAPE,          /**< ESC */
  VK_PRINTSCREEN1,    /**< PRINTSCREEN is translated as separated VK_PRINTSCREEN1 and VK_PRINTSCREEN2. VK_PRINTSCREEN2 is also generated by CTRL or SHIFT + PRINTSCREEN. So pressing PRINTSCREEN both VK_PRINTSCREEN1 and VK_PRINTSCREEN2 are generated, while pressing CTRL+PRINTSCREEN or SHIFT+PRINTSCREEN only VK_PRINTSCREEN2 is generated. */
  VK_PRINTSCREEN2,    /**< PRINTSCREEN. See VK_PRINTSCREEN1 */
  VK_SYSREQ,          /**< SYSREQ */
  VK_INSERT,          /**< INS */
  VK_KP_INSERT,       /**< Keypad INS */
  VK_DELETE,          /**< DEL */
  VK_KP_DELETE,       /**< Keypad DEL */
  VK_BACKSPACE,       /**< Backspace */
  VK_HOME,            /**< HOME */
  VK_KP_HOME,         /**< Keypad HOME */
  VK_END,             /**< END */
  VK_KP_END,          /**< Keypad END */
  VK_PAUSE,           /**< PAUSE */
  VK_BREAK,           /**< CTRL + PAUSE */
  VK_SCROLLLOCK,      /**< SCROLLLOCK */
  VK_NUMLOCK,         /**< NUMLOCK */
  VK_CAPSLOCK,        /**< CAPSLOCK */
  VK_TAB,             /**< TAB */
  VK_RETURN,          /**< RETURN */
  VK_KP_ENTER,        /**< Keypad ENTER */
  VK_APPLICATION,     /**< APPLICATION / MENU key */
  VK_PAGEUP,          /**< PAGEUP */
  VK_KP_PAGEUP,       /**< Keypad PAGEUP */
  VK_PAGEDOWN,        /**< PAGEDOWN */
  VK_KP_PAGEDOWN,     /**< Keypad PAGEDOWN */
  VK_UP,              /**< Cursor UP */
  VK_KP_UP,           /**< Keypad cursor UP  */
  VK_DOWN,            /**< Cursor DOWN */
  VK_KP_DOWN,         /**< Keypad cursor DOWN */
  VK_LEFT,            /**< Cursor LEFT */
  VK_KP_LEFT,         /**< Keypad cursor LEFT */
  VK_RIGHT,           /**< Cursor RIGHT */
  VK_KP_RIGHT,        /**< Keypad cursor RIGHT */
  VK_KP_CENTER,       /**< Keypad CENTER key */
  VK_F1,              /**< F1 function key */
  VK_F2,              /**< F2 function key */
  VK_F3,              /**< F3 function key */
  VK_F4,              /**< F4 function key */
  VK_F5,              /**< F5 function key */
  VK_F6,              /**< F6 function key */
  VK_F7,              /**< F7 function key */
  VK_F8,              /**< F8 function key */
  VK_F9,              /**< F9 function key */
  VK_F10,             /**< F10 function key */
  VK_F11,             /**< F11 function key */
  VK_F12,             /**< F12 function key */
  // Additional keys
  VK_CTRL_A,
  VK_CTRL_B,
  VK_CTRL_C,
  VK_CTRL_D,
  VK_LAST,            // marks the last virtual key
};
