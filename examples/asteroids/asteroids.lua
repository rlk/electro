
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

button_fire   = 2
button_thrust = 3

-------------------------------------------------------------------------------

state     = "none"

init      = { }
init_flag = true

time  = 0
fire  = nil
boom  = nil
music = nil
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

    E.entity_flag(thrust, E.entity_flag_hidden, not thrusting)

    if ship then
        E.entity_flag(ship, E.entity_flag_hidden, false)

        E.entity_position(ship, 0, 0, 0)
        E.entity_rotation(ship, 0, 0, 0)
    end
end

-------------------------------------------------------------------------------

function spare_set(n)
    for i = 1, 5 do
        E.entity_flag(spares[i], E.entity_flag_hidden, (n < i))
    end
end

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
    E.entity_parent  (asteroid.entity, light2)
    E.entity_position(asteroid.entity, x, y, 0)

    table.insert(asteroids, asteroid)
    undone = undone + 1
end

function del_asteroid(id, asteroid)
    E.entity_delete(asteroid.entity)
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

function add_explosion(entity, source, size)
    local explosion = { }
    local scale = 0.05 * size / 3.0

    -- Clone a new explosion sprite.

    explosion.entity = E.entity_clone(source)

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

function add_shockwave(entity, source, size)
    local shockwave = { }
    local scale = 0.01 * size / 3.0

    -- Clone a new shockwave sprite.

    shockwave.entity = E.entity_clone(source)

    -- Add the new sprite to the scene.

    local x, y, z = E.entity_get_position(entity)

    E.entity_scale   (shockwave.entity, scale, scale, scale)
    E.entity_parent  (shockwave.entity, below)
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

    -- Award a free ship

    if curr_score > free_score then
        ships = ships + 1
        spare_set(ships)
        free_score = free_score + 1000
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
            add_explosion(ship, global_ship_explosion, 5)
            add_shockwave(ship, global_ship_shockwave, 5)

            E.entity_flag(ship, E.entity_flag_hidden, true)

            velocity_dx = 0
            velocity_dy = 0
            goto_state("dead")
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

function enter_state(s)
    if     s == "title" then
        curr_score = 0
        free_score = 1000
        speed      = 7
        ships      = 2
        level      = 1
        spare_set(ships)
        E.entity_flag(overlay_title, E.entity_flag_hidden, false)

    elseif s == "ready" then

        digit_set(overlay_level, level)
        spare_set(ships)
        level_init()
        player_reset()
        curr_score_set(curr_score)

        E.entity_flag(overlay_ready, E.entity_flag_hidden, false)
        E.entity_flag(overlay_level, E.entity_flag_hidden, false)

    elseif s == "play"  then

    elseif s == "dead"  then

    elseif s == "clear" then
        E.entity_flag(overlay_clear, E.entity_flag_hidden, false)

    elseif s == "high" then
        init_flag = true
        high_score = curr_score
        high_score_set(high_score)
        index = 1
        E.entity_scale(overlay_init[index], 0.004, 0.008, 0.008)
        E.entity_flag(overlay_high, E.entity_flag_hidden,  false)

    elseif s == "over"  then
        E.entity_flag(overlay_over,  E.entity_flag_hidden, false)

    end
end

function leave_state(s)
    if     s == "title" then
        E.entity_flag(overlay_title, E.entity_flag_hidden, true)

    elseif s == "ready" then
        E.entity_flag(overlay_ready, E.entity_flag_hidden, true)
        E.entity_flag(overlay_level, E.entity_flag_hidden, true)

    elseif s == "play"  then

    elseif s == "dead"  then

    elseif s == "clear" then
        E.entity_flag(overlay_clear, E.entity_flag_hidden, true)

    elseif s == "high" then
        E.entity_flag(overlay_high,  E.entity_flag_hidden, true)
        E.entity_scale(overlay_init[index], 0.004, 0.008, 0.008)

        local fout = io.open(high_score_file, "w")
        if fout then
            fout:write(high_score, init[1], init[2], init[3])
            fout:close()
        end

    elseif s == "over"  then
        E.entity_flag(overlay_over,  E.entity_flag_hidden, true)
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

        E.entity_scale(overlay_init[index], scale / 2, scale, scale)

        if      E.joystick_axis(0, 0) >  0.5 then
            if  init_flag then
                init_flag = false

                if byte >= 90 then
                    init[index] = "A"
                else
                    init[index] = string.char(byte + 1)
                end
                alpha_set(overlay_init[index], init[index])
            end

        elseif  E.joystick_axis(0, 0) < -0.5 then
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
    end
    if music then
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
    ship   = E.entity_clone(global_ship)
    thrust = E.create_sprite("thrust.png")
    galaxy = E.create_galaxy("../hip_main.bin")

    for i = 1, 5 do
        spares[i] = E.entity_clone(global_ship)
        E.entity_parent  (spares[i], light2)
        E.entity_position(spares[i], global_r - 3 * i, global_b + 3, 0)
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

    E.entity_parent(above,  camera)
    E.entity_parent(below,  camera)
    E.entity_parent(light1, camera)
    E.entity_parent(light2, light1)
    E.entity_parent(ship,   light2)
    E.entity_parent(thrust, ship)
    E.entity_parent(galaxy, space)

    E.entity_scale(thrust, 0.05, 0.05, 0.05)
    E.entity_flag(thrust, E.entity_flag_unlit,  true)
    E.entity_flag(thrust, E.entity_flag_hidden, true)

    E.entity_parent(overlay_title,   below)
    E.entity_parent(overlay_level,   below)
    E.entity_parent(overlay_ready,   below)
    E.entity_parent(overlay_clear,   below)
    E.entity_parent(overlay_high,    below)
    E.entity_parent(overlay_over,    below)
    E.entity_parent(overlay_init[1], below)
    E.entity_parent(overlay_init[2], below)
    E.entity_parent(overlay_init[3], below)

    E.entity_scale(overlay_title,   0.07, 0.07, 0.07)
    E.entity_scale(overlay_level,   0.02, 0.02, 0.02)
    E.entity_scale(overlay_ready,   0.07, 0.07, 0.07)
    E.entity_scale(overlay_clear,   0.07, 0.07, 0.07)
    E.entity_scale(overlay_high,    0.07, 0.07, 0.07)
    E.entity_scale(overlay_over,    0.07, 0.07, 0.07)
    E.entity_scale(overlay_init[1], 0.004, 0.008, 0.008)
    E.entity_scale(overlay_init[2], 0.004, 0.008, 0.008)
    E.entity_scale(overlay_init[3], 0.004, 0.008, 0.008)

    E.entity_flag(overlay_ready, E.entity_flag_hidden, true)
    E.entity_flag(overlay_level, E.entity_flag_hidden, true)
    E.entity_flag(overlay_clear, E.entity_flag_hidden, true)
    E.entity_flag(overlay_high,  E.entity_flag_hidden, true)
    E.entity_flag(overlay_over,  E.entity_flag_hidden, true)

    E.entity_position(overlay_init[1], global_l + 16.0, global_t - 2.5, 0)
    E.entity_position(overlay_init[2], global_l + 19.0, global_t - 2.5, 0)
    E.entity_position(overlay_init[3], global_l + 22.0, global_t - 2.5, 0)
    
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

    E.entity_position(light1,  10, 0, 10)
    E.entity_position(light2, -10, 0, 10)

    E.light_color(light1, 1.0, 0.8, 0.5)
    E.light_color(light2, 0.5, 0.8, 1.0)

    E.camera_zoom(camera, global_z)

    E.galaxy_magn(galaxy, 100.0)
    E.camera_dist(space,  100.0)
    E.camera_zoom(space,    0.5)
    E.entity_position(space, 0, 15.5, 9200)

    goto_state("title")
    E.enable_idle(true)

    return true
end

-------------------------------------------------------------------------------

function do_timer(dt)

    global_dt = dt

    state_time = state_time + dt
    total_time = total_time + dt

    E.camera_dist(space, 100 * math.sin(total_time / 10))
    E.entity_rotation(space, 0, total_time, 0)

    timer_state(state)

    return true
end

function do_joystick(n, b, s)
    
    if b == button_thrust then
        thrusting = s
        E.entity_flag(thrust, E.entity_flag_hidden, not thrusting)

        if state == "high" and index > 1 then
            E.entity_scale(overlay_init[index], 0.004, 0.008, 0.008)
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
            E.entity_scale(overlay_init[index], 0.004, 0.008, 0.008)
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
