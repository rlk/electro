--    Copyright (C) 2005 Robert Kooima
--
--    ELECTRO is free software;  you can redistribute it and/or modify it
--    under the terms of the  GNU General Public License  as published by
--    the  Free Software Foundation;  either version 2 of the License, or
--    (at your option) any later version.
--
--    This program is distributed in the hope that it will be useful, but
--    WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
--    MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
--    General Public License for more details.

-------------------------------------------------------------------------------
-- Global variables, game state, and scene graph entities:

state      = { }
player     = { }
overlay    = { }
viewport   = { }

curr_state = nil
curr_speed = 0
curr_score = 0
curr_level = 0
curr_ships = 0
high_score = 0
free_score = 0

scene_2d   = nil
scene_3d   = nil

-------------------------------------------------------------------------------
-- The extent of the viewport must be known in order to adapt the game layout
-- to any arbitrary display size.

function init_viewport()
    viewport.x, viewport.y, viewport.w, viewport.h = E.get_viewport()

    viewport.a = viewport.h / viewport.w
    viewport.k =     1024.0 / viewport.w
    viewport.T =     -512.0 * viewport.a
    viewport.B =      512.0 * viewport.a
    viewport.L =     -512.0
    viewport.R =      512.0
end

function init_scene()
    local camera_2d = E.create_camera(E.camera_type_orthogonal)
    local camera_3d = E.create_camera(E.camera_type_orthogonal)

    scene_2d = E.create_pivot()
    scene_3d = E.create_pivot()

    E.parent_entity(scene_2d, camera_2d)
    E.parent_entity(scene_3d, camera_3d)

    -- Move the origin to the center of the screen.

    E.set_entity_position(camera_2d, -viewport.w / 2, -viewport.h / 2, 0)
    E.set_entity_position(camera_3d, -viewport.w / 2, -viewport.h / 2, 0)

    -- Scale the scene to match the resolution of the display.

    E.set_entity_scale(scene_2d, viewport.k, viewport.k, viewport.k)
    E.set_entity_scale(scene_3d, viewport.k, viewport.k, viewport.k)
end

-------------------------------------------------------------------------------
-- Overlays are 2D image elements that lie in a plane before the 3D scene.

function create_overlay(filename)
    sprite = E.create_sprite(filename)

    E.set_entity_flag(sprite, E.entity_flag_hidden, true);
    E.parent_entity  (sprite, scene_2d)

    return sprite
end

function init_overlay()
    overlay.init1 = create_overlay("alpha.png")
    overlay.init2 = create_overlay("alpha.png")
    overlay.init3 = create_overlay("alpha.png")
    overlay.title = create_overlay("title.png")
    overlay.ready = create_overlay("ready.png")
    overlay.level = create_overlay("digit.png")
    overlay.clear = create_overlay("clear.png")
    overlay.high  = create_overlay("high.png")
    overlay.over  = create_overlay("over.png")

    E.set_entity_scale(overlay.level, 0.25, 0.25, 0.25);
    E.set_entity_scale(overlay.init1, 0.01, 0.01, 0.01);
    E.set_entity_scale(overlay.init2, 0.01, 0.01, 0.01);
    E.set_entity_scale(overlay.init3, 0.01, 0.01, 0.01);
end

function set_overlay_digit(sprite, n)
    if     n == 0 then E.set_sprite_bounds(sprite, 0.00, 0.25, 0.75, 1.00)
    elseif n == 1 then E.set_sprite_bounds(sprite, 0.25, 0.50, 0.75, 1.00)
    elseif n == 2 then E.set_sprite_bounds(sprite, 0.50, 0.75, 0.75, 1.00)
    elseif n == 3 then E.set_sprite_bounds(sprite, 0.75, 1.00, 0.75, 1.00)
    elseif n == 4 then E.set_sprite_bounds(sprite, 0.00, 0.25, 0.50, 0.75)
    elseif n == 5 then E.set_sprite_bounds(sprite, 0.25, 0.50, 0.50, 0.75)
    elseif n == 6 then E.set_sprite_bounds(sprite, 0.50, 0.75, 0.50, 0.75)
    elseif n == 7 then E.set_sprite_bounds(sprite, 0.75, 1.00, 0.50, 0.75)
    elseif n == 8 then E.set_sprite_bounds(sprite, 0.00, 0.25, 0.25, 0.50)
    elseif n == 9 then E.set_sprite_bounds(sprite, 0.25, 0.50, 0.25, 0.50)
    end
end

function set_overlay_alpha(sprite, c)
    if     c == "A" then E.set_sprite_bounds(sprite, 0.000, 0.125, 0.75, 1.00)
    elseif c == "B" then E.set_sprite_bounds(sprite, 0.125, 0.250, 0.75, 1.00)
    elseif c == "C" then E.set_sprite_bounds(sprite, 0.250, 0.375, 0.75, 1.00)
    elseif c == "D" then E.set_sprite_bounds(sprite, 0.375, 0.500, 0.75, 1.00)
    elseif c == "E" then E.set_sprite_bounds(sprite, 0.500, 0.625, 0.75, 1.00)
    elseif c == "F" then E.set_sprite_bounds(sprite, 0.625, 0.750, 0.75, 1.00)
    elseif c == "G" then E.set_sprite_bounds(sprite, 0.750, 0.875, 0.75, 1.00)
    elseif c == "H" then E.set_sprite_bounds(sprite, 0.875, 1.000, 0.75, 1.00)
    elseif c == "I" then E.set_sprite_bounds(sprite, 0.000, 0.125, 0.50, 0.75)
    elseif c == "J" then E.set_sprite_bounds(sprite, 0.125, 0.250, 0.50, 0.75)
    elseif c == "K" then E.set_sprite_bounds(sprite, 0.250, 0.375, 0.50, 0.75)
    elseif c == "L" then E.set_sprite_bounds(sprite, 0.375, 0.500, 0.50, 0.75)
    elseif c == "M" then E.set_sprite_bounds(sprite, 0.500, 0.625, 0.50, 0.75)
    elseif c == "N" then E.set_sprite_bounds(sprite, 0.625, 0.750, 0.50, 0.75)
    elseif c == "O" then E.set_sprite_bounds(sprite, 0.750, 0.875, 0.50, 0.75)
    elseif c == "P" then E.set_sprite_bounds(sprite, 0.875, 1.000, 0.50, 0.75)
    elseif c == "Q" then E.set_sprite_bounds(sprite, 0.000, 0.125, 0.25, 0.50)
    elseif c == "R" then E.set_sprite_bounds(sprite, 0.125, 0.250, 0.25, 0.50)
    elseif c == "S" then E.set_sprite_bounds(sprite, 0.250, 0.375, 0.25, 0.50)
    elseif c == "T" then E.set_sprite_bounds(sprite, 0.375, 0.500, 0.25, 0.50)
    elseif c == "U" then E.set_sprite_bounds(sprite, 0.500, 0.625, 0.25, 0.50)
    elseif c == "V" then E.set_sprite_bounds(sprite, 0.625, 0.750, 0.25, 0.50)
    elseif c == "W" then E.set_sprite_bounds(sprite, 0.750, 0.875, 0.25, 0.50)
    elseif c == "X" then E.set_sprite_bounds(sprite, 0.875, 1.000, 0.25, 0.50)
    elseif c == "Y" then E.set_sprite_bounds(sprite, 0.000, 0.125, 0.00, 0.25)
    elseif c == "Z" then E.set_sprite_bounds(sprite, 0.125, 0.250, 0.00, 0.25)
    end
end

-------------------------------------------------------------------------------
-- The null state bootstraps the game, ensuring that title.enter is called.

state.null = { }

function state.null.timer()
    return state.title
end

-------------------------------------------------------------------------------
-- The title state displays the title screen and attract mode.

state.title = { }

function state.title.enter()
    E.set_entity_flag(overlay.title, E.entity_flag_hidden, false)
    return state.title
end

function state.title.leave()
    E.set_entity_flag(overlay.title, E.entity_flag_hidden, true)
    return state.title
end

function state.title.timer(t, dt)
    return state.title
end

function state.title.button_A(s)
    if s then
        return state.ready
    else
        return state.title
    end
end

function state.title.button_B(s)
    return state.title
end

-------------------------------------------------------------------------------
-- The ready state is the pause before a level begins.

state.ready = { }

function state.ready.enter()
    E.set_entity_flag(overlay.ready, E.entity_flag_hidden, false)
    E.set_entity_flag(overlay.level, E.entity_flag_hidden, false)
    set_overlay_digit(overlay.level, curr_level)
    return state.ready
end

function state.ready.leave()
    E.set_entity_flag(overlay.ready, E.entity_flag_hidden, true)
    E.set_entity_flag(overlay.level, E.entity_flag_hidden, true)
    return state.ready
end

function state.ready.timer(t, dt)
    return state.ready
end

function state.ready.button_A(s)
    if s then
        return state.title
    else
        return state.ready
    end
end

function state.ready.button_B(s)
    return state.ready
end

-------------------------------------------------------------------------------

function do_state(next_state)
    if not (next_state == curr_state) then
        if curr_state and curr_state.leave then
            curr_state.leave()
        end

        curr_state = next_state

        if curr_state and curr_state.enter then
            curr_state.enter()
        end
    end
end

-------------------------------------------------------------------------------
-- These are the Electro callbacks.  They distribute user events to the current
-- state's event handlers.

function do_start()
    curr_state = state.null

    init_viewport()
    init_scene()
    init_overlay()

    E.enable_timer(true);
end

function do_timer(dt)
    if dt > 0 and curr_state and curr_state.timer then
        do_state(curr_state.timer(global_t, dt))
        return true
    else
        return false
    end
end

function do_joystick(n, b, s)
    if n == 0 then
        if b == 0 and curr_state and curr_state.button_A then
            do_state(curr_state.button_A(s))
            return true
        end
        if b == 1 and curr_state and curr_state.button_B then
            do_state(curr_state.button_B(s))
            return true
        end
    end
    return false
end

function do_keyboard(k, s)
    if k ==  32 and curr_state and curr_state.button_A then
        do_state(curr_state.button_A(s))
        return true
    end
    if k == 273 and curr_state and curr_state.button_B then
        do_state(curr_state.button_B(s))
        return true
    end

    if k == 275 then
        player.turning_L = s
    end
    if k == 276 then
        player.turning_R = s
    end

    return false
end

-------------------------------------------------------------------------------
