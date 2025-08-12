#!/usr/bin/env python3
"""
Script to auto-generate raylib_api.gen.h from raylib.h
Extracts all RLAPI functions and creates the hot reload wrapper.
"""

import re
import sys
from pathlib import Path
from datetime import datetime

def camel_to_snake(name):
    """Convert CamelCase to snake_case"""
    # Handle special cases first
    name = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    name = re.sub('([a-z0-9])([A-Z])', r'\1_\2', name)
    return name.lower()

def parse_raylib_functions(raylib_h_path):
    """Parse raylib.h and extract all RLAPI function declarations"""
    functions = []
    
    with open(raylib_h_path, 'r') as f:
        content = f.read()
    
    # First, find all lines that start with RLAPI and end with );
    rlapi_pattern = rf'RLAPI[^;]+;'
    declarations = re.findall(rlapi_pattern, content, re.MULTILINE | re.DOTALL)
    
    for decl in declarations:
        # Clean up the declaration (remove newlines and extra spaces)
        decl = ' '.join(decl.split())
        
        # Find the function name and parameters
        # Function name is the word immediately before the opening parenthesis
        func_match = re.search(r'(\w+)\s*\(([^)]*)\)\s*;', decl)
        if not func_match:
            continue
            
        func_name = func_match.group(1)
        params = func_match.group(2).strip()
        
        # Extract return type (everything between RLAPI and function name)
        # Handle cases where there might not be whitespace before function name (e.g., "char *FuncName")
        return_type_match = re.search(rf'RLAPI\s+(.*?)\s*' + re.escape(func_name) + r'\s*\(', decl)
        if not return_type_match:
            continue
            
        return_type = return_type_match.group(1).strip()
        
        # Parse parameters
        param_list = []
        param_names = []
        
        if params and params != 'void':
            # Split parameters and clean them up
            for param in params.split(','):
                param = param.strip()
                if param:
                    param_list.append(param)
                    # Handle variadic parameters
                    if param == '...':
                        param_names.append('...')
                    else:
                        # Extract parameter name (last word, handling pointers)
                        param_parts = param.split()
                        if param_parts:
                            param_name = param_parts[-1]
                            # Remove array brackets and pointer stars from the name
                            param_name = re.sub(r'[\[\]*]', '', param_name)
                            param_names.append(param_name)
        
        functions.append({
            'name': func_name,
            'snake_name': camel_to_snake(func_name),
            'return_type': return_type,
            'params': param_list,
            'param_names': param_names,
            'params_str': params if params else 'void'
        })
    
    return functions

def parse_raymath_functions(raymath_h_path):
    """Parse raymath.h and extract all RMAPI inline function definitions"""
    functions = []
    
    with open(raymath_h_path, 'r') as f:
        content = f.read()
    
    # Pattern for inline function definitions (raymath.h style)
    inline_pattern = rf'RMAPI\s+[^{{;]+?\([^)]*\)\s*(?=\{{)'
    declarations = re.findall(inline_pattern, content, re.MULTILINE | re.DOTALL)
    
    for decl in declarations:
        # Clean up the declaration (remove newlines and extra spaces)
        decl = ' '.join(decl.split())
        
        # Find the function name and parameters
        func_match = re.search(r'(\w+)\s*\(([^)]*)\)\s*$', decl)
        if not func_match:
            continue
            
        func_name = func_match.group(1)
        params = func_match.group(2).strip()
        
        # Extract return type (everything between RMAPI and function name)
        return_type_match = re.search(rf'RMAPI\s+(.*?)\s*' + re.escape(func_name) + r'\s*\(', decl)
        if not return_type_match:
            continue
            
        return_type = return_type_match.group(1).strip()
        
        # Parse parameters
        param_list = []
        param_names = []
        
        if params and params != 'void':
            # Split parameters and clean them up
            for param in params.split(','):
                param = param.strip()
                if param:
                    param_list.append(param)
                    # Handle variadic parameters
                    if param == '...':
                        param_names.append('...')
                    else:
                        # Extract parameter name (last word, handling pointers)
                        param_parts = param.split()
                        if param_parts:
                            param_name = param_parts[-1]
                            # Remove array brackets and pointer stars from the name
                            param_name = re.sub(r'[\[\]*]', '', param_name)
                            param_names.append(param_name)
        
        functions.append({
            'name': func_name,
            'snake_name': camel_to_snake(func_name),
            'return_type': return_type,
            'params': param_list,
            'param_names': param_names,
            'params_str': params if params else 'void'
        })
    
    return functions

def generate_struct_members(functions):
    """Generate the RaylibAPI struct members"""
    lines = []
    for func in functions:
        # Create function pointer declaration
        params_str = ', '.join(func['params']) if func['params'] else 'void'
        line = f"    {func['return_type']} (*{func['snake_name']})({params_str});"
        lines.append(line)
    return '\n'.join(lines)

def generate_api_assignments(functions):
    """Generate the function pointer assignments in create_raylib_api"""
    lines = []
    for func in functions:
        line = f"        .{func['snake_name']} = {func['name']},"
        lines.append(line)
    return '\n'.join(lines)

def generate_macros(functions):
    """Generate the macro definitions for hot reload"""
    lines = []
    for func in functions:
        if func['param_names']:
            # Check if this is a variadic function
            if any('...' in param for param in func['params']):
                # For variadic functions, use __VA_ARGS__
                non_variadic_params = [name for name in func['param_names'] if name != '...']
                if non_variadic_params:
                    params_def = ', '.join(non_variadic_params) + ', ...'
                    params_call = ', '.join(non_variadic_params) + ', __VA_ARGS__'
                else:
                    params_def = '...'
                    params_call = '__VA_ARGS__'
                line = f"#define {func['name']}({params_def}) rl->{func['snake_name']}({params_call})"
            else:
                params_call = ', '.join(func['param_names'])
                line = f"#define {func['name']}({', '.join(func['param_names'])}) rl->{func['snake_name']}({params_call})"
        else:
            line = f"#define {func['name']}() rl->{func['snake_name']}()"
        lines.append(line)
    return '\n'.join(lines)

def generate_raylib_api_h(functions):
    """Generate the complete raylib_api.h file content"""
    
    struct_members = generate_struct_members(functions)
    api_assignments = generate_api_assignments(functions)
    macros = generate_macros(functions)
    
    content = f'''#ifndef RAYLIB_API_GEN_H
#define RAYLIB_API_GEN_H

// DO NOT EDIT THIS FILE, CHANGES WILL BE LOST
// Auto-generated by generate_raylib_api.py from raylib.h
// Timestamp: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}

#include "raylib.h"
#include "raymath.h"

typedef struct {{
{struct_members}
}} RaylibAPI;

// Global API pointer (set by main executable)
extern RaylibAPI* rl;

static inline RaylibAPI* create_raylib_api(void) {{
    static RaylibAPI api = {{
{api_assignments}
    }};
    return &api;
}}

#ifdef HOT_RELOAD
{macros}
#endif

#endif // RAYLIB_API_GEN_H
'''
    
    return content

def main():
    if len(sys.argv) > 1:
        raylib_h_path = sys.argv[1]
        raymath_h_path = sys.argv[2]
    else:
        raylib_h_path = "deps/raylib/src/raylib.h"
        raymath_h_path = "deps/raylib/src/raymath.h"
    
    if not Path(raylib_h_path).exists():
        print(f"Error: {raylib_h_path} not found")
        sys.exit(1)
    
    print(f"Parsing {raylib_h_path}...")
    functions = parse_raylib_functions(raylib_h_path)
    print(f"Found {len(functions)} RLAPI functions")
    
    print(f"Parsing {raymath_h_path}...")
    math_functions = parse_raymath_functions(raymath_h_path)
    print(f"Found {len(math_functions)} RMAPI functions")
    functions += math_functions

    header_content = generate_raylib_api_h(functions)

    output_path = "src/hot_reload/raylib_api.gen.h"
    with open(output_path, 'w') as f:
        f.write(header_content)
    
    print(f"Generated {output_path} with {len(functions)} functions")

    print("\nExtracted functions:")
    for i, func in enumerate(functions[:5]):
        print(f"  {func['name']} -> {func['snake_name']}")
    if len(functions) > 5:
        print(f"  ... and {len(functions) - 5} more")

if __name__ == "__main__":
    main() 