
key = { 0, 0 }

speed = 2
scale = 0.1
magn  = 100

-------------------------------------------------------------------------------

function do_start()
    local x, y, w, h = E.get_display_union()

    local brush = E.create_brush()
        
    E.set_brush_frag_prog(brush, "../data/star.fp")
    E.set_brush_vert_prog(brush, "../data/star.vp")

    camera  = E.create_camera(E.camera_type_perspective)
    galaxy  = E.create_galaxy("../data/galaxy_hip.gal", brush)
    object  = E.create_object("../data/orion.obj")
    pointer = E.create_pivot()

    magn = w / 8

    E.set_galaxy_magnitude(galaxy, magn)
    E.parent_entity(galaxy, camera)
    E.parent_entity(object, camera)

    E.set_entity_scale(galaxy, scale, scale, scale)
    E.set_entity_scale(object, scale, scale, scale)

    E.set_entity_tracking(pointer, 1, E.tracking_mode_local)
    E.set_entity_flags(pointer, E.entity_flag_track_pos, true)
    E.set_entity_flags(pointer, E.entity_flag_track_rot, true)
    E.set_entity_flags(object,  E.entity_flag_line_smooth,   true)

    E.set_brush_flags(E.get_mesh(object, 0), E.brush_flag_unlit, true)

    E.set_background(0, 0, 0)

    E.enable_timer(true)
end

function do_timer(dt)
    local joy = { E.get_joystick(0)              }
    local dir = { E.get_entity_z_vector(pointer) }

    -- Map the arrow keys on to joystick input.

    if math.abs(key[1]) > math.abs(joy[1]) then joy[1] = key[1] end
    if math.abs(key[2]) > math.abs(joy[2]) then joy[2] = key[2] end

    -- Turn about the Y axis.

    if joy[1] < -0.1 or 0.1 < joy[1] then
        E.turn_entity(camera, 0, -90 * joy[1] * dt, 0)
    end

    -- Move along the hand vector.

    if joy[2] < -0.1 or 0.1 < joy[2] then
        E.move_entity(camera, -speed * dir[1] * joy[2] * dt,
                              -speed * dir[2] * joy[2] * dt,
                              -speed * dir[3] * joy[2] * dt)
    end

    return true
end

function do_keyboard(k, s)
    if varrier_keyboard then
        if varrier_keyboard(k, s, camera) then
            return true
        end
    end

    if s then
        -- Arrow keys simulate joystick input.

        if k == E.key_right then key[1] = key[1] + 1 end
        if k == E.key_left  then key[1] = key[1] - 1 end
        if k == E.key_up    then key[2] = key[2] + 1 end
        if k == E.key_down  then key[2] = key[2] - 1 end

        -- Page up and page down control the galaxy magnitude.

        if k == E.key_pageup then
            magn = magn + 10
            E.set_galaxy_magnitude(galaxy, magn)
        end
        if k == E.key_pagedown then
            magn = magn - 10
            E.set_galaxy_magnitude(galaxy, magn)
        end

        -- Recenter the view on Return

        if k == E.key_return then
            E.set_entity_position(camera, 0, 0, 0)
            E.set_entity_rotation(camera, 0, 0, 0)
        end

        -- Return to the demo menu on F12

        if k == E.key_F12 then
            E.nuke()
            E.chdir("..")
            dofile("demo.lua")
        end

        return true
    else
        -- Arrow keys simulate joystick input.

        if k == E.key_right then key[1] = key[1] - 1 end
        if k == E.key_left  then key[1] = key[1] + 1 end
        if k == E.key_up    then key[2] = key[2] - 1 end
        if k == E.key_down  then key[2] = key[2] + 1 end

        return false
    end
end

-------------------------------------------------------------------------------

do_start()
