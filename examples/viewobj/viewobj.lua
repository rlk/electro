tumble = false
scale  = false
test   = false
scene  = nil

zoom  = 1
rot_x = 0
rot_y = 0
pan_x = 0
pan_y = 0
pan_z = 0
rot_dy = 0

function help()
    E.print_console("ESC: Exit\n")
    E.print_console(" F1: Toggle server console\n")
    E.print_console(" F2: Toggle server rendering\n")
    E.print_console(" F3: Decrease server window resolution\n")
    E.print_console(" F4: Increase server window resolution\n")

    if varrier_help then varrier_help() end
end

function add_object(i, s)
    local object = E.create_object(s)
    E.parent_entity(object, scene)
end

function do_start()
    camera = E.create_camera(E.camera_type_perspective)
    light  = E.create_light(E.light_type_directional)
    scene  = E.create_pivot()
    hand   = E.create_pivot()

    E.parent_entity(light, camera)
    E.parent_entity(scene, light)
    E.parent_entity(pivot, scene)

    E.set_entity_position(light,  0.0,  8.0,   8.0)
    E.set_entity_position(scene,  0.0, -8.0, -20.0)

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

    if rot_dy < 0 or 0 < rot_dy then
        rot_y = rot_y + rot_dy * dt * 10
        E.set_entity_rotation(scene, rot_x, rot_y, 0)
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
        zoom = zoom + dy / 500

        E.set_entity_scale(scene, zoom, zoom, zoom)

        return true
    end

    return false
end

function do_keyboard(k, s)

    if varrier_keyboard then
        if varrier_keyboard(k, s, camera) then
            return true
        end
    end

    if s then
        if k == 13 then
            E.set_entity_rotation(scene,  0.0, 0.0, 0.0)
            E.set_entity_position(camera, 0.0, 0.0, 0.0)
            E.set_entity_rotation(camera, 0.0, 0.0, 0.0)
            rot_x = 0
            rot_y = 0
            pan_x = 0
            pan_y = 0
            pan_z = 0
            return true
        end

	if k == 277 then
            rot_dy = rot_dy + 1
	end
	if k == 127 then
            rot_dy = rot_dy - 1
	end

        if not E.get_modifier(E.key_modifier_control) then
            if k == 273 then pan_z = pan_z + 1 end
            if k == 274 then pan_z = pan_z - 1 end
            if k == 281 then pan_y = pan_y + 1 end
            if k == 280 then pan_y = pan_y - 1 end
            if k == 275 then pan_x = pan_x + 1 end
            if k == 276 then pan_x = pan_x - 1 end
        end
    else
        if not E.get_modifier(E.key_modifier_control) then
            if k == 273 then pan_z = pan_z - 1 end
            if k == 274 then pan_z = pan_z + 1 end
            if k == 281 then pan_y = pan_y - 1 end
            if k == 280 then pan_y = pan_y + 1 end
            if k == 275 then pan_x = pan_x - 1 end
            if k == 276 then pan_x = pan_x + 1 end
        end
    end

    return false
end

help()
do_start()

E.set_background(0, 0, 0)
