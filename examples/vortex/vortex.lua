
key = { 0, 0 }

speed     = 2
scale     = 0.1
lines     = true
rotate    = false
magnitude = 100

-------------------------------------------------------------------------------

function do_start()
    local x, y, w, h = E.get_display_union()

    magnitude = w / 8

    -- Prepare star and sign brushes.

    local star_brush = E.create_brush()
    E.set_brush_frag_prog(star_brush, "../data/star.fp")
    E.set_brush_vert_prog(star_brush, "../data/star.vp")

    local sign_image = E.create_image("../data/sunsign.png")
    local sign_brush = E.create_brush()
    E.set_brush_image(sign_brush, sign_image)
    E.set_brush_flags(sign_brush, E.brush_flag_unlit, true)

    -- Build the scene graph.

    camera  = E.create_camera(E.camera_type_perspective)
    galaxy  = E.create_galaxy("../data/galaxy_hip.gal", star_brush)
    orion   = E.create_object("../data/orion.obj")
    sign    = E.create_sprite(sign_brush)
    pointer = E.create_pivot()

    E.set_galaxy_magnitude(galaxy, magnitude)
    E.parent_entity(sign,   camera)
    E.parent_entity(galaxy, camera)
    E.parent_entity(orion,  camera)

    E.set_entity_scale(galaxy, scale, scale, scale)
    E.set_entity_scale(orion,  scale, scale, scale)
    E.set_entity_scale(sign,   0.005, 0.005, 0.005)
    E.set_entity_flags(orion,  E.entity_flag_line_smooth, true)
    E.set_entity_flags(sign,   E.entity_flag_billboard,   true)

    E.set_brush_flags(E.get_mesh(orion, 0), E.brush_flag_unlit, true)
    E.set_brush_line_width(E.get_mesh(orion, 0), 8)

    -- Enable tracking on the pointer.

    E.set_entity_tracking(pointer, 1, E.tracking_mode_local)
    E.set_entity_flags(pointer, E.entity_flag_track_pos, true)
    E.set_entity_flags(pointer, E.entity_flag_track_rot, true)

    E.set_background(0, 0, 0)

    E.enable_timer(true)
end

function do_timer(dt)
    local joy = { E.get_joystick(0)              }
    local dir = { E.get_entity_z_vector(pointer) }

    if rotate then
        -- Turn abouth the Y axis.

        if joy[1] < -0.1 or 0.1 < joy[1] then
            E.turn_entity(camera, 0, -90 * joy[1] * dt, 0)
        end

        -- Turn abouth the X axis.

        if joy[2] < -0.1 or 0.1 < joy[2] then
            E.turn_entity(camera, -90 * joy[2] * dt, 0, 0)
        end
        
    else
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
    end

    return true
end

function do_joystick(d, b, s)

    -- Button 1 resets the view.

    if b == 1 and s then
        E.set_entity_position(camera, 0, 0, 0)
        E.set_entity_rotation(camera, 0, 0, 0)
    end

    -- Button 2 toggles rotate mode.

    if b == 2 then
        rotate = s
    end

    -- Button 3 toggles the constellations.

    if b == 3 and s then
        lines = not lines
        E.set_entity_flags(orion, E.entity_flag_hidden, not lines)
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
            magnitude = magnitude + 10
            E.set_galaxy_magnitude(galaxy, magnitude)
        end
        if k == E.key_pagedown then
            magnitude = magnitude - 10
            E.set_galaxy_magnitude(galaxy, magnitude)
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

        return true
    end
end

-------------------------------------------------------------------------------

do_start()
