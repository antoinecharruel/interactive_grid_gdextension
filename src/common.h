/*+===================================================================
File: common.cpp

Summary: This file provides utility functions and common includes 
         used across the InteractiveGrid Godot GDExtension project.

Last Modified: October 11, 2025

This file is part of the InteractiveGrid GDExtension Source Code.
Repository: https://github.com/antoinecharruel/interactive_grid

Version InteractiveGrid: 1.0.0
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

#include <array>
#include <sstream>
#include <vector>

// GODOT.
#include <godot_cpp/classes/a_star2d.hpp>
#include <godot_cpp/classes/area3d.hpp>
#include <godot_cpp/classes/box_mesh.hpp>
#include <godot_cpp/classes/box_shape3d.hpp>
#include <godot_cpp/classes/character_body3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/cylinder_mesh.hpp>
#include <godot_cpp/classes/cylinder_shape3d.hpp>
#include <godot_cpp/classes/geometry_instance3d.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/multi_mesh.hpp>
#include <godot_cpp/classes/multi_mesh_instance3d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/physics_direct_space_state3d.hpp>
#include <godot_cpp/classes/physics_ray_query_parameters3d.hpp>
#include <godot_cpp/classes/physics_server3d.hpp>
#include <godot_cpp/classes/physics_shape_query_parameters3d.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/type_info.hpp>
#include <godot_cpp/variant/callable.hpp>

inline bool PrintError(const godot::String &file, const godot::String &func,
		const int &line, const godot::String &str) {
	/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    Summary: Prints an error message to the Godot console with the specified
  file, function, line number, and error message string. This function is useful
  for debugging and logging errors in Godot scripts.

                     The format of the error message is:
                     [file@function:line] error_message

                     Example usage:
                     PrintError(__FILE__, __FUNCTION__, __LINE__, "An error
  occurred");

    Last Modified: September 21, 2025
  -----------------------------------------------------------------F-F*/
	godot::print_error(godot::String("[") + file + "@" + func + ":" +
			godot::itos(line) + "] " + str);
	return false;
}

inline bool PrintLine(const godot::String &file, const godot::String &func,
		const int &line, const godot::String &str) {
	/*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    Summary: Prints an log message to the Godot console with the specified file,
                     function, line number, and log message string. This
  function is useful for debugging and logging in Godot scripts.

                     The format of the log message is:
                     [file@function:line] error_message

                     Example usage:
                     PrintLine(__FILE__, __FUNCTION__, __LINE__, "log");

    Last Modified: September 20, 2025
  -----------------------------------------------------------------F-F*/
	godot::print_line(godot::String("[") + file + "@" + func + ":" +
			godot::itos(line) + "] " + str);
	return false;
}