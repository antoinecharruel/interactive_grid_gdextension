/*+===================================================================
File: common.cpp

Summary: This file provides utility functions and common includes 
         used across the InteractiveGrid Godot GDExtension project.

Last Modified: November 25, 2025

This file is part of the InteractiveGrid GDExtension Source Code.
Repository: https://github.com/antoinecharruel/interactive_grid

Version InteractiveGrid: 1.5.0
Version: Godot Engine v4.5.stable.steam - https://godotengine.org

Author: Antoine Charruel
===================================================================+*/

#pragma once

#ifdef _WIN32
#include <string.h>
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#else
#include <strings.h> // POSIX systems
#endif

#include <array>
#include <sstream>
#include <vector>

// Godot engine
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

inline void PrintError(const godot::String &file, const godot::String &func, const int &line, const godot::Variant &p_variant) {
	/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Summary: Prints an error message to the Godot console with the specified
  file, function, line number, and error message string.

  ref : godot_cpp/core/print_string.hpp
  -----------------------------------------------------------------F-F*/
	godot::String line_str = godot::String("[") + file + "@" + func + ":" + godot::itos(line) + "] " + godot::String(p_variant);
	godot::UtilityFunctions::printerr(line_str);
}

template <typename... Args>
// ref : godot_cpp/core/print_string.hpp
void PrintError(const godot::String &file, const godot::String &func, const int &line, const godot::Variant &p_variant, Args... p_args) {
	/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Summary: Prints an error message to the Godot console with the specified
  file, function, line number, and error message string with args.

  ref : godot_cpp/core/print_string.hpp
  -----------------------------------------------------------------F-F*/
	godot::String line_str = godot::String("[") + file + "@" + func + ":" + godot::itos(line) + "] " + godot::String(p_variant);
	godot::UtilityFunctions::printerr(line_str, p_args...);
}

inline void PrintLine(const godot::String &file, const godot::String &func, const int &line, const godot::Variant &p_variant) {
	/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Summary: Prints an log message to the Godot console with the specified file,
                    function, line number, and log message string.
  -----------------------------------------------------------------F-F*/
	godot::String line_str = godot::String("[") + file + "@" + func + ":" + godot::itos(line) + "] " + godot::String(p_variant);
	godot::UtilityFunctions::print(line_str);
}

template <typename... Args>
// ref : godot_cpp/core/print_string.hpp
void PrintLine(const godot::String &file, const godot::String &func, const int &line, const godot::Variant &p_variant, Args... p_args) {
	/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Summary: Prints an log message to the Godot console with the specified
  file, function, line number, and error message string with args.

  ref : godot_cpp/core/print_string.hpp
  -----------------------------------------------------------------F-F*/
	godot::String line_str = godot::String("[") + file + "@" + func + ":" + godot::itos(line) + "] " + godot::String(p_variant);
	godot::UtilityFunctions::print(line_str, p_args...);
}