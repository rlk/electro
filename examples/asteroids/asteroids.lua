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

joy_dev = 0

sound_on = true
music_on = true
stars_on = true

-------------------------------------------------------------------------------
-- Global variables, game state, and scene graph entities:

high_score_file = "asteroids.dat"

state      = { }
sound      = { }
viewport   = { }
overlay    = { }
player     = { }
entity     = { }
scores     = { }
bullets    = { }
shockwaves = { }
explosions = { }
asteroids  = { }
initials   = { }

scene_2d   = nil
scene_3d   = nil

curr_state = nil
time_state = 0

curr_ships = 0
curr_speed = 0
curr_level = 0
curr_score = 0
curr_count = 0
free_score = 0
high_score = 0

space      = nil
space_y    = 0
space_z    = 0

-------------------------------------------------------------------------------
-- Miscellaneous handy utility functions...

function hide(entity)
    E.set_entity_flags(entity, E.entity_flag_hidden, true)

    return entity
end

function next_letter(letter)
    local byte = string.byte(letter)

    if byte >= 90 then
        return "A"
    else
        return string.char(byte + 1)
    end
end

function prev_letter(letter)
    local byte = string.byte(letter)

    if byte <= 65 then
        return "Z"
    else
        return string.char(byte - 1)
    end
end

-------------------------------------------------------------------------------
-- The extent of the viewport must be known in order to adapt the game layout
-- to any arbitrary display size.

function init_viewport()
    viewport.x, viewport.y, viewport.w, viewport.h = E.get_display_union()

    local width = 512 + 384 * viewport.w / viewport.h

    viewport.a = viewport.h / viewport.w
    viewport.k = viewport.w / width

    viewport.h =  width * viewport.a
    viewport.B = -width * viewport.a / 2
    viewport.T =  width * viewport.a / 2

    viewport.w =  width 
    viewport.L = -width / 2
    viewport.R =  width / 2
end

function make_sprite(filename)
    local image = E.create_image(filename)
    local brush = E.create_brush()

    E.set_brush_flags(brush, E.brush_flag_transparent, true)
    E.set_brush_flags(brush, E.brush_flag_unlit,       true)
    E.set_brush_color(brush, 1.0, 1.0, 1.0, 1.0)
    E.set_brush_image(brush, image)

    return E.create_sprite(brush)
end

function init_scene()
    local camera_2d = E.create_camera(E.camera_type_orthogonal)
    local camera_3d = E.create_camera(E.camera_type_orthogonal)
    local light_L   = E.create_light (E.light_type_positional)
    local light_R   = E.create_light (E.light_type_positional)
    local galaxy    = nil

    space    = E.create_camera(E.camera_type_perspective)
    scene_2d = E.create_pivot()
    scene_3d = E.create_pivot()

    if stars_on then
        local brush = E.create_brush()
        
        E.set_brush_frag_prog(brush, "../data/star.fp")
        E.set_brush_vert_prog(brush, "../data/star.vp")
        galaxy = E.create_galaxy("../data/galaxy_hip.gal", brush)
    end

    E.parent_entity(scene_2d, camera_2d)
    E.parent_entity(light_L,  camera_3d)
    E.parent_entity(light_R,  light_L)
    E.parent_entity(scene_3d, light_R)
    E.parent_entity(galaxy,   space)

    -- Move the origin to the center of the screen.

    E.set_entity_position(camera_2d, -viewport.k * viewport.w / 2,
                                     -viewport.k * viewport.h / 2, 0)
    E.set_entity_position(camera_3d, -viewport.k * viewport.w / 2,
                                     -viewport.k * viewport.h / 2, 0)

    -- Scale the scene to match the resolution of the display.

    E.set_entity_scale(scene_2d, viewport.k, viewport.k, viewport.k)
    E.set_entity_scale(scene_3d, viewport.k, viewport.k, viewport.k)

    -- Configure the background stars.

    if galaxy then
       E.set_galaxy_magnitude(galaxy, 100.0 * viewport.k)
    end

    -- Configure the lights.

    E.set_entity_position(light_L, -viewport.w / 2, 0, viewport.h / 2)
    E.set_entity_position(light_R,  viewport.w / 2, 0, viewport.h / 2)

    E.set_light_color(light_L, 1.0, 0.8, 0.5)
    E.set_light_color(light_R, 0.5, 0.8, 1.0)
end

function step_scene(dt)
    space_y = space_y + dt
    space_z = space_z + dt * 2

    if space_y > 180 then space_y = space_y - 360 end
    if space_z > 180 then space_z = space_z - 360 end

    E.set_entity_rotation(space, 0, space_y, space_z)
end

-------------------------------------------------------------------------------
-- Overlays are 2D image elements that lie in a plane before the 3D scene.

function create_overlay(filename, scale, hidden)
    sprite = make_sprite(filename)

    -- Add the new overlay sprite to the scene, hidden if requested.

    E.parent_entity   (sprite, scene_2d)
    E.set_entity_flags(sprite, E.entity_flag_hidden, hidden)

    -- Marginal hack: the aspect ratio of the alphabet sprite is 2:1.

    if filename == "alpha.png" then
        E.set_entity_scale(sprite, scale / 2, scale, scale)
    else
        E.set_entity_scale(sprite, scale,     scale, scale)
    end

    return sprite
end

function init_overlay()
    local w = viewport.w

    -- Create all game state text overlays.

    overlay.title = create_overlay("title.png", 1.00, true)
    overlay.ready = create_overlay("ready.png", 1.00, true)
    overlay.level = create_overlay("digit.png", 0.25, true)
    overlay.clear = create_overlay("clear.png", 1.00, true)
    overlay.high  = create_overlay("high.png",  1.00, true)
    overlay.over  = create_overlay("over.png",  1.00, true)

    overlay.high_inits = { }
    overlay.high_score = { }
    overlay.curr_score = { }
    overlay.curr_ships = { }

    -- Create and position the high score player's initials display.

    for i = 1, 3 do
        overlay.high_inits[i] = create_overlay("alpha.png", 0.125, false)

        E.set_entity_position(overlay.high_inits[i], viewport.L + 48 * i + 200,
                                                     viewport.T - 32, 0)
    end

    -- Create and position the score, high score, and spare ships displays.

    for i = 1, 5 do
        overlay.high_score[i] = create_overlay("digit.png", 0.125, false)
        overlay.curr_score[i] = create_overlay("digit.png", 0.125, false)
        overlay.curr_ships[i] = create_ship(0, 0, 0)

        E.set_entity_position(overlay.curr_score[i], viewport.R - 40 * i,
                                                     viewport.T - 32, 0)
        E.set_entity_position(overlay.high_score[i], viewport.L - 40 * i + 230,
                                                     viewport.T - 32, 0)
        E.set_entity_position(overlay.curr_ships[i], viewport.R - 48 * i,
                                                     viewport.B + 32, 0)

        E.set_entity_flags(overlay.curr_ships[i], E.entity_flag_hidden, true)
    end
end

-------------------------------------------------------------------------------

overlay_digit_map = {
    [0] = { 0.00, 0.25, 0.75, 1.00 },
    [1] = { 0.25, 0.50, 0.75, 1.00 },
    [2] = { 0.50, 0.75, 0.75, 1.00 },
    [3] = { 0.75, 1.00, 0.75, 1.00 },
    [4] = { 0.00, 0.25, 0.50, 0.75 },
    [5] = { 0.25, 0.50, 0.50, 0.75 },
    [6] = { 0.50, 0.75, 0.50, 0.75 },
    [7] = { 0.75, 1.00, 0.50, 0.75 },
    [8] = { 0.00, 0.25, 0.25, 0.50 },
    [9] = { 0.25, 0.50, 0.25, 0.50 }
}

overlay_alpha_map = {
    ["A"] = { 0.000, 0.125, 0.75, 1.00 },
    ["B"] = { 0.125, 0.250, 0.75, 1.00 },
    ["C"] = { 0.250, 0.375, 0.75, 1.00 },
    ["D"] = { 0.375, 0.500, 0.75, 1.00 },
    ["E"] = { 0.500, 0.625, 0.75, 1.00 },
    ["F"] = { 0.625, 0.750, 0.75, 1.00 },
    ["G"] = { 0.750, 0.875, 0.75, 1.00 },
    ["H"] = { 0.875, 1.000, 0.75, 1.00 },
    ["I"] = { 0.000, 0.125, 0.50, 0.75 },
    ["J"] = { 0.125, 0.250, 0.50, 0.75 },
    ["K"] = { 0.250, 0.375, 0.50, 0.75 },
    ["L"] = { 0.375, 0.500, 0.50, 0.75 },
    ["M"] = { 0.500, 0.625, 0.50, 0.75 },
    ["N"] = { 0.625, 0.750, 0.50, 0.75 },
    ["O"] = { 0.750, 0.875, 0.50, 0.75 },
    ["P"] = { 0.875, 1.000, 0.50, 0.75 },
    ["Q"] = { 0.000, 0.125, 0.25, 0.50 },
    ["R"] = { 0.125, 0.250, 0.25, 0.50 },
    ["S"] = { 0.250, 0.375, 0.25, 0.50 },
    ["T"] = { 0.375, 0.500, 0.25, 0.50 },
    ["U"] = { 0.500, 0.625, 0.25, 0.50 },
    ["V"] = { 0.625, 0.750, 0.25, 0.50 },
    ["W"] = { 0.750, 0.875, 0.25, 0.50 },
    ["X"] = { 0.875, 1.000, 0.25, 0.50 },
    ["Y"] = { 0.000, 0.125, 0.00, 0.25 },
    ["Z"] = { 0.125, 0.250, 0.00, 0.25 }
}

function set_overlay_digit(sprite, n)
    E.set_sprite_range(sprite, overlay_digit_map[n][1],
                               overlay_digit_map[n][2],
                               overlay_digit_map[n][3],
                               overlay_digit_map[n][4])
end

function set_overlay_alpha(sprite, c)
    E.set_sprite_range(sprite, overlay_alpha_map[c][1],
                               overlay_alpha_map[c][2],
                               overlay_alpha_map[c][3],
                               overlay_alpha_map[c][4])
end

function set_overlay_curr_score(n)
    local d1 = math.mod(n,                     10)
    local d2 = math.mod(math.floor(n / 10),    10)
    local d3 = math.mod(math.floor(n / 100),   10)
    local d4 = math.mod(math.floor(n / 1000),  10)
    local d5 = math.mod(math.floor(n / 10000), 10)

    set_overlay_digit(overlay.curr_score[1], d1)
    set_overlay_digit(overlay.curr_score[2], d2)
    set_overlay_digit(overlay.curr_score[3], d3)
    set_overlay_digit(overlay.curr_score[4], d4)
    set_overlay_digit(overlay.curr_score[5], d5)
end

function set_overlay_high_score(n)
    local d1 = math.mod(n,                     10)
    local d2 = math.mod(math.floor(n / 10),    10)
    local d3 = math.mod(math.floor(n / 100),   10)
    local d4 = math.mod(math.floor(n / 1000),  10)
    local d5 = math.mod(math.floor(n / 10000), 10)

    set_overlay_digit(overlay.high_score[1], d1)
    set_overlay_digit(overlay.high_score[2], d2)
    set_overlay_digit(overlay.high_score[3], d3)
    set_overlay_digit(overlay.high_score[4], d4)
    set_overlay_digit(overlay.high_score[5], d5)
end

function set_overlay_curr_ships(n)
    for i = 1, 5 do
        E.set_entity_flags(overlay.curr_ships[i], E.entity_flag_hidden, i > n)
    end
end

-------------------------------------------------------------------------------

function init_entity()
    entity.ship           = hide(E.create_object("ship.obj"))
    entity.rock           = hide(E.create_object("rock.obj"))

    entity.thrust         = hide(make_sprite("thrust.png"))
    entity.bullet         = hide(make_sprite("bullet.png"))
    entity.score05        = hide(make_sprite("score05.png"))
    entity.score10        = hide(make_sprite("score10.png"))
    entity.score25        = hide(make_sprite("score25.png"))
    entity.ship_explosion = hide(make_sprite("ship_explosion.png"))
    entity.ship_shockwave = hide(make_sprite("ship_shockwave.png"))
    entity.rock_explosion = hide(make_sprite("rock_explosion.png"))
    entity.rock_shockwave = hide(make_sprite("rock_shockwave.png"))
end

function create_ship(x, y, z)
    local object = E.create_clone(entity.ship)
    local s = 16.0

    E.parent_entity      (object, scene_3d)
    E.set_entity_scale   (object, s, s, s)
    E.set_entity_position(object, x, y, z)

    return object
end

function create_rock(size, x, y, z)
    local object = E.create_clone(entity.rock)
    local s = 20.0 * size / 3.0

    E.parent_entity      (object, scene_3d)
    E.set_entity_scale   (object, s, s, s)
    E.set_entity_position(object, x, y, z)

    return object
end

function create_bullet(x, y, z)
    local object = E.create_clone(entity.bullet)
    local s = 0.30

    E.parent_entity      (object, scene_3d)
    E.set_entity_position(object, x, y, z)
    E.set_entity_scale   (object, s, s, s)

    return object
end

function create_explosion(source, size, x, y, z)
    local object = E.create_clone(source)
    local s = 1.0 * size / 3.0

    E.parent_entity      (object, scene_3d)
    E.set_entity_position(object, x, y, z)
    E.set_entity_scale   (object, s, s, s)

    return object
end

function create_shockwave(source, size, x, y, z)
    local object = E.create_clone(source)
    local s = 0.25 * size / 3.0

    E.parent_entity      (object, scene_3d)
    E.set_entity_position(object, x, y, z)
    E.set_entity_scale   (object, s, s, s)

    return object
end

function create_score(value, x, y, z)
    local object
    local s = 0.25

    if value ==  5 then object = E.create_clone(entity.score05) end
    if value == 10 then object = E.create_clone(entity.score10) end
    if value == 25 then object = E.create_clone(entity.score25) end

    E.parent_entity      (object, scene_3d)
    E.set_entity_position(object, x, y, z)
    E.set_entity_scale   (object, s, s, s)

    return object
end

-------------------------------------------------------------------------------

function init_sound()
    if sound_on then
        sound.bullet          = E.load_sound("bullet.ogg")
        sound.free            = E.load_sound("free.ogg")
        sound.rock1_explosion = E.load_sound("rock1_explosion.ogg")
        sound.rock2_explosion = E.load_sound("rock2_explosion.ogg")
        sound.rock3_explosion = E.load_sound("rock3_explosion.ogg")
        sound.ship_explosion  = E.load_sound("ship_explosion.ogg")
        sound.thrust1         = E.load_sound("thrust1.ogg")
        sound.thrust2         = E.load_sound("thrust2.ogg")
        sound.thrust3         = E.load_sound("thrust3.ogg")
    end

    if music_on then
        music = E.load_sound("inter.ogg")
        E.loop_sound(music)
    end
end

-------------------------------------------------------------------------------

function init_score()

    -- Set defaults in case the high score file does not exist.

    initials[1] = "A"
    initials[2] = "A"
    initials[3] = "A"

    high_score = 100

    -- Try to read the high score file.

    local fin = io.open(high_score_file, "r")

    if fin then
        local score = fin:read("*n")
        local name  = fin:read("*a")

        fin:close(fin)

        if score then
            high_score = score
        end

        if name then
            initials[1] = string.sub(name, 1, 1)
            initials[2] = string.sub(name, 2, 2)
            initials[3] = string.sub(name, 3, 3)
        end
    end

    -- Apply the current high score to the display.

    set_overlay_alpha(overlay.high_inits[1], initials[1])
    set_overlay_alpha(overlay.high_inits[2], initials[2])
    set_overlay_alpha(overlay.high_inits[3], initials[3])

    set_overlay_curr_score(curr_score)
    set_overlay_high_score(high_score)
end

function write_score()
    local fout = io.open(high_score_file, "w")

    if fout then
        fout:write(high_score, initials[1], initials[2], initials[3])
        fout:close()
    end
end

-------------------------------------------------------------------------------

function entity_distance(entity1, entity2)
    local pos_x, pos_y, pos_z = E.get_entity_position(entity1)
    local pos_X, pos_Y, pos_Z = E.get_entity_position(entity2)

    return math.sqrt((pos_X - pos_x) * (pos_X - pos_x) +
                     (pos_Y - pos_y) * (pos_Y - pos_y))
end

-------------------------------------------------------------------------------

function add_bullet()
    local bullet = { }

    -- Compute the bullet's position and velocity from the ship's location.

    local pos_x, pos_y, pos_z = E.get_entity_position(player.entity)

    local a  = -math.rad(player.rot)
    local dx =  math.sin(a)
    local dy =  math.cos(a)

    bullet.dx = dx * 250 + player.dx
    bullet.dy = dy * 250 + player.dy

    -- Add a new bullet sprite to the scene.

    bullet.entity = create_bullet(pos_x + dx * 10, pos_y + dy * 10, pos_z)

    table.insert(bullets, bullet)

    -- Bullets cost one point.

    if curr_score > 0 then
        curr_score = curr_score - 1
        set_overlay_curr_score(curr_score)
    end

    if sound_on then
        E.play_sound(sound.bullet)
    end
end

function del_bullet(id, bullet)
    E.delete_entity(bullet.entity)
    table.remove(bullets, id)
end

function step_bullet(dt, id, bullet)

    -- Find the bullet's new position.
    
    local x, y, z = E.get_entity_position(bullet.entity)

    x = x + bullet.dx * dt
    y = y + bullet.dy * dt

    -- Clip the position to the viewport.

    if     x < viewport.L then del_bullet(id, bullet)
    elseif x > viewport.R then del_bullet(id, bullet)
    elseif y < viewport.B then del_bullet(id, bullet)
    elseif y > viewport.T then del_bullet(id, bullet)

    -- Apply the new position and orientation.

    else
        E.set_entity_position(bullet.entity, x, y, z)
    end
end

-------------------------------------------------------------------------------

function add_explosion(entity, source, size)
    local explosion = { }

    local x, y, z = E.get_entity_position(entity)

    -- Add the new explosion sprite to the scene.

    explosion.entity = create_explosion(source, size, x, y, 0)

    table.insert(explosions, explosion)

    if sound_on then
        if     size == 1 then E.play_sound(sound.rock1_explosion)
        elseif size == 2 then E.play_sound(sound.rock2_explosion)
        elseif size == 3 then E.play_sound(sound.rock3_explosion)
        elseif size == 5 then E.play_sound(sound.ship_explosion) end
    end
end

function del_explosion(id, explosion)
    E.delete_entity(explosion.entity)
    table.remove(explosions, id)
end

function step_explosion(dt, id, explosion)
    local a = E.get_entity_alpha(explosion.entity) - dt

    -- Fade an explosion over 1 second, delete it when it disappears.

    if a < 0 then
        del_explosion(id, explosion)
    else
        E.set_entity_alpha(explosion.entity, a)
    end
end

-------------------------------------------------------------------------------

function add_shockwave(entity, source, size)
    local shockwave = { }

    local x, y, z = E.get_entity_position(entity)

    -- Add the new shockwave sprite to the scene.

    shockwave.entity = create_shockwave(source, size, x, y, 0)

    table.insert(shockwaves, shockwave)
end

function del_shockwave(id, shockwave)
    E.delete_entity(shockwave.entity)
    table.remove(shockwaves, id)
end

function step_shockwave(dt, id, shockwave)
    local a       = E.get_entity_alpha(shockwave.entity) - dt
    local x, y, z = E.get_entity_scale(shockwave.entity)

    -- Fade and expand a shockwave over 1 second, delete it when it disappears.

    x = x + dt
    y = y + dt

    if a < 0 then
        del_shockwave(id, shockwave)
    else
        E.set_entity_alpha(shockwave.entity, a)
        E.set_entity_scale(shockwave.entity, x, y, z)
    end
end

-------------------------------------------------------------------------------

function add_score(entity, value)
    local score = { }

    local x, y, z = E.get_entity_position(entity)

    -- Tally the score

    curr_score = curr_score + value
    set_overlay_curr_score(curr_score)

    -- Award a free ship

    if curr_score > free_score then
        curr_ships = curr_ships + 1
        free_score = free_score + 1000

        set_overlay_curr_ships(curr_ships)

        if sound_on then
            E.play_sound(sound.free)
        end
    end

    -- Add the new score sprite to the scene.

    score.entity = create_score(value, x, y, 0)

    table.insert(scores, score)
end

function del_score(id, score)
    E.delete_entity(score.entity)
    table.remove(scores, id)
end

function step_score(dt, id, score)
    local a = E.get_entity_alpha(score.entity) - dt / 3.0

    -- Fade a score over 3 seconds, delete it when it disappears.

    if a < 0 then
        del_score(id, score)
    else
        E.set_entity_alpha(score.entity, a)
    end
end

-------------------------------------------------------------------------------

function add_asteroid(entity, size)
    local asteroid = { }
    local scale    = size / 3

    local x, y, z

    -- Pick a random velocity vector.

    local a  = math.rad(math.random(-180, 180))
    local dx = math.sin(a)
    local dy = math.cos(a)

    -- If an entity is given, use that location, else use the random velocity.

    if entity then
        x, y, z = E.get_entity_position(entity)
    else
        x = dx * viewport.h * 0.4
        y = dy * viewport.h * 0.4
    end

    -- Add a new asteroid object and define its motion parameters.

    asteroid.entity = create_rock(size, x, y, 0)
    asteroid.size   = size
    asteroid.dx     = dx * curr_speed
    asteroid.dy     = dy * curr_speed
    asteroid.wx     = math.random(-90, 90)
    asteroid.wy     = math.random(-90, 90)
    asteroid.rot_x  = 0
    asteroid.rot_y  = 0

    table.insert(asteroids, asteroid)

    curr_count = curr_count + 1
end

function del_asteroid(id, asteroid)
    E.delete_entity(asteroid.entity)
    table.remove(asteroids, id)

    curr_count = curr_count - 1
end

function test_asteroid(asteroid, jd, bullet)
    local size = asteroid.size

    if entity_distance(asteroid.entity, bullet.entity) < 15 * size then
        del_bullet(jd, bullet)
        return true
    else
        return nil
    end
end

function frag_asteroid(id, asteroid)

    local size = asteroid.size

    -- Break a big asteroid into smaller asteroids.

    add_shockwave(asteroid.entity, entity.rock_shockwave, size)
    add_explosion(asteroid.entity, entity.rock_explosion, size)

    if size > 1 then
        add_asteroid(asteroid.entity, size - 1)
        add_asteroid(asteroid.entity, size - 1)
        add_asteroid(asteroid.entity, size - 1)
    end

    if size == 3 then add_score(asteroid.entity, 25) end
    if size == 2 then add_score(asteroid.entity, 10) end
    if size == 1 then add_score(asteroid.entity,  5) end

    -- Delete the original asteroid.

    del_asteroid(id, asteroid)
end

function step_asteroid(dt, id, asteroid)
    local size = asteroid.size
    local dx   = asteroid.dx
    local dy   = asteroid.dy
    local wx   = asteroid.wx
    local wy   = asteroid.wy

    -- Find the asteroid's new position and orientation.
    
    local pos_x, pos_y, pos_z = E.get_entity_position(asteroid.entity)
    local rot_x, rot_y = asteroid.rot_x, asteroid.rot_y

    pos_x = pos_x + dx * dt
    pos_y = pos_y + dy * dt
    rot_x = rot_x + wx * dt
    rot_y = rot_y + wy * dt

    -- Wrap the position to the viewport.

    if     pos_x < viewport.L then pos_x = pos_x + viewport.w
    elseif pos_x > viewport.R then pos_x = pos_x - viewport.w end
    if     pos_y < viewport.B then pos_y = pos_y + viewport.h
    elseif pos_y > viewport.T then pos_y = pos_y - viewport.h end

    -- Wrap the orientation to (-180, 180).

    if     rot_x < -180 then rot_x = rot_x + 360
    elseif rot_x >  180 then rot_x = rot_x - 360 end
    if     rot_y < -180 then rot_y = rot_y + 360
    elseif rot_y >  180 then rot_y = rot_y - 360 end

    -- Apply the new position and orientation.

    asteroid.rot_x = rot_x
    asteroid.rot_y = rot_y

    E.set_entity_position(asteroid.entity, pos_x, pos_y, pos_z)
    E.set_entity_rotation(asteroid.entity, rot_x, rot_y, 0)

    -- Check this asteroid against all bullets.

    if (table.foreach(bullets, function (jd, bullet)
                                   return test_asteroid(asteroid, jd, bullet)
                               end)) then
        frag_asteroid(id, asteroid)
    end

    return true
end

-------------------------------------------------------------------------------

function set_thrust(b)
    player.thrusting = b

    E.set_entity_flags(entity.thrust, E.entity_flag_hidden, not b)

    if sound_on then
        if b then
            E.play_sound(sound.thrust1)
            E.loop_sound(sound.thrust2)
        else
            E.stop_sound(sound.thrust2)
            E.play_sound(sound.thrust3)
        end
    end
end

function test_player(id, asteroid)
    local size = asteroid.size

    if entity_distance(asteroid.entity, player.entity) < 20 * size then
        return asteroid
    else
        return nil
    end
end

function init_player(new)
    local s = 0.05

    -- If the player has not yet been initialized, create its entities.

    if not player.entity then
        player.entity = create_ship(0, 0, 0)

        E.parent_entity(entity.thrust, player.entity)
        E.set_entity_scale(entity.thrust, s, s, s)

        E.set_entity_flags(entity.thrust, E.entity_flag_hidden, true)
    end

    -- If any ships remain, initialize one of them.

    if curr_ships > 0 then
        if new then
            curr_ships = curr_ships - 1
        end

        set_overlay_curr_ships(curr_ships)

        E.set_entity_position(player.entity, 0, 0, 0)
        E.set_entity_rotation(player.entity, 0, 0, 0)
        E.set_entity_flags   (player.entity, E.entity_flag_hidden, false)
        E.set_entity_flags   (entity.thrust, E.entity_flag_hidden, true)

        set_thrust(false)

        player.turning_L = false
        player.turning_R = false
        player.rot       = 0
        player.dx        = 0
        player.dy        = 0

        return true
    else
        return false
    end
end

function step_player(dt)
    local pos_x, pos_y, pos_z = E.get_entity_position(player.entity)
    local joy_x, joy_y        = E.get_joystick(joy_dev)

    -- Handle thrust.

    if player.thrusting then
        player.dx = player.dx - math.sin(math.rad(player.rot)) * dt * 200
        player.dy = player.dy + math.cos(math.rad(player.rot)) * dt * 200
    end

    -- Handle position change.

    pos_x = pos_x + player.dx * dt
    pos_y = pos_y + player.dy * dt

    -- Wrap the position to the viewport.

    if pos_x < viewport.L then pos_x = pos_x + viewport.w end
    if pos_x > viewport.R then pos_x = pos_x - viewport.w end
    if pos_y < viewport.B then pos_y = pos_y + viewport.h end
    if pos_y > viewport.T then pos_y = pos_y - viewport.h end

    -- Handle rotation change.

    if joy_x < -0.5 or player.turning_L then
        player.rot = player.rot + dt * 180
    end
    if joy_x >  0.5 or player.turning_R then
        player.rot = player.rot - dt * 180
    end

    -- Wrap the orientation to (-180, 180).

    if     player.rot < -180 then player.rot = player.rot + 360
    elseif player.rot >  180 then player.rot = player.rot - 360 end

    -- Apply the new position and rotation.

    E.set_entity_position(player.entity, pos_x, pos_y, pos_z)
    E.set_entity_rotation(player.entity, 0, 0, player.rot)

    -- Check the player against all asteroids.

    return table.foreach(asteroids, test_player)
end

function kill_player()

    add_explosion(player.entity, entity.ship_explosion, 5)
    add_shockwave(player.entity, entity.ship_shockwave, 5)
    
    set_thrust(false)

    player.turning_L = false
    player.turning_r = false
    player.dx        = 0
    player.dy        = 0

    E.set_entity_flags(player.entity, E.entity_flag_hidden, true)

    if sound_on then
        E.stop_sound(sound.thrust2)
    end
end

-------------------------------------------------------------------------------

function init_level()
    for i = 1, curr_level + 3 do
        add_asteroid(nil, 3)
    end
end

function stop_level()
    while curr_count > 0 do
        table.foreach(asteroids, del_asteroid)
    end
end

function step_level(dt)
    table.foreach(scores,     function (i, v) step_score    (dt, i, v) end)
    table.foreach(bullets,    function (i, v) step_bullet   (dt, i, v) end)
    table.foreach(asteroids,  function (i, v) step_asteroid (dt, i, v) end)
    table.foreach(shockwaves, function (i, v) step_shockwave(dt, i, v) end)
    table.foreach(explosions, function (i, v) step_explosion(dt, i, v) end)
end

-------------------------------------------------------------------------------
-- The NULL state bootstraps the game, ensuring that title.enter is called.

state.null = { }

function state.null.timer()
    return state.title
end

-------------------------------------------------------------------------------
-- The TITLE state displays the title screen and attract mode.

state.title = { }

function state.title.enter()
    E.set_entity_flags(overlay.title, E.entity_flag_hidden, false)
end

function state.title.leave()
    curr_ships =    2
    curr_speed =  100
    curr_level =    0
    curr_score =    0
    free_score = 1000

    set_overlay_curr_score(curr_score)
    set_overlay_curr_ships(curr_ships)

    E.set_entity_flags(overlay.title, E.entity_flag_hidden, true)
end

function state.title.timer(dt)
    step_scene(dt)
    step_level(dt)
    return state.title
end

function state.title.button_A(s)
    if s then
        return state.ready
    else
        return state.title
    end
end

-------------------------------------------------------------------------------
-- The READY state handles the pause before a level begins.

state.ready = { }

function state.ready.enter()
    E.set_entity_flags(overlay.ready, E.entity_flag_hidden, false)
    E.set_entity_flags(overlay.level, E.entity_flag_hidden, false)

    curr_level = curr_level + 1

    stop_level()
    init_level()
    init_player(false)

    set_overlay_digit(overlay.level, curr_level)
end

function state.ready.leave()
    E.set_entity_flags(overlay.ready, E.entity_flag_hidden, true)
    E.set_entity_flags(overlay.level, E.entity_flag_hidden, true)
end

function state.ready.timer(dt)
    step_scene(dt)
    return state.ready
end

function state.ready.button_A(s)
    if s then
        return state.play
    else
        return state.ready
    end
end

-------------------------------------------------------------------------------
-- The PLAY state handles normal gameplay.

state.play = { }

function state.play.timer(dt)
    if step_player(dt) then
        kill_player()
        return state.dead
    else
        step_scene(dt)
        step_level(dt)

        if curr_count == 0 then
            return state.clear
        else
            return state.play
        end
    end
end

function state.play.button_A(s)
    if s then
        add_bullet(player.entity)
    end
    return state.play
end

function state.play.button_B(s)
    set_thrust(s)
    return state.play
end

-------------------------------------------------------------------------------
-- The DEAD state handles the transition after the player dies.

state.dead = { }

function state.dead.timer(dt)
    step_scene(dt)
    step_level(dt)
    return state.dead
end

function state.dead.button_A(s)
    if s and time_state > 1 then
        if init_player(true) then
            return state.play
        else
            if curr_score > high_score then
                return state.high
            else
                return state.over
            end
        end
    else
        return state.dead
    end
end

-------------------------------------------------------------------------------
-- The CLEAR state handles the transition when a level is successfully cleared.

state.clear = { }

function state.clear.enter()
    E.set_entity_flags(overlay.clear, E.entity_flag_hidden, false)
end

function state.clear.leave()
    curr_speed = curr_speed * 1.25
    E.set_entity_flags(overlay.clear, E.entity_flag_hidden, true)
end

function state.clear.timer(dt)
    step_player(dt)
    step_scene(dt)
    return state.clear
end

function state.clear.button_A(s)
    if s and time_state > 1 then
        return state.ready
    else
        return state.clear
    end
end

-------------------------------------------------------------------------------
-- The HIGH state allows the player to enter their initials for the high score.

state.high = { }

function state.high.enter()
    E.set_entity_flags(overlay.high, E.entity_flag_hidden, false)

    high_score = curr_score
    init_ready = true
    init_index = 1

    set_overlay_high_score(high_score)
end

function state.high.leave()
    local k = 0.125

    write_score()

    E.set_entity_scale(overlay.high_inits[1], k / 2, k, k)
    E.set_entity_scale(overlay.high_inits[2], k / 2, k, k)
    E.set_entity_scale(overlay.high_inits[3], k / 2, k, k)

    E.set_entity_flags(overlay.high, E.entity_flag_hidden, true)
end

function state.high.timer(dt)
    local k    = 0.15625 + 0.03125 * math.sin(time_state * 10)
    local x, y = E.get_joystick(joy_dev)

    E.set_entity_scale(overlay.high_inits[init_index], k / 2, k, k)

    if -0.5 < x and x < 0.5 then
        init_ready = true
    else
        if init_ready then
            if x < -0.5 then
                initials[init_index] = prev_letter(initials[init_index])
                set_overlay_alpha(overlay.high_inits[init_index],
                                            initials[init_index])
            end
            if x >  0.5 then
                initials[init_index] = next_letter(initials[init_index])
                set_overlay_alpha(overlay.high_inits[init_index],
                                            initials[init_index])
            end
        end

        init_ready = false
    end

    step_scene(dt)

    return state.high
end

function state.high.button_A(s)
    local k = 0.125

    if s then
        if init_index < 3 then
            E.set_entity_scale(overlay.high_inits[1], k / 2, k, k)
            E.set_entity_scale(overlay.high_inits[2], k / 2, k, k)
            E.set_entity_scale(overlay.high_inits[3], k / 2, k, k)

            init_index = init_index + 1
        else
            return state.title
        end
    end

    return state.high
end

function state.high.button_B(s)
    local k = 0.125

    if s then
        if init_index > 1 then
            E.set_entity_scale(overlay.high_inits[1], k / 2, k, k)
            E.set_entity_scale(overlay.high_inits[2], k / 2, k, k)
            E.set_entity_scale(overlay.high_inits[3], k / 2, k, k)

            init_index = init_index - 1
        end
    end

    return state.high
end

-------------------------------------------------------------------------------
-- The OVER state handles game-over, man.

state.over = { }

function state.over.enter()
    E.set_entity_flags(overlay.over, E.entity_flag_hidden, false)
end

function state.over.leave()
    E.set_entity_flags(overlay.over, E.entity_flag_hidden, true)
end

function state.over.timer(dt)
    step_scene(dt)
    step_level(dt)
    return state.over
end

function state.over.button_A(s)
    if s and time_state > 1 then
        return state.title
    else
        return state.over
    end
end

-------------------------------------------------------------------------------
-- State handler functions return a reference to a state.  If this state is
-- not the current one, then a transition has occured and the enter and leave
-- handlers must be called.

function do_state(next_state)
    if not (next_state == curr_state) then
        if curr_state and curr_state.leave then
            curr_state.leave()
        end

        curr_state = next_state
        time_state = 0

        if curr_state and curr_state.enter then
            curr_state.enter()
        end
    end
end

-------------------------------------------------------------------------------
-- These are the Electro callbacks.  They distribute user events to the current
-- state's event handlers.

function do_start()
    math.randomseed(os.time())

    init_viewport()
    init_scene()
    init_entity()
    init_overlay()
    init_score()
    init_sound()

    curr_state = state.null

    E.enable_timer(true)
end

function do_timer(time)
    time_state = time_state + time

    if time > 0 and curr_state and curr_state.timer then
        do_state(curr_state.timer(time))
        return true
    end

    return false
end

function do_joystick(device, button, down)
    if device == joy_dev then
        if button == 0 and curr_state and curr_state.button_A then
            do_state(curr_state.button_A(down))
            return true
        end
        if button == 1 and curr_state and curr_state.button_B then
            do_state(curr_state.button_B(down))
            return true
        end
        if button == 2 and curr_state and curr_state.button_A then
            do_state(curr_state.button_A(down))
            return true
        end
    end

    return false
end

function do_keyboard(key, down)
    if key ==  E.key_space and curr_state and curr_state.button_A then
        do_state(curr_state.button_A(down))
        return true
    end
    if key == E.key_up and curr_state and curr_state.button_B then
        do_state(curr_state.button_B(down))
        return true
    end

    if key == E.key_right then
        player.turning_R = down
    end
    if key == E.key_left  then
        player.turning_L = down
    end

    if key == E.key_F12 then
        E.nuke()
        E.chdir("..")
        dofile("demo.lua")
        return true
    end

    return false
end

-------------------------------------------------------------------------------

E.set_background(0.0, 0.0, 0.0, 0.1, 0.2, 0.4)
do_start()
