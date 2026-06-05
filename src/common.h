/**************************************************************************/
/*  common.h                                                              */
/**************************************************************************/
/*                         This file is part of:                          */
/*                     INTERACTIVE GRID GDExtension                       */
/*         https://github.com/antoinecharruel/interactive_grid            */
/**************************************************************************/
/* Copyright (c) 2025 Antoine Charruel.                                   */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#ifdef _WIN32
#include <string.h>
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#else
#include <strings.h> // POSIX systems
#endif

#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

inline void PrintError(const godot::String &p_file, const godot::String &p_func, const int &p_line, const godot::Variant &p_variant) {
	godot::String line_str = godot::String("[") + p_file + "@" + p_func + ":" + godot::itos(p_line) + "] " + godot::String(p_variant);
	godot::UtilityFunctions::printerr(line_str);
}

template <typename... Args>
void PrintError(const godot::String &p_file, const godot::String &p_func, const int &p_line, const godot::Variant &p_variant, Args... p_args) {
	godot::String line_str = godot::String("[") + p_file + "@" + p_func + ":" + godot::itos(p_line) + "] " + godot::String(p_variant);
	godot::UtilityFunctions::printerr(line_str, p_args...);
}

inline void PrintLine(const godot::String &p_file, const godot::String &p_func, const int &p_line, const godot::Variant &p_variant) {
	godot::String line_str = godot::String("[") + p_file + "@" + p_func + ":" + godot::itos(p_line) + "] " + godot::String(p_variant);
	godot::UtilityFunctions::print(line_str);
}

template <typename... Args>
void PrintLine(const godot::String &p_file, const godot::String &p_func, const int &p_line, const godot::Variant &p_variant, Args... p_args) {
	godot::String line_str = godot::String("[") + p_file + "@" + p_func + ":" + godot::itos(p_line) + "] " + godot::String(p_variant);
	godot::UtilityFunctions::print(line_str, p_args...);
}