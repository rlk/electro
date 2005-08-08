tumble = false
scale  = false
test   = false
scene  = nil

objects = { }

zoom  = 1
rot_x = 0
rot_y = 0
pan_x = 0
pan_y = 0
pan_z = 0
rot_dy = 0

X0 = 0
Y0 = 0
Z0 = 0
X1 = 0
Y1 = 0
Z1 = 0
XC = 0
YC = 0
ZC = 0

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
    local x0, y0, z0, x1, y1, z1 = E.get_entity_bound(object)
    local s = 0.5 * (X0 - X1) / (x0 - x1)

    table.insert(objects, object)

    E.parent_entity(object, pivot)
    E.set_entity_scale(object, s, s, s)
end

function do_start()
    X0, Y0, Z0, X1, Y1, Z1 = E.get_display_bound()

    XC = (X0 + X1) / 2
    YC = (Y0 + Y1) / 2
    ZC = (Z0 + Z1) / 2

    camera = E.create_camera(E.camera_type_perspective)
    light  = E.create_light(E.light_type_directional)
    scene  = E.create_pivot()
    pivot  = E.create_pivot()
    hand   = E.create_pivot()

    E.parent_entity(light, camera)
    E.parent_entity(scene, light)
    E.parent_entity(pivot, scene)

    E.set_entity_position(light,  0.0,  8.0,  8.0)
    E.set_entity_position(scene,  0.0, -8.0, -8.0)
    E.set_entity_position(pivot,  XC,   YC,   ZC)

    E.set_entity_flags(hand, E.entity_flag_pos_tracked_1, true)
    E.set_entity_flags(hand, E.entity_flag_rot_tracked_1, true)

    table.foreach(E.argument, add_object)

    E.enable_timer(true)
end

function do_timer(dt)
    local joy_x, joy_y = E.get_joystick(0)
    local s = 1

    if joy_x < -0.1 or 0.1 < joy_x then
        E.turn_entity(camera, 0, -joy_x * dt * 90, 0)
    end

    if joy_y < -0.1 or 0.1 < joy_y then
        mov_x, mov_y, mov_z = E.get_entity_z_vector(hand)
        E.move_entity(camera, -mov_x * joy_y * dt * s,
                              -mov_y * joy_y * dt * s,
                              -mov_z * joy_y * dt * s)
    else
        E.turn_entity(camera, 0, -pan_x * dt * 90, 0)

        mov_x, mov_y, mov_z = E.get_entity_y_vector(hand)
        E.move_entity(camera, -mov_x * pan_y * dt * s,
                              -mov_y * pan_y * dt * s,
                              -mov_z * pan_y * dt * s)

        mov_x, mov_y, mov_z = E.get_entity_z_vector(hand)
        E.move_entity(camera, -mov_x * pan_z * dt * s,
                              -mov_y * pan_z * dt * s,
                              -mov_z * pan_z * dt * s)
    end

    if rot_dy < 0 or 0 < rot_dy then
        rot_y = rot_y + rot_dy * dt * 10
        E.set_entity_rotation(pivot, rot_x, rot_y, 0)
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

        E.set_entity_rotation(pivot, rot_x, rot_y, 0)

        return true
    end

    if scale then
        zoom = zoom + dy / 500

        E.set_entity_scale(pivot, zoom, zoom, zoom)

        return true
    end

    return false
end

function do_keyboard(k, s)
    local d = 0.125

    if varrier_keyboard then
        if varrier_keyboard(k, s, camera) then
            return true
        end
    end

    if s then
        if k == 13 then
            E.set_entity_rotation(pivot,  0.0, 0.0, 0.0)
            E.set_entity_position(camera, 0.0, 0.0, 0.0)
            E.set_entity_rotation(camera, 0.0, 0.0, 0.0)
            rot_x = 0
            rot_y = 0
            pan_x = 0
            pan_y = 0
            pan_z = 0
            return true
        end

        if k == 287 then
            table.foreach(objects, function (id, object)
                E.set_entity_flags(object, E.entity_flag_wireframe, true)
            end)
            return true
        end
        if k == 288 then
            table.foreach(objects, function (id, object)
                E.set_entity_flags(object, E.entity_flag_wireframe, false)
            end)
            return true
        end
        if k == 289 then
            E.set_camera_stereo(camera, E.stereo_mode_none,
                                -d, 0, 0, d, 0, 0)
            return true
        end
        if k == 290 then
            E.set_camera_stereo(camera, E.stereo_mode_red_blue,
                                -d, 0, 0, d, 0, 0)
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

--help()
do_start()

--E.set_background(0, 0, 0)
