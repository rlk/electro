
high_score_file = "asteroids.dat"

sound = true
music = false

-------------------------------------------------------------------------------

global_ship      = nil
global_rock      = nil
global_bullet    = nil
global_score05   = nil
global_score10   = nil
global_score25   = nil

global_ship_explosion = nil
global_ship_shockwave = nil
global_rock_explosion = nil
global_rock_shockwave = nil

button_fire   = 0
button_thrust = 1

-------------------------------------------------------------------------------

sound_bullet          = nil
sound_free            = nil
sound_rock1_explosion = nil
sound_rock2_explosion = nil
sound_rock3_explosion = nil
sound_ship_explosion  = nil
sound_thrust1         = nil
sound_thrust2         = nil
sound_thrust3         = nil
music                 = nil

-------------------------------------------------------------------------------

state     = "none"

init      = { }
init_flag = true

time  = 0
index = 0

serial    = 1
space     = nil
galaxy    = nil
camera    = nil
above     = nil
below     = nil
light1    = nil
light2    = nil
ship      = nil
thrust    = nil

undone     = 0
bullets    = { }
asteroids  = { }
explosions = { }
shockwaves = { }
scores     = { }
spares     = { }

thrusting   = false
velocity_dx = 0
velocity_dy = 0

level      = 1
ships      = 0
curr_score = 0
free_score = 0
high_score = 100
speed      = 7

total_time = 0
state_time = 0

overlay_title = nil
overlay_ready = nil
overlay_level = nil
overlay_clear = nil
overlay_high  = nil
overlay_over  = nil
overlay_init  = { }

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

    E.stop_sound(sound_thrust2)
    E.set_entity_flag(thrust, E.entity_flag_hidden, not thrusting)

    if ship then
        E.set_entity_flag(ship, E.entity_flag_hidden, false)

        E.set_entity_position(ship, 0, 0, 0)
        E.set_entity_rotation(ship, 0, 0, 0)
    end
end

-------------------------------------------------------------------------------

function spare_set(n)
    for i = 1, 5 do
        E.set_entity_flag(spares[i], E.entity_flag_hidden, (n < i))
    end
end

function digit_set(sprite, n)
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

function alpha_set(sprite, c)
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

function init_set()
    alpha_set(overlay_init[1], init[1])
    alpha_set(overlay_init[2], init[2])
    alpha_set(overlay_init[3], init[3])
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

        E.parent_entity      (curr_score_digit[i], camera)
        E.set_entity_scale   (curr_score_digit[i], scale, scale, scale);
        E.set_entity_position(curr_score_digit[i], curr_r - dx * (i + 1),
                                                 global_t - dy, 0)
    end

    -- Create the high score display.

    for i = 0, 4 do
        high_score_digit[i] = E.create_sprite("digit.png")

        E.parent_entity      (high_score_digit[i], camera)
        E.set_entity_scale   (high_score_digit[i], scale, scale, scale);
        E.set_entity_position(high_score_digit[i], high_r - dx * (i + 1),
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
        x, y, z = E.get_entity_position(entity)
    else
        x = dx * global_h * 0.4
        y = dy * global_h * 0.4
        z = 0
    end

    -- Clone a new asteroid object and define its motion parameters.

    asteroid.entity = E.create_clone(global_rock)
    asteroid.size   = size
    asteroid.dx     = dx * speed
    asteroid.dy     = dy * speed
    asteroid.wx     = math.random(-90, 90)
    asteroid.wy     = math.random(-90, 90)

    -- Add the new asteroid to the scene.

    E.parent_entity      (asteroid.entity, light2)
    E.set_entity_scale   (asteroid.entity, scale, scale, scale)
    E.set_entity_position(asteroid.entity, x, y, 0)

    table.insert(asteroids, asteroid)
    undone = undone + 1
end

function del_asteroid(id, asteroid)
    E.delete_entity(asteroid.entity)
    table.remove(asteroids, id)
    undone = undone - 1

    if undone == 0 then
        return true
    else
        return false
    end
end

-------------------------------------------------------------------------------

function add_bullet()
    local bullet = { }
    local scale  = 1 / 64

    -- Clone a new bullet sprite.

    bullet.entity = E.create_clone(global_bullet)

    -- Compute the bullet's position and velocity from the ship's location.

    local pos_x, pos_y, pos_z = E.get_entity_position(ship)
    local rot_x, rot_y, rot_z = E.get_entity_rotation(ship)

    local a = -math.rad(rot_z)
    local x =  math.sin(a)
    local y =  math.cos(a)

    bullet.dx = x * 15 + velocity_dx
    bullet.dy = y * 15 + velocity_dy

    -- Add the new sprite to the scene.

    E.parent_entity      (bullet.entity, below)
    E.set_entity_scale   (bullet.entity, scale, scale, scale)
    E.set_entity_position(bullet.entity, pos_x + x, pos_y + y, pos_z)

    table.insert(bullets, bullet)

    -- Bullets cost one point.

    if curr_score > 0 then
        curr_score = curr_score - 1
        curr_score_set(curr_score)
    end

    if sound then E.play_sound(sound_bullet) end
end

function del_bullet(id, bullet)
    E.delete_entity(bullet.entity)
    table.remove(bullets, id)
end

-------------------------------------------------------------------------------

function add_explosion(entity, source, size)
    local explosion = { }
    local scale = 0.05 * size / 3.0

    -- Clone a new explosion sprite.

    explosion.entity = E.create_clone(source)

    -- Add the new sprite to the scene.

    local x, y, z = E.get_entity_position(entity)

    E.parent_entity      (explosion.entity, above)
    E.set_entity_scale   (explosion.entity, scale, scale, scale)
    E.set_entity_position(explosion.entity, x, y, 0)

    table.insert(explosions, explosion)

    if size == 1 then E.play_sound(sound_rock1_explosion) end
    if size == 2 then E.play_sound(sound_rock2_explosion) end
    if size == 3 then E.play_sound(sound_rock3_explosion) end
    if size == 5 then E.play_sound(sound_ship_explosion) end
end

function del_explosion(id, explosion)
    E.delete_entity(explosion.entity)
    table.remove(explosions, id)
end

-------------------------------------------------------------------------------

function add_shockwave(entity, source, size)
    local shockwave = { }
    local scale = 0.01 * size / 3.0

    -- Clone a new shockwave sprite.

    shockwave.entity = E.create_clone(source)

    -- Add the new sprite to the scene.

    local x, y, z = E.get_entity_position(entity)

    E.parent_entity      (shockwave.entity, below)
    E.set_entity_scale   (shockwave.entity, scale, scale, scale)
    E.set_entity_position(shockwave.entity, x, y, 0)

    table.insert(shockwaves, shockwave)
end

function del_shockwave(id, shockwave)
    E.delete_entity(shockwave.entity)
    table.remove(shockwaves, id)
end

-------------------------------------------------------------------------------

function add_score(entity, value)
    local score = { }
    local scale = 0.01

    -- Tally the score

    curr_score = curr_score + value
    curr_score_set(curr_score)

    -- Award a free ship

    if curr_score > free_score then
        ships = ships + 1
        spare_set(ships)
        free_score = free_score + 1000
        E.play_sound(sound_free)
    end

    -- Select the correct sprite for the given value.

    if value ==  5 then score.entity = E.create_clone(global_score05) end
    if value == 10 then score.entity = E.create_clone(global_score10) end
    if value == 25 then score.entity = E.create_clone(global_score25) end

    -- Add the new sprite to the scene.

    local x, y, z = E.get_entity_position(entity)

    E.parent_entity      (score.entity, above)
    E.set_entity_scale   (score.entity, scale, scale, scale)
    E.set_entity_position(score.entity, x, y, 0)

    table.insert(scores, score)
end

function del_score(id, score)
    E.delete_entity(score.entity)
    table.remove(scores, id)
end

-------------------------------------------------------------------------------

global_dt = 0

function asteroid_frag(id, asteroid)

    local entity = asteroid.entity
    local size   = asteroid.size

    -- Break a big asteroid into smaller asteroids.

    add_shockwave(entity, global_rock_shockwave, size)
    add_explosion(entity, global_rock_explosion, size)

    if size > 1 then
        add_asteroid(entity, size - 1)
        add_asteroid(entity, size - 1)
        add_asteroid(entity, size - 1)
    end

    if size == 3 then add_score(entity, 25) end
    if size == 2 then add_score(entity, 10) end
    if size == 1 then add_score(entity,  5) end

    -- Delete the original asteroid.

    if del_asteroid(id, asteroid) then
        goto_state("clear")
    end
end

function asteroid_step(id, asteroid)

    local entity = asteroid.entity
    local size   = asteroid.size
    local dx     = asteroid.dx
    local dy     = asteroid.dy
    local wx     = asteroid.wx
    local wy     = asteroid.wy

    -- Find the asteroid's new position and orientation.
    
    local pos_x, pos_y, pos_z = E.get_entity_position(entity)
    local rot_x, rot_y, rot_z = E.get_entity_rotation(entity)

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

    E.set_entity_position(entity, pos_x, pos_y, pos_z)
    E.set_entity_rotation(entity, rot_x, rot_y, rot_z)

    -- Check this asteroid against all bullets.

    for jd, bullet in pairs(bullets) do
        local pos_X, pos_Y, pos_Z = E.get_entity_position(bullet.entity)

        local dist = math.sqrt((pos_X - pos_x) * (pos_X - pos_x) +
                               (pos_Y - pos_y) * (pos_Y - pos_y));
        if dist < size then
            asteroid_frag(id, asteroid)
            del_bullet   (jd, bullet)
        end
    end

    -- Check this asteroid against the player.

    if state == "play" and ship then
        local pos_X, pos_Y, pos_Z = E.get_entity_position(ship)

        local dist = math.sqrt((pos_X - pos_x) * (pos_X - pos_x) +
                               (pos_Y - pos_y) * (pos_Y - pos_y));

        if dist < size + 0.5 then
            add_explosion(ship, global_ship_explosion, 5)
            add_shockwave(ship, global_ship_shockwave, 5)

            E.set_entity_flag(ship, E.entity_flag_hidden, true)

            velocity_dx = 0
            velocity_dy = 0
            E.stop_sound(sound_thrust2)
            goto_state("dead")
        end
    end
end

function bullet_step(id, bullet)

    local entity = bullet.entity
    local dx     = bullet.dx
    local dy     = bullet.dy

    -- Find the bullet's new position.
    
    local pos_x, pos_y, pos_z = E.get_entity_position(entity)

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
        E.set_entity_position(entity, pos_x, pos_y, pos_z)
    end
end

function explosion_step(id, explosion)
    local entity = explosion.entity

    -- Fade explosion sprites over 1 second.

    E.set_entity_alpha(entity, E.get_entity_alpha(entity) - global_dt)

    -- If this sprite is transparent, delete it.

    if E.get_entity_alpha(entity) < 0 then
        del_explosion(id, explosion)
    end
end

function shockwave_step(id, shockwave)
    local entity = shockwave.entity

    local scl_x, scl_y, scl_z = E.get_entity_scale(entity);

    -- Fade and scale shockwave sprites oven 1 second.

    E.set_entity_scale(entity, scl_x + global_dt * 0.05,
                               scl_y + global_dt * 0.05, scl_z)
    E.set_entity_alpha(entity, E.get_entity_alpha(entity) - global_dt)

    -- If this sprite is transparent, delete it.

    if E.get_entity_alpha(entity) < 0 then
        del_shockwave(id, shockwave)
    end
end

function score_step(id, score)
    local entity = score.entity

    -- Fade score sprites over 2 seconds.

    E.set_entity_alpha(entity, E.get_entity_alpha(entity) - global_dt / 3)
    
    -- If this sprite is transparent, delete it.

    if E.get_entity_alpha(entity) < 0 then
        del_score(id, score)
    end
end

-------------------------------------------------------------------------------

function level_stop()
    while undone > 0 do
        table.foreach(asteroids,  del_asteroid)
    end
end

function level_init()
    level_stop()
    for i = 1, level + 3 do
        add_asteroid(nil, 3)
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

    local pos_x, pos_y, pos_z = E.get_entity_position(ship)
    local rot_x, rot_y, rot_z = E.get_entity_rotation(ship)
    local joy_x, joy_y = E.get_joystick(0)

    -- Handle thrust

    if thrusting then
        local r = global_dt * 20

        velocity_dx = velocity_dx - math.sin(math.rad(rot_z)) * r
        velocity_dy = velocity_dy + math.cos(math.rad(rot_z)) * r
    end

    -- Handle rotation change

    if joy_x < -0.5 then
        rot_z = rot_z + global_dt * 180
    end
    if joy_x >  0.5 then
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

    E.set_entity_position(ship, pos_x, pos_y, pos_z)
    E.set_entity_rotation(ship, rot_x, rot_y, rot_z)
end

-------------------------------------------------------------------------------

function enter_state(s)
    if     s == "title" then
        curr_score = 0
        free_score = 1000
        speed      = 7
        ships      = 2
        level      = 1
        spare_set(ships)
        E.set_entity_flag(overlay_title, E.entity_flag_hidden, false)

    elseif s == "ready" then

        digit_set(overlay_level, level)
        spare_set(ships)
        level_init()
        player_reset()
        curr_score_set(curr_score)

        E.set_entity_flag(overlay_ready, E.entity_flag_hidden, false)
        E.set_entity_flag(overlay_level, E.entity_flag_hidden, false)

    elseif s == "play"  then

    elseif s == "dead"  then

    elseif s == "clear" then
        E.set_entity_flag(overlay_clear, E.entity_flag_hidden, false)

    elseif s == "high" then
        init_flag = true
        high_score = curr_score
        high_score_set(high_score)
        index = 1
        E.set_entity_scale(overlay_init[index], 0.004, 0.008, 0.008)
        E.set_entity_flag(overlay_high, E.entity_flag_hidden,  false)

    elseif s == "over"  then
        E.set_entity_flag(overlay_over,  E.entity_flag_hidden, false)

    end
end

function leave_state(s)
    if     s == "title" then
        E.set_entity_flag(overlay_title, E.entity_flag_hidden, true)

    elseif s == "ready" then
        E.set_entity_flag(overlay_ready, E.entity_flag_hidden, true)
        E.set_entity_flag(overlay_level, E.entity_flag_hidden, true)

    elseif s == "play"  then

    elseif s == "dead"  then

    elseif s == "clear" then
        E.set_entity_flag(overlay_clear, E.entity_flag_hidden, true)

    elseif s == "high" then
        E.set_entity_flag(overlay_high,  E.entity_flag_hidden, true)
        E.set_entity_scale(overlay_init[index], 0.004, 0.008, 0.008)

        local fout = io.open(E.get_directory()..high_score_file, "w")
        if fout then
            fout:write(high_score, init[1], init[2], init[3])
            fout:close()
        end

    elseif s == "over"  then
        E.set_entity_flag(overlay_over,  E.entity_flag_hidden, true)
    end
end

function timer_state(s)
    if     s == "title" then
        level_step()

    elseif s == "ready" then
        stuff_step()
        player_step()

    elseif s == "play"  then
        level_step()
        player_step()

    elseif s == "dead"  then
        level_step()

    elseif s == "clear" then
        stuff_step()
        player_step()

    elseif s == "high"  then
        local byte  = string.byte(init[index])
        local scale = 0.01 + 0.001 * math.sin(state_time * 8)

        E.set_entity_scale(overlay_init[index], scale / 2, scale, scale)

        if      E.get_joystick(0, 0) >  0.5 then
            if  init_flag then
                init_flag = false

                if byte >= 90 then
                    init[index] = "A"
                else
                    init[index] = string.char(byte + 1)
                end
                alpha_set(overlay_init[index], init[index])
            end

        elseif  E.get_joystick(0, 0) < -0.5 then
            if init_flag then
                init_flag = false

                if byte <= 65 then
                    init[index] = "Z"
                else
                    init[index] = string.char(byte - 1)
                end
                alpha_set(overlay_init[index], init[index])
            end

        else
            init_flag = true
        end

    elseif s == "over"  then
        level_step()

    end
end

function goto_state(s)
    leave_state(state)
    state       = s
    state_time  = 0
    enter_state(state)
end

-------------------------------------------------------------------------------

function new_global_object(filename)
    object = E.create_object(filename)
    E.set_entity_flag(object, E.entity_flag_hidden, true)

    return object
end

function new_global_sprite(filename)
    sprite = E.create_sprite(filename)
    E.set_entity_flag(sprite, E.entity_flag_hidden, true)

    return sprite
end

-------------------------------------------------------------------------------

function do_start()

    math.randomseed(os.time())

    if sound then
        sound_bullet          = E.load_sound("bullet.ogg")
        sound_free            = E.load_sound("free.ogg")
        sound_rock1_explosion = E.load_sound("rock1_explosion.ogg")
        sound_rock2_explosion = E.load_sound("rock2_explosion.ogg")
        sound_rock3_explosion = E.load_sound("rock3_explosion.ogg")
        sound_ship_explosion  = E.load_sound("ship_explosion.ogg")
        sound_thrust1         = E.load_sound("thrust1.ogg")
        sound_thrust2         = E.load_sound("thrust2.ogg")
        sound_thrust3         = E.load_sound("thrust3.ogg")
    end
    if music then
        music = E.load_sound("inter.ogg")
        E.loop_sound(music)
    end

    -- Establish the boundries of the viewport.

    global_l, global_r, global_b, global_t = E.get_viewport()

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

    global_ship_explosion = new_global_sprite("ship_explosion.png")
    global_ship_shockwave = new_global_sprite("ship_shockwave.png")
    global_rock_explosion = new_global_sprite("rock_explosion.png")
    global_rock_shockwave = new_global_sprite("rock_shockwave.png")

    -- Initialize all entities

    camera  = E.create_camera(E.camera_type_orthogonal)
    space   = E.create_camera(E.camera_type_perspective)

    light1 = E.create_light(E.light_type_positional)
    light2 = E.create_light(E.light_type_positional)
    above  = E.create_pivot()
    below  = E.create_pivot()
    ship   = E.create_clone(global_ship)
    thrust = E.create_sprite("thrust.png")
    galaxy = E.create_galaxy("../galaxy_hip.gal")

    for i = 1, 5 do
        spares[i] = E.create_clone(global_ship)
        E.parent_entity      (spares[i], light2)
        E.set_entity_position(spares[i], global_r - 3 * i, global_b + 3, 0)
    end

    overlay_title = E.create_sprite("title.png")
    overlay_ready = E.create_sprite("ready.png")
    overlay_level = E.create_sprite("digit.png")
    overlay_clear = E.create_sprite("clear.png")
    overlay_high  = E.create_sprite("high.png")
    overlay_over  = E.create_sprite("over.png")

    overlay_init[1] = E.create_sprite("alpha.png")
    overlay_init[2] = E.create_sprite("alpha.png")
    overlay_init[3] = E.create_sprite("alpha.png")

    -- Initialize the hierarchy

    E.parent_entity(above,  camera)
    E.parent_entity(below,  camera)
    E.parent_entity(light1, camera)
    E.parent_entity(light2, light1)
    E.parent_entity(ship,   light2)
    E.parent_entity(thrust, ship)
    E.parent_entity(galaxy, space)

    E.set_entity_scale(thrust, 0.05, 0.05, 0.05)
    E.set_entity_flag(thrust, E.entity_flag_unlit,  true)
    E.set_entity_flag(thrust, E.entity_flag_hidden, true)

    E.parent_entity(overlay_title,   below)
    E.parent_entity(overlay_level,   below)
    E.parent_entity(overlay_ready,   below)
    E.parent_entity(overlay_clear,   below)
    E.parent_entity(overlay_high,    below)
    E.parent_entity(overlay_over,    below)
    E.parent_entity(overlay_init[1], below)
    E.parent_entity(overlay_init[2], below)
    E.parent_entity(overlay_init[3], below)

    E.set_entity_scale(overlay_title,   0.07, 0.07, 0.07)
    E.set_entity_scale(overlay_level,   0.02, 0.02, 0.02)
    E.set_entity_scale(overlay_ready,   0.07, 0.07, 0.07)
    E.set_entity_scale(overlay_clear,   0.07, 0.07, 0.07)
    E.set_entity_scale(overlay_high,    0.07, 0.07, 0.07)
    E.set_entity_scale(overlay_over,    0.07, 0.07, 0.07)
    E.set_entity_scale(overlay_init[1], 0.004, 0.008, 0.008)
    E.set_entity_scale(overlay_init[2], 0.004, 0.008, 0.008)
    E.set_entity_scale(overlay_init[3], 0.004, 0.008, 0.008)

    E.set_entity_flag(overlay_ready, E.entity_flag_hidden, true)
    E.set_entity_flag(overlay_level, E.entity_flag_hidden, true)
    E.set_entity_flag(overlay_clear, E.entity_flag_hidden, true)
    E.set_entity_flag(overlay_high,  E.entity_flag_hidden, true)
    E.set_entity_flag(overlay_over,  E.entity_flag_hidden, true)

    E.set_entity_position(overlay_init[1], global_l + 16.0, global_t - 2.5, 0)
    E.set_entity_position(overlay_init[2], global_l + 19.0, global_t - 2.5, 0)
    E.set_entity_position(overlay_init[3], global_l + 22.0, global_t - 2.5, 0)
    
    -- Read the current high score

    score_init()

    high_score = 100
    init[1]    = "A"
    init[2]    = "A"
    init[3]    = "A"

    local fin = io.open(high_score_file, "r")
    if fin then
        local score  = fin:read("*n")
        local player = fin:read("*a")

        fin:close(fin)

        if score then
            high_score = score
            high_score_set(high_score)
        end

        if player then
            init[1] = string.sub(player, 1, 1)
            init[2] = string.sub(player, 2, 2)
            init[3] = string.sub(player, 3, 3)
        end
    end

    init_set()

    -- Initialize the view

    E.set_entity_position(light1,  10, 0, 10)
    E.set_entity_position(light2, -10, 0, 10)

    E.set_light_color(light1, 1.0, 0.8, 0.5)
    E.set_light_color(light2, 0.5, 0.8, 1.0)

    E.set_camera_zoom(camera, global_z)

    E.set_galaxy_magnitude(galaxy, 8.0 / global_z)
    E.set_camera_distance(space,  100.0)
    E.set_camera_zoom    (space,    0.5)

    goto_state("title")
    E.enable_timer(true)

    return true
end

-------------------------------------------------------------------------------

function do_timer(dt)

    global_dt = dt

    state_time = state_time + dt
    total_time = total_time + dt

    E.set_camera_distance(space, 100 * math.sin(total_time / 10))
    E.set_entity_rotation(space, 0, total_time, 0)

    timer_state(state)

    return true
end

function do_joystick(n, b, s)
    
    if b == button_thrust then
        thrusting = s
        E.set_entity_flag(thrust, E.entity_flag_hidden, not thrusting)

        if state == "ready" then
            goto_state("play")
        end

        if state == "play" then
            if thrusting then
                E.play_sound(sound_thrust1)
                E.loop_sound(sound_thrust2)
            else
                E.stop_sound(sound_thrust2)
                E.play_sound(sound_thrust3)
            end
        end

        if state == "high" and index > 1 then
            E.set_entity_scale(overlay_init[index], 0.004, 0.008, 0.008)
            index = index - 1
        end
    end

    if b == button_fire and s then
        if     state == "title" then
            goto_state("ready")

        elseif state == "ready" then
            goto_state("play")

        elseif state == "play" then
            add_bullet()

        elseif state == "dead" then
            if state_time > 1 then
                if ships > 0 then
                    ships = ships - 1
                    spare_set(ships)
                    player_reset()
                    goto_state("play")

                elseif curr_score > high_score then
                    goto_state("high")

                else
                    goto_state("over")
                end
            end

        elseif state == "clear" then
            if state_time > 1 then
                speed = speed + 2
                level = level + 1
                goto_state("ready")
            end

        elseif state == "high" then
            E.set_entity_scale(overlay_init[index], 0.004, 0.008, 0.008)
            if index < 3 then
                index = index + 1
            else
                goto_state("over")
            end

        elseif state == "over" then
            if state_time > 1 then
                goto_state("title")
            end
        end
    end

    return true
end

-------------------------------------------------------------------------------
