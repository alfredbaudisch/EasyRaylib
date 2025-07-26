/*
This file is the starting point of your game.

Some important procedures are:
- game_init_window: Opens the window
- game_init: Sets up the game state
- game_update: Run once per frame
- game_should_close: For stopping your game when close button is pressed
- game_shutdown: Shuts down game and frees memory
- game_shutdown_window: Closes window

The procs above are used regardless if you compile using the `build_release`
script or the `build_hot_reload` script. However, in the hot reload case, the
contents of this file is compiled as part of `build/hot_reload/game.dll` (or
.dylib/.so on mac/linux). In the hot reload cases some other procedures are
also used in order to facilitate the hot reload functionality:

- game_memory: Run just before a hot reload. That way game_hot_reload.exe has a
      pointer to the game's memory that it can hand to the new game DLL.
- game_hot_reloaded: Run after a hot reload so that the `g_mem` global
      variable can be set to whatever pointer it was in the old DLL.

NOTE: When compiled as part of `build_release`, `build_debug` or `build_web`
then this whole package is just treated as a normal Odin package. No DLL is
created.
*/

package game

import "core:fmt"
import "core:math/linalg"
import rl "vendor:raylib"

PIXEL_WINDOW_HEIGHT :: 180
AUTO_RELOAD :: true

Game_Memory :: struct {
	player_pos: rl.Vector2,
	player_texture: rl.Texture,
	some_number: int,
	run: bool,
	auto_reload: bool,
	window_manager: Window_Manager,
}

g_mem: ^Game_Memory

game_camera :: proc() -> rl.Camera2D {
	w := f32(rl.GetScreenWidth())
	h := f32(rl.GetScreenHeight())

	return {
		zoom = h/PIXEL_WINDOW_HEIGHT,
		target = g_mem.player_pos,
		offset = { w/2, h/2 },
	}
}

ui_camera :: proc() -> rl.Camera2D {
	return {
		zoom = f32(rl.GetScreenHeight())/PIXEL_WINDOW_HEIGHT,
	}
}

is_mouse_over_ui :: proc() -> bool {
	mouse_pos := rl.GetMousePosition()
	
	// Transform mouse position to game camera space
	camera := game_camera()
	mouse_world_pos := rl.GetScreenToWorld2D(mouse_pos, camera)
	
	// Check if mouse is over any game objects or UI elements
	if is_point_inside_rect(mouse_world_pos, {20, 20}, {10, 10}) {
		return true
	}
	
	if is_point_inside_rect(mouse_world_pos, {-30, -20}, {10, 10}) {
		return true
	}
	
	// Check if mouse is over the player texture
	player_size := rl.Vector2{f32(g_mem.player_texture.width), f32(g_mem.player_texture.height)}
	if is_point_inside_rect(mouse_world_pos, g_mem.player_pos, player_size) {
		return true
	}
	
	// Check if mouse is over text UI (using ui_camera space)
	ui_camera_pos := rl.GetScreenToWorld2D(mouse_pos, ui_camera())
	text_pos := rl.Vector2{5, 5}
	text_size := rl.Vector2{150, 30} // Approximate size for the info text
	if is_point_inside_rect(ui_camera_pos, text_pos, text_size) {
		return true
	}
	
	return false
}

update :: proc() {
	input: rl.Vector2

	if rl.IsKeyDown(.UP) || rl.IsKeyDown(.W) {
		input.y -= 1
	}
	if rl.IsKeyDown(.DOWN) || rl.IsKeyDown(.S) {
		input.y += 1
	}
	if rl.IsKeyDown(.LEFT) || rl.IsKeyDown(.A) {
		input.x -= 1
	}
	if rl.IsKeyDown(.RIGHT) || rl.IsKeyDown(.D) {
		input.x += 1
	}

	input = linalg.normalize0(input)
	g_mem.player_pos += input * rl.GetFrameTime() * 100
	g_mem.some_number += 1

	if rl.IsKeyPressed(.ESCAPE) {
		g_mem.run = false
	}

	update_window_manager(&g_mem.window_manager, is_mouse_over_ui())
}

draw :: proc() {
	rl.BeginDrawing()
	// Use a semi-transparent background instead of solid black
	rl.ClearBackground(rl.Color{0, 0, 0, 0})

	rl.BeginMode2D(game_camera())
	rl.DrawTextureEx(g_mem.player_texture, g_mem.player_pos, 0, 1, rl.WHITE)
	// Use semi-transparent colors for better effect on transparent window
	rl.DrawRectangleV({20, 20}, {10, 10}, rl.Color{255, 0, 0, 255})
	rl.DrawRectangleV({-30, -20}, {10, 10}, rl.Color{0, 255, 0, 255})
	rl.EndMode2D()

	ui_camera := ui_camera()
	rl.BeginMode2D(ui_camera)

	// Draw a rectanble around the area of the whole window
	adjusted_width := f32(rl.GetScreenWidth()) / ui_camera.zoom
	rl.DrawRectangleLines(0, 0, i32(adjusted_width), PIXEL_WINDOW_HEIGHT, rl.WHITE)
	
	// NOTE: `fmt.ctprintf` uses the temp allocator. The temp allocator is
	// cleared at the end of the frame by the main application, meaning inside
	// `main_hot_reload.odin`, `main_release.odin` or `main_web_entry.odin`.
	rl.DrawText(fmt.ctprintf(
		"some_number: %v\n" +
		"player_pos = %v\n",
		g_mem.some_number, 
		g_mem.player_pos), 5, 5, 8, rl.WHITE)

	rl.EndMode2D()

	rl.EndDrawing()
}

@(export)
game_update :: proc() {
	when ODIN_DEBUG {
		dev_update()
	}

	update()
	draw()
}

@(export)
game_init_window :: proc() {
	init_transparent_window(1280, 720, "Odin + Raylib + Hot Reload template", 200, 200)
}

@(export)
game_init :: proc() {
	g_mem = new(Game_Memory)

	g_mem^ = Game_Memory {
		run = true,
		some_number = 100,

		// You can put textures, sounds and music in the `assets` folder. Those
		// files will be part any release or web build.
		player_texture = rl.LoadTexture("assets/round_cat.png"),

		auto_reload = AUTO_RELOAD,
		window_manager = init_window_manager(),
	}

	game_hot_reloaded(g_mem)
}

@(export)
game_should_run :: proc() -> bool {
	when ODIN_OS != .JS {
		// Never run this proc in browser. It contains a 16 ms sleep on web!
		if rl.WindowShouldClose() {
			return false
		}
	}

	return g_mem.run
}

@(export)
game_shutdown :: proc() {
	free(g_mem)
}

@(export)
game_shutdown_window :: proc() {
	rl.CloseWindow()
}

@(export)
game_memory :: proc() -> rawptr {
	return g_mem
}

@(export)
game_memory_size :: proc() -> int {
	return size_of(Game_Memory)
}

@(export)
game_hot_reloaded :: proc(mem: rawptr) {
	g_mem = (^Game_Memory)(mem)

	// Here you can also set your own global variables. A good idea is to make
	// your global variables into pointers that point to something inside
	// `g_mem`.
}

@(export)
game_force_reload :: proc() -> bool {
	return rl.IsKeyPressed(.F5)
}

@(export)
game_force_restart :: proc() -> bool {
	return rl.IsKeyPressed(.F6)
}

// In a web build, this is called when browser changes size. Remove the
// `rl.SetWindowSize` call if you don't want a resizable game.
game_parent_window_size_changed :: proc(w, h: int) {
	rl.SetWindowSize(i32(w), i32(h))
}
