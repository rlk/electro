tumble = false
scale  = false

zoom  = 1
rot_x = 0
rot_y = 0
pan_x = 0
pan_y = 0
pan_z = 0

function add_object(i, s)
    local object = E.create_object(s)
    E.parent_entity(object, scene)
end

function do_start()
    local x, y, w, h = E.get_viewport()

    nearby = E.create_camera(E.camera_type_perspective)
    camera = E.create_camera(E.camera_type_perspective)
    light  = E.create_light(E.light_type_directional)
    scene  = E.create_pivot()
    hand   = E.create_pivot()

    E.parent_entity(light, camera)
    E.parent_entity(scene, light)
    E.parent_entity(pivot, scene)

    E.set_entity_position(light,  0.0,  8.0,   8.0)
    E.set_entity_position(scene,  0.0, -8.0, -16.0)

    E.set_entity_flag(hand, E.entity_flag_pos_tracked_1, true)
    E.set_entity_flag(hand, E.entity_flag_rot_tracked_1, true)

    table.foreach(E.argument, add_object)

    E.enable_timer(true)
end

function do_timer(dt)
    local joy_x, joy_y = E.get_joystick(0)

    if joy_x < -0.1 or 0.1 < joy_x then
        E.turn_entity(camera, 0, -joy_x * dt * 90, 0)
    end

    if joy_y < -0.1 or 0.1 < joy_y then
        mov_x, mov_y, mov_z = E.get_entity_z_vector(hand)
        E.move_entity(camera, -mov_x * joy_y * dt * 10,
                              -mov_y * joy_y * dt * 10,
                              -mov_z * joy_y * dt * 10)
    else
        E.turn_entity(camera, 0, -pan_x * dt * 90, 0)

        mov_x, mov_y, mov_z = E.get_entity_y_vector(hand)
        E.move_entity(camera, -mov_x * pan_y * dt * 10,
                              -mov_y * pan_y * dt * 10,
                              -mov_z * pan_y * dt * 10)

        mov_x, mov_y, mov_z = E.get_entity_z_vector(hand)
        E.move_entity(camera, -mov_x * pan_z * dt * 10,
                              -mov_y * pan_z * dt * 10,
                              -mov_z * pan_z * dt * 10)
    end

    return true
end

function do_click(b, s)
    if b == 1 then
        tumble = s
    end
    if b == 3 then
        scale  = s
    end

    return true
end

function do_point(dx, dy)
    if tumble then
        rot_x = rot_x + dy
        rot_y = rot_y + dx

        if rot_x >  90.0 then rot_x =  90 end
        if rot_x < -90.0 then rot_x = -90 end

        E.set_entity_rotation(scene, rot_x, rot_y, 0)

        return true
    end

    if scale then
        zoom = zoom + dy / 100

        E.set_entity_scale(scene, zoom, zoom, zoom)

        return true
    end

    return false
end

function do_keyboard(k, s)
    local d = 0.25

    if s then
        if k == 13 then
            E.set_entity_position(camera, 0.0, 0.0, 0.0)
            E.set_entity_rotation(camera, 0.0, 0.0, 0.0)
            return true
        end
        if k == 287 then
            E.set_camera_stereo(camera, E.stereo_mode_none, 0, 0, 0)
            E.set_camera_stereo(nearby, E.stereo_mode_none, 0, 0, 0)
            E.set_background(0.0, 0.0, 0.0, 0.1, 0.2, 0.4)
            return true
        end
        if k == 288 then
            E.set_camera_stereo(camera, E.stereo_mode_red_blue, d, -d, d)
            E.set_camera_stereo(nearby, E.stereo_mode_red_blue, d, -d, d)
            E.set_background(0.0, 0.0, 0.0)
            return true
        end
        if k == 289 then
            E.set_camera_stereo(camera, E.stereo_mode_quad, d, -d, d)
            E.set_camera_stereo(nearby, E.stereo_mode_quad, d, -d, d)
            E.set_background(0.0, 0.0, 0.0, 0.1, 0.2, 0.4)
            return true
        end

        if k == 273 then pan_z = pan_z + 1 end
        if k == 274 then pan_z = pan_z - 1 end
        if k == 281 then pan_y = pan_y + 1 end
        if k == 280 then pan_y = pan_y - 1 end
        if k == 275 then pan_x = pan_x + 1 end
        if k == 276 then pan_x = pan_x - 1 end
    else
        if k == 273 then pan_z = pan_z - 1 end
        if k == 274 then pan_z = pan_z + 1 end
        if k == 281 then pan_y = pan_y - 1 end
        if k == 280 then pan_y = pan_y + 1 end
        if k == 275 then pan_x = pan_x - 1 end
        if k == 276 then pan_x = pan_x + 1 end
    end



    return false
end

do_start()

E.print_console("ESC: Exit\n")
E.print_console(" F1: Toggle server console\n")
E.print_console(" F2: Toggle server rendering\n")
E.print_console(" F3: Decrease server window resolution\n")
E.print_console(" F4: Increase server window resolution\n")
E.print_console(" F5: Toggle server fullscreen\n")
E.print_console(" F6: Select mono-scopic rendering\n")
E.print_console(" F7: Select red-blue stereo\n")
E.print_console(" F8: Select quad-buffered stereo\n")

E.color_console(0.0, 0.5, 0.0)
E.print_console("(dismiss the console before switching stereo modes)\n")
E.color_console(0.0, 1.0, 0.0)