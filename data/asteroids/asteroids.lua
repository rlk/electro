
-------------------------------------------------------------------------------

global_ship   = nil
global_rock   = nil
global_bullet = nil

serial    = 1
camera    = nil
pivot     = nil
light     = nil
asteroids = { }
bullets   = { }

level     = 1
score     = 0
speed     = 5

-------------------------------------------------------------------------------

global_l = 0
global_r = 0
global_b = 0
global_t = 0
global_w = 0
global_h = 0
global_z = 1

-------------------------------------------------------------------------------

function add_asteroid(entity, size)

    local a  = math.rad(math.random(-180, 180))
    local dx = math.sin(a)
    local dy = math.cos(a)
    local x  = dx * global_h * 0.4
    local y  = dy * global_h * 0.4
    local z  = 0

    if entity then
        x, y, z = entity_get_position(entity)
    end

    local asteroid = { }

    asteroid.object = entity_clone(global_rock)
    asteroid.size   = size
    asteroid.dx     = dx * speed
    asteroid.dy     = dy * speed
    asteroid.wx     = math.random(-90, 90)
    asteroid.wy     = math.random(-90, 90)

    entity_scale   (asteroid.object, size / 3, size / 3, size / 3)
    entity_parent  (asteroid.object, light)
    entity_position(asteroid.object, x , y, 0)

    table.insert(asteroids, asteroid)
end

function del_asteroid(id, asteroid)
    entity_delete(asteroid.object)

    table.remove(asteroids, id)
end

-------------------------------------------------------------------------------

function add_bullet()

    local pos_x, pos_y, pos_z = entity_get_position(ship)
    local rot_x, rot_y, rot_z = entity_get_rotation(ship)

    local a = -math.rad(rot_z)
    local x =  math.sin(a)
    local y =  math.cos(a)

    local bullet = { }

    bullet.object = entity_clone(global_bullet)
    bullet.dx     = x * 15
    bullet.dy     = y * 15

    entity_position(bullet.object, pos_x + x,
                                   pos_y + y, pos_z)

    entity_scale (bullet.object, 1 / 64, 1 / 64, 1 / 64)
    entity_parent(bullet.object, pivot)

    table.insert(bullets, bullet)
end

function del_bullet(id, bullet)
    entity_delete(bullet.object)

    table.remove(bullets, id)
end

-------------------------------------------------------------------------------

global_dt = 0

function asteroid_step(id, asteroid)

    local object = asteroid.object
    local size   = asteroid.size
    local dx     = asteroid.dx
    local dy     = asteroid.dy
    local wx     = asteroid.wx
    local wy     = asteroid.wy

    if object then

        -- Find the asteroid's new position and orientation.
        
        local pos_x, pos_y, pos_z = entity_get_position(object)
        local rot_x, rot_y, rot_z = entity_get_rotation(object)

        pos_x = pos_x + dx * global_dt
        pos_y = pos_y + dy * global_dt
        rot_x = rot_x + wx * global_dt
        rot_y = rot_y + wy * global_dt

        -- Wrap the position to the viewport.

        if pos_x < global_l then
            pos_x = pos_x + global_w
        end
        if pos_x > global_r then
            pos_x = pos_x - global_w
        end
        if pos_y < global_b then
            pos_y = pos_y + global_h
        end
        if pos_y > global_t then
            pos_y = pos_y - global_h
        end

        -- Wrap the orientation to (-180, 180).

        if rot_x < -180 then
            rot_x = rot_x + 360
        end
        if rot_x >  180 then
            rot_x = rot_x - 360
        end
        if rot_y < -180 then
            rot_y = rot_y + 360
        end
        if rot_y >  180 then
            rot_y = rot_y - 360
        end

        -- Apply the new position and orientation.

        entity_position(object, pos_x, pos_y, pos_z)
        entity_rotation(object, rot_x, rot_y, rot_z)

        -- Check this asteroid against all bullets.

        for jd, bullet in pairs(bullets) do
            local pos_X, pos_Y, pos_Z = entity_get_position(bullet.object)

            local dist = math.sqrt((pos_X - pos_x) * (pos_X - pos_x) +
                                   (pos_Y - pos_y) * (pos_Y - pos_y));
            if dist < size then

                -- Break a big asteroid into smaller asteroids.

                if size > 1 then
                    add_asteroid(object, size - 1)
                    add_asteroid(object, size - 1)
                    add_asteroid(object, size - 1)
                end

                -- Delete the original asteroid and the bullet.

                del_asteroid(id, asteroid)
                del_bullet  (jd, bullet)
            end
        end
    end
end

function bullet_step(id, bullet)

    local object = bullet.object
    local dx     = bullet.dx
    local dy     = bullet.dy

    if object then
        -- Find the bullet's new position.
        
        local pos_x, pos_y, pos_z = entity_get_position(object)

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
            entity_position(object, pos_x, pos_y, pos_z)
        end
    end
end

-------------------------------------------------------------------------------

function level_init()
    for i = 1, level + 3 do
        add_asteroid(nil, 3)
    end

    entity_position(ship, 0, 0, 0);
end

function level_step()
    table.foreach(asteroids, asteroid_step)
    table.foreach(bullets,   bullet_step)
end

-------------------------------------------------------------------------------

function player_step()

    local rot_x, rot_y, rot_z = entity_get_rotation(ship)
    local changed = false

    if (joystick_axis(0, 0) < -0.5) then
        rot_z = rot_z + global_dt * 180
        changed = true
    end
    if (joystick_axis(0, 0) >  0.5) then
        rot_z = rot_z - global_dt * 180
        changed = true
    end

    if changed then entity_rotation(ship, rot_x, rot_y, rot_z) end
end

-------------------------------------------------------------------------------

function do_start()

    math.randomseed(os.time())

    -- Establish the boundries of the viewport.

    global_l, global_r, global_b, global_t = viewport_get()

    global_z = 64 / (global_r - global_l)
    
    global_l = global_l * global_z
    global_r = global_r * global_z
    global_b = global_b * global_z
    global_t = global_t * global_z
    global_w = global_r - global_l
    global_h = global_t - global_b

    -- Initialize some source objects.

    global_ship   = object_create("ship.obj")
    global_rock   = object_create("rock.obj")
    global_bullet = sprite_create("bullet.png")

    entity_flag(global_ship,   1, 1);
    entity_flag(global_rock,   1, 1);
    entity_flag(global_bullet, 1, 1);

    -- Initialize the scene.

    ship   = entity_clone(global_ship);
    camera = camera_create(1)
    pivot  = pivot_create( )
    light  = light_create(1)

    entity_parent(pivot, camera)
    entity_parent(light, camera)
    entity_parent(ship,  light)

    entity_position(light, 0, 0, 10)

    camera_zoom(camera, global_z)

    level_init()

    enable_idle(true)

    return true
end

function do_timer(dt)

    global_dt = dt

    if global_dt > 0 then
        level_step()
        player_step()
        return true
    else
        return false
    end
end

function do_joystick(n, b, s)
    if s and b == 1 then
        add_bullet()
    end
end

-------------------------------------------------------------------------------
