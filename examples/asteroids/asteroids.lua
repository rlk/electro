
sound = false

-------------------------------------------------------------------------------

global_ship      = nil
global_rock      = nil
global_bullet    = nil
global_score05   = nil
global_score10   = nil
global_score25   = nil
global_explosion = nil
global_shockwave = nil

button_fire   = 0
button_thrust = 1

-------------------------------------------------------------------------------

time  = 0
fire  = nil
boom  = nil
music = nil

state     = "start"

serial    = 1
space     = nil
galaxy    = nil
camera    = nil
above     = nil
below     = nil
light     = nil
ship      = nil

undone     = 0
bullets    = { }
asteroids  = { }
explosions = { }
shockwaves = { }
scores     = { }

thrusting   = false
velocity_dx = 0
velocity_dy = 0

level      = 1
ships      = 0
curr_score = 0
high_score = 100
speed      = 7

wait_time  = 0

-------------------------------------------------------------------------------

global_l = 0
global_r = 0
global_b = 0
global_t = 0
global_w = 0
global_h = 0
global_z = 1

function player_reset()

    thrusting   = false
    velocity_dx = 0
    velocity_dy = 0

    if ship then
        E.entity_flag(ship, E.entity_flag_hidden, false)

        E.entity_position(ship, 0, 0, 0)
        E.entity_rotation(ship, 0, 0, 0)
    end
end

-------------------------------------------------------------------------------

function digit_set(sprite, n)
    if     n == 0 then E.sprite_bounds(sprite, 0.00, 0.25, 0.75, 1.00)
    elseif n == 1 then E.sprite_bounds(sprite, 0.25, 0.50, 0.75, 1.00)
    elseif n == 2 then E.sprite_bounds(sprite, 0.50, 0.75, 0.75, 1.00)
    elseif n == 3 then E.sprite_bounds(sprite, 0.75, 1.00, 0.75, 1.00)
    elseif n == 4 then E.sprite_bounds(sprite, 0.00, 0.25, 0.50, 0.75)
    elseif n == 5 then E.sprite_bounds(sprite, 0.25, 0.50, 0.50, 0.75)
    elseif n == 6 then E.sprite_bounds(sprite, 0.50, 0.75, 0.50, 0.75)
    elseif n == 7 then E.sprite_bounds(sprite, 0.75, 1.00, 0.50, 0.75)
    elseif n == 8 then E.sprite_bounds(sprite, 0.00, 0.25, 0.25, 0.50)
    elseif n == 9 then E.sprite_bounds(sprite, 0.25, 0.50, 0.25, 0.50) end
end

function alpha_set(sprite, c)
    if     c == "A" then E.sprite_bounds(sprite, 0.000, 0.125, 0.75, 1.00)
    elseif c == "B" then E.sprite_bounds(sprite, 0.125, 0.250, 0.75, 1.00)
    elseif c == "C" then E.sprite_bounds(sprite, 0.250, 0.375, 0.75, 1.00)
    elseif c == "D" then E.sprite_bounds(sprite, 0.375, 0.500, 0.75, 1.00)
    elseif c == "E" then E.sprite_bounds(sprite, 0.500, 0.625, 0.75, 1.00)
    elseif c == "F" then E.sprite_bounds(sprite, 0.625, 0.750, 0.75, 1.00)
    elseif c == "G" then E.sprite_bounds(sprite, 0.750, 0.875, 0.75, 1.00)
    elseif c == "H" then E.sprite_bounds(sprite, 0.875, 1.000, 0.75, 1.00)
    elseif c == "I" then E.sprite_bounds(sprite, 0.000, 0.125, 0.50, 0.75)
    elseif c == "J" then E.sprite_bounds(sprite, 0.125, 0.250, 0.50, 0.75)
    elseif c == "K" then E.sprite_bounds(sprite, 0.250, 0.375, 0.50, 0.75)
    elseif c == "L" then E.sprite_bounds(sprite, 0.375, 0.500, 0.50, 0.75)
    elseif c == "M" then E.sprite_bounds(sprite, 0.500, 0.625, 0.50, 0.75)
    elseif c == "N" then E.sprite_bounds(sprite, 0.625, 0.750, 0.50, 0.75)
    elseif c == "O" then E.sprite_bounds(sprite, 0.750, 0.875, 0.50, 0.75)
    elseif c == "P" then E.sprite_bounds(sprite, 0.875, 1.000, 0.50, 0.75)
    elseif c == "Q" then E.sprite_bounds(sprite, 0.000, 0.125, 0.25, 0.50)
    elseif c == "R" then E.sprite_bounds(sprite, 0.125, 0.250, 0.25, 0.50)
    elseif c == "S" then E.sprite_bounds(sprite, 0.250, 0.375, 0.25, 0.50)
    elseif c == "T" then E.sprite_bounds(sprite, 0.375, 0.500, 0.25, 0.50)
    elseif c == "U" then E.sprite_bounds(sprite, 0.500, 0.625, 0.25, 0.50)
    elseif c == "V" then E.sprite_bounds(sprite, 0.625, 0.750, 0.25, 0.50)
    elseif c == "W" then E.sprite_bounds(sprite, 0.750, 0.875, 0.25, 0.50)
    elseif c == "X" then E.sprite_bounds(sprite, 0.875, 1.000, 0.25, 0.50)
    elseif c == "Y" then E.sprite_bounds(sprite, 0.000, 0.125, 0.00, 0.25)
    elseif c == "Z" then E.sprite_bounds(sprite, 0.125, 0.250, 0.00, 0.25) end
end

-------------------------------------------------------------------------------

function create_string(s)
    local scale = 1 / 128
    local dx    = 2.5
    local dy    = 2.5
    local l     = string.len(s)

    local pivot = E.create_pivot()
    local glyph

    for i = 1, l do
        local c = string.upper(string.sub(s, i, i))

        if "0" <= c and c <= "9" then
            glyph = E.create_sprite("digit.png")
            digit_set(glyph, string.byte(c) - 48)
            E.entity_scale(glyph, scale, scale, scale);
        end
        if "A" <= c and c <= "Z" then
            glyph = E.create_sprite("alpha.png")
            alpha_set(glyph, c)
            E.entity_scale(glyph, scale / 2, scale, scale);
        end

        E.entity_parent  (glyph, pivot)
        E.entity_position(glyph, dx * (i - (l + 1) / 2), 0, 0)
    end

    E.entity_scale(pivot, 2, 2, 2)
    return pivot
end

-------------------------------------------------------------------------------

overlay = nil

function add_overlay(s)
    overlay = create_string(s)
    E.entity_parent(overlay, camera)
end

function del_overlay()
    E.entity_delete(overlay)
end

-------------------------------------------------------------------------------

local curr_score_digit = { }
local high_score_digit = { }

function curr_score_set(n)
    digit_set(curr_score_digit[0], math.mod(n,                     10));
    digit_set(curr_score_digit[1], math.mod(math.floor(n / 10),    10));
    digit_set(curr_score_digit[2], math.mod(math.floor(n / 100),   10));
    digit_set(curr_score_digit[3], math.mod(math.floor(n / 1000),  10));
    digit_set(curr_score_digit[4], math.mod(math.floor(n / 10000), 10));
end

function high_score_set(n)
    digit_set(high_score_digit[0], math.mod(n,                     10));
    digit_set(high_score_digit[1], math.mod(math.floor(n / 10),    10));
    digit_set(high_score_digit[2], math.mod(math.floor(n / 100),   10));
    digit_set(high_score_digit[3], math.mod(math.floor(n / 1000),  10));
    digit_set(high_score_digit[4], math.mod(math.floor(n / 10000), 10));
end

function score_init()
    local scale = 1 / 128
    local dx    = 2.5
    local dy    = 2.5

    local curr_r = global_r
    local high_r = global_l + dx * 6

    -- Create the current score display.

    for i = 0, 4 do
        curr_score_digit[i] = E.create_sprite("digit.png")

        E.entity_scale   (curr_score_digit[i], scale, scale, scale);
        E.entity_parent  (curr_score_digit[i], camera)
        E.entity_position(curr_score_digit[i], curr_r - dx * (i + 1),
                                             global_t - dy, 0)
    end

    -- Create the high score display.

    for i = 0, 4 do
        high_score_digit[i] = E.create_sprite("digit.png")

        E.entity_scale   (high_score_digit[i], scale, scale, scale);
        E.entity_parent  (high_score_digit[i], camera)
        E.entity_position(high_score_digit[i], high_r - dx * (i + 1),
                                             global_t - dy, 0)
    end

    curr_score_set(curr_score)
    high_score_set(high_score)
end


-------------------------------------------------------------------------------

function add_asteroid(entity, size)
    local asteroid = { }
    local scale = size / 3

    -- Pick a random velocity vector.

    local a  = math.rad(math.random(-180, 180))
    local dx = math.sin(a)
    local dy = math.cos(a)

    -- If an entity is given, use that location, else use the random velocity.

    if entity then
        x, y, z = E.entity_get_position(entity)
    else
        x = dx * global_h * 0.4
        y = dy * global_h * 0.4
        z = 0
    end

    -- Clone a new asteroid object and define its motion parameters.

    asteroid.entity = E.entity_clone(global_rock)
    asteroid.size   = size
    asteroid.dx     = dx * speed
    asteroid.dy     = dy * speed
    asteroid.wx     = math.random(-90, 90)
    asteroid.wy     = math.random(-90, 90)

    -- Add the new asteroid to the scene.

    E.entity_scale   (asteroid.entity, scale, scale, scale)
    E.entity_parent  (asteroid.entity, light)
    E.entity_position(asteroid.entity, x, y, 0)

    table.insert(asteroids, asteroid)
    undone = undone + 1
end

function del_asteroid(id, asteroid)
    E.entity_delete(asteroid.entity)
    table.remove(asteroids, id)
    undone = undone - 1

    if undone == 0 then
        state = "done"
    end
end

-------------------------------------------------------------------------------

function add_bullet()
    local bullet = { }
    local scale  = 1 / 64

    -- Clone a new bullet sprite.

    bullet.entity = E.entity_clone(global_bullet)

    -- Compute the bullet's position and velocity from the ship's location.

    local pos_x, pos_y, pos_z = E.entity_get_position(ship)
    local rot_x, rot_y, rot_z = E.entity_get_rotation(ship)

    local a = -math.rad(rot_z)
    local x =  math.sin(a)
    local y =  math.cos(a)

    bullet.dx = x * 15 + velocity_dx
    bullet.dy = y * 15 + velocity_dy

    -- Add the new sprite to the scene.

    E.entity_scale   (bullet.entity, scale, scale, scale)
    E.entity_parent  (bullet.entity, below)
    E.entity_position(bullet.entity, pos_x + x, pos_y + y, pos_z)

    table.insert(bullets, bullet)

    -- Bullets cost one point.

    if curr_score > 0 then
        curr_score = curr_score - 1
        curr_score_set(curr_score)
    end

    if sound then E.sound_play(fire) end
end

function del_bullet(id, bullet)
    E.entity_delete(bullet.entity)
    table.remove(bullets, id)
end

-------------------------------------------------------------------------------

function add_explosion(entity, size)
    local explosion = { }
    local scale = 0.05 * size / 3.0

    -- Clone a new explosion sprite.

    explosion.entity = E.entity_clone(global_explosion)

    -- Add the new sprite to the scene.

    local x, y, z = E.entity_get_position(entity)

    E.entity_scale   (explosion.entity, scale, scale, scale)
    E.entity_parent  (explosion.entity, above)
    E.entity_position(explosion.entity, x, y, 0)

    table.insert(explosions, explosion)

    if sound then E.sound_play(boom) end
end

function del_explosion(id, explosion)
    E.entity_delete(explosion.entity)
    table.remove(explosions, id)
end

-------------------------------------------------------------------------------

function add_shockwave(entity, size)
    local shockwave = { }
    local scale = 0.025 * size / 3.0

    -- Clone a new shockwave sprite.

    shockwave.entity = E.entity_clone(global_shockwave)

    -- Add the new sprite to the scene.

    local x, y, z = E.entity_get_position(entity)

    E.entity_scale   (shockwave.entity, scale, scale, scale)
    E.entity_parent  (shockwave.entity, above)
    E.entity_position(shockwave.entity, x, y, 0)

    table.insert(shockwaves, shockwave)
end

function del_shockwave(id, shockwave)
    E.entity_delete(shockwave.entity)
    table.remove(shockwaves, id)
end

-------------------------------------------------------------------------------

function add_score(entity, value)
    local score = { }
    local scale = 0.01

    -- Tally the score

    curr_score = curr_score + value
    curr_score_set(curr_score)

    if curr_score > high_score then
        high_score = curr_score
        high_score_set(high_score)
    end

    -- Select the correct sprite for the given value.

    if value ==  5 then score.entity = E.entity_clone(global_score05) end
    if value == 10 then score.entity = E.entity_clone(global_score10) end
    if value == 25 then score.entity = E.entity_clone(global_score25) end

    -- Add the new sprite to the scene.

    local x, y, z = E.entity_get_position(entity)

    E.entity_scale   (score.entity, scale, scale, scale)
    E.entity_parent  (score.entity, above)
    E.entity_position(score.entity, x, y, 0)

    table.insert(scores, score)
end

function del_score(id, score)
    E.entity_delete(score.entity)
    table.remove(scores, id)
end

-------------------------------------------------------------------------------

global_dt = 0

function asteroid_frag(id, asteroid)

    local entity = asteroid.entity
    local size   = asteroid.size

    -- Break a big asteroid into smaller asteroids.

    add_shockwave(entity, size)
    add_explosion(entity, size)

    if size > 1 then
        add_asteroid(entity, size - 1)
        add_asteroid(entity, size - 1)
        add_asteroid(entity, size - 1)
    end

    if size == 3 then add_score(entity, 25) end
    if size == 2 then add_score(entity, 10) end
    if size == 1 then add_score(entity,  5) end

    -- Delete the original asteroid.

    del_asteroid(id, asteroid)
end

function asteroid_step(id, asteroid)

    local entity = asteroid.entity
    local size   = asteroid.size
    local dx     = asteroid.dx
    local dy     = asteroid.dy
    local wx     = asteroid.wx
    local wy     = asteroid.wy

    -- Find the asteroid's new position and orientation.
    
    local pos_x, pos_y, pos_z = E.entity_get_position(entity)
    local rot_x, rot_y, rot_z = E.entity_get_rotation(entity)

    pos_x = pos_x + dx * global_dt
    pos_y = pos_y + dy * global_dt
    rot_x = rot_x + wx * global_dt
    rot_y = rot_y + wy * global_dt

    -- Wrap the position to the viewport.

    if pos_x < global_l then pos_x = pos_x + global_w end
    if pos_x > global_r then pos_x = pos_x - global_w end
    if pos_y < global_b then pos_y = pos_y + global_h end
    if pos_y > global_t then pos_y = pos_y - global_h end

    -- Wrap the orientation to (-180, 180).

    if rot_x < -180 then rot_x = rot_x + 360 end
    if rot_x >  180 then rot_x = rot_x - 360 end
    if rot_y < -180 then rot_y = rot_y + 360 end
    if rot_y >  180 then rot_y = rot_y - 360 end

    -- Apply the new position and orientation.

    E.entity_position(entity, pos_x, pos_y, pos_z)
    E.entity_rotation(entity, rot_x, rot_y, rot_z)

    -- Check this asteroid against all bullets.

    for jd, bullet in pairs(bullets) do
        local pos_X, pos_Y, pos_Z = E.entity_get_position(bullet.entity)

        local dist = math.sqrt((pos_X - pos_x) * (pos_X - pos_x) +
                               (pos_Y - pos_y) * (pos_Y - pos_y));
        if dist < size then
            asteroid_frag(id, asteroid)
            del_bullet   (jd, bullet)
        end
    end

    -- Check this asteroid against the player.

    if state == "play" and ship then
        local pos_X, pos_Y, pos_Z = E.entity_get_position(ship)

        local dist = math.sqrt((pos_X - pos_x) * (pos_X - pos_x) +
                               (pos_Y - pos_y) * (pos_Y - pos_y));

        if dist < size + 0.5 then
            add_explosion(ship, 10)
            add_shockwave(ship, 10)

            E.entity_flag(ship, E.entity_flag_hidden, true)

            velocity_dx = 0
            velocity_dy = 0
            wait_time   = 0
            state       = "dead"
        end
    end
end

function bullet_step(id, bullet)

    local entity = bullet.entity
    local dx     = bullet.dx
    local dy     = bullet.dy

    -- Find the bullet's new position.
    
    local pos_x, pos_y, pos_z = E.entity_get_position(entity)

    pos_x = pos_x + dx * global_dt
    pos_y = pos_y + dy * global_dt

    -- Clip the position to the viewport.

    if     pos_x < global_l then
        del_bullet(id, bullet)
    elseif pos_x > global_r then
        del_bullet(id, bullet)
    elseif pos_y < global_b then
        del_bullet(id, bullet)
    elseif pos_y > global_t then
        del_bullet(id, bullet)

    -- Apply the new position and orientation.

    else
        E.entity_position(entity, pos_x, pos_y, pos_z)
    end
end

function explosion_step(id, explosion)
    local entity = explosion.entity

    -- Fade explosion sprites over 1 second.

    E.entity_alpha(entity, E.entity_get_alpha(entity) - global_dt)

    -- If this sprite is transparent, delete it.

    if E.entity_get_alpha(entity) < 0 then
        del_explosion(id, explosion)
    end
end

function shockwave_step(id, shockwave)
    local entity = shockwave.entity

    local scl_x, scl_y, scl_z = E.entity_get_scale(entity);

    -- Fade and scale shockwave sprites oven 1 second.

    E.entity_scale(entity, scl_x + global_dt * 0.05,
                   scl_y + global_dt * 0.05, scl_z)
    E.entity_alpha(entity, E.entity_get_alpha(entity) - global_dt)

    -- If this sprite is transparent, delete it.

    if E.entity_get_alpha(entity) < 0 then
        del_shockwave(id, shockwave)
    end
end

function score_step(id, score)
    local entity = score.entity

    -- Fade score sprites over 2 seconds.

    E.entity_alpha(entity, E.entity_get_alpha(entity) - global_dt / 2)
    
    -- If this sprite is transparent, delete it.

    if E.entity_get_alpha(entity) < 0 then
        del_score(id, score)
    end
end

-------------------------------------------------------------------------------

function level_init()
    for i = 1, level + 3 do
        add_asteroid(nil, 3)
    end
end

function level_stop()
    while undone > 0 do
        table.foreach(asteroids,  del_asteroid)
    end
end

function level_step()
    table.foreach(bullets,    bullet_step)
    table.foreach(explosions, explosion_step)
    table.foreach(shockwaves, shockwave_step)
    table.foreach(scores,     score_step)
    table.foreach(asteroids,  asteroid_step)
end

function stuff_step()
    table.foreach(bullets,    bullet_step)
    table.foreach(explosions, explosion_step)
    table.foreach(shockwaves, shockwave_step)
    table.foreach(scores,     score_step)
end

-------------------------------------------------------------------------------

function player_step()

    local pos_x, pos_y, pos_z = E.entity_get_position(ship)
    local rot_x, rot_y, rot_z = E.entity_get_rotation(ship)

    -- Handle thrust

    if thrusting then
        local r = global_dt * 20

        velocity_dx = velocity_dx - math.sin(math.rad(rot_z)) * r
        velocity_dy = velocity_dy + math.cos(math.rad(rot_z)) * r
    end

    -- Handle rotation change

    if E.joystick_axis(0, 0) < -0.5 then
        rot_z = rot_z + global_dt * 180
    end
    if E.joystick_axis(0, 0) >  0.5 then
        rot_z = rot_z - global_dt * 180
    end

    -- Handle position change

    if math.abs(velocity_dx) > 0.0 then
        pos_x = pos_x + velocity_dx * global_dt
    end
    if math.abs(velocity_dy) > 0.0 then
        pos_y = pos_y + velocity_dy * global_dt
    end

    -- Wrap the orientation to (-180, 180).

    if rot_x < -180 then rot_x = rot_x + 360 end
    if rot_x >  180 then rot_x = rot_x - 360 end
    if rot_y < -180 then rot_y = rot_y + 360 end
    if rot_y >  180 then rot_y = rot_y - 360 end

    -- Wrap the position to the viewport.

    if pos_x < global_l then pos_x = pos_x + global_w end
    if pos_x > global_r then pos_x = pos_x - global_w end
    if pos_y < global_b then pos_y = pos_y + global_h end
    if pos_y > global_t then pos_y = pos_y - global_h end

    -- Apply the new position and rotation.

    E.entity_position(ship, pos_x, pos_y, pos_z)
    E.entity_rotation(ship, rot_x, rot_y, rot_z)
end

-------------------------------------------------------------------------------

function new_global_object(filename)
    object = E.create_object(filename)
    E.entity_flag(object, E.entity_flag_hidden, true)

    return object
end

function new_global_sprite(filename)
    sprite = E.create_sprite(filename)
    E.entity_flag(sprite, E.entity_flag_hidden, true)

    return sprite
end

-------------------------------------------------------------------------------

function do_start()

    math.randomseed(os.time())

    if sound then
        fire  = E.sound_load("fizzle.ogg")
        boom  = E.sound_load("explosion.ogg")
        music = E.sound_load("inter.ogg")

        E.sound_loop(music)
    end

    -- Establish the boundries of the viewport.

    global_l, global_r, global_b, global_t = E.viewport_get()

    global_z = 64 / (global_r - global_l)
    
    global_l = global_l * global_z
    global_r = global_r * global_z
    global_b = global_b * global_z
    global_t = global_t * global_z
    global_w = global_r - global_l
    global_h = global_t - global_b

    -- Initialize some source objects.

    global_ship      = new_global_object("ship.obj")
    global_rock      = new_global_object("rock.obj")
    global_bullet    = new_global_sprite("bullet.png")
    global_score05   = new_global_sprite("score05.png")
    global_score10   = new_global_sprite("score10.png")
    global_score25   = new_global_sprite("score25.png")
    global_explosion = new_global_sprite("explosion.png")
    global_shockwave = new_global_sprite("shockwave.png")

    -- Initialize the scene.

    camera  = E.create_camera(E.camera_type_orthogonal)
    space   = E.create_camera(E.camera_type_perspective)

    light  = E.create_light(E.light_type_positional)
    above  = E.create_pivot()
    below  = E.create_pivot()
    ship   = E.entity_clone(global_ship);
    galaxy = E.create_galaxy("../hip_main.bin")

    E.entity_parent(above,  camera)
    E.entity_parent(light,  camera)
    E.entity_parent(ship,   light)
    E.entity_parent(below,  camera)
    E.entity_parent(galaxy, space)

    E.entity_position(light, 0, 0, 10)
    E.camera_zoom(camera, global_z)

    E.galaxy_magn(galaxy, 500.0)
    E.camera_dist(space,  100.0)
    E.camera_zoom(space,    0.5)
    E.entity_position(space, 0, 15.5, 9200)

    add_overlay("ASTEROIDS")
    score_init()

    E.enable_idle(true)
    return true
end

-------------------------------------------------------------------------------

function do_timer(dt)

    local rot_x, rot_y, rot_z = E.entity_get_rotation(space)
    local scale = 2.0 + 0.5 * math.sin(time);

    time = time + dt

    E.entity_rotation(space, rot_x, rot_y + dt, rot_z)

    wait_time = wait_time + dt

    if overlay then E.entity_scale(overlay, scale, scale, scale) end

    if global_dt > 0 then

        global_dt = dt

        if state == "ready" then
            stuff_step()
            player_step()
        end

        if state == "play" then
            level_step()
            player_step()
        end

        if state == "done" then
            stuff_step()
            player_step()
        end

        if state == "dead" then
            level_step()
        end
    end

    return true
end

function do_joystick(n, b, s)
    
    if state == "start" then
        if b == button_fire and s then
            curr_score_set(0)
            curr_score = 0
            player_reset()
            speed = 7
            ships = 3
            level = 1
            level_init()
            state = "ready"
        end

    elseif state == "ready" then
        if b == button_fire and s then
            state = "play"
        end

    elseif state == "play" then
        if b == button_fire and s then
            add_bullet()
        end
        if b == button_thrust then
            thrusting = s
        end

    elseif state == "dead" then
        if wait_time > 1 and b == button_fire and s then
            if ships > 0 then
                ships = ships - 1
                player_reset()
                state = "play"
            else
                state = "over"
            end
        end

    elseif state == "done" then
        if b == button_thrust then
            thrusting = s
        end
        if wait_time > 1 and b == button_fire and s then
            speed = speed + 2
            level = level + 1
            level_init()
            state = "ready"
        end

    elseif state == "over" then
        if wait_time > 1 and s then
            level_stop()
            state = "start"
        end
    end

    return true
end

-------------------------------------------------------------------------------
