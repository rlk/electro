
-------------------------------------------------------------------------------

function add_object(name, parent, r, x, y, z, k)
    local object = E.create_object(name)
    local x0, y0, z0, x1, y1, z1 = E.get_entity_bound(object)

    E.set_entity_position(object, x, y - y0 * k, z)
    E.set_entity_rotation(object, 0.0, r, 0.0)
    E.set_entity_scale(object, k, k, k)
    E.parent_entity(object, parent)
end

function do_start()

    camera = E.create_camera(E.camera_type_perspective)
    light1 = E.create_light(E.light_type_directional)
    light2 = E.create_light(E.light_type_directional)
    pivot  = E.create_pivot()
    head_L = E.create_pivot()
    head_W = E.create_pivot()
    wand_L = E.create_pivot()
    wand_W = E.create_pivot()
    hand   = E.create_object("../data/axes.obj")
    floor  = E.create_object("../data/checker.obj")

    E.parent_entity(light1, camera)
    E.parent_entity(light2, light1)
    E.parent_entity(pivot,  light2)
    E.parent_entity(head_L, light2)
    E.parent_entity(head_W, light2)
    E.parent_entity(wand_L, light2)
    E.parent_entity(wand_W, light2)

    E.parent_entity(hand, wand_W)
    E.parent_entity(floor, pivot)

    local X0, Y0, Z0, X1, Y1, Z1 = E.get_display_bound()
    local scl = (X1 - X0) / 10.0

    add_object("../data/cow.obj",  pivot, -90.0, -3.0, 0.0, -5.0, 0.3)
    add_object("../data/ball.obj", pivot,  0.0, 2.0, 0.0, -1.0, 0.8)
    add_object("../data/box.obj", pivot,  30.0, 0.0, 0.0, -8.0, 1.0)
    add_object("../data/747.obj", pivot,  150.0, 0.0, 3.0, 1.0, 1.0)
    add_object("../data/teapot.obj", pivot, 0.0, 0.0, 0.0, -4.0, 1.5)

    E.set_light_color(light1, 0.8, 0.8, 0.6)
    E.set_light_color(light2, 0.6, 0.8, 0.8)

    E.set_entity_position(light1,  8.0, 7.0, 5.0)
    E.set_entity_position(light2, -8.0, 5.0, 7.0)
    E.set_entity_position(floor, 0.0, 0.0, -5.0)
    E.set_entity_scale   (hand,  0.3, 0.3, 0.3)
    E.set_entity_scale   (pivot, scl, scl, scl)
    E.set_entity_position(pivot, 0.0,  Y0, 0.0)

    E.set_entity_tracking(head_L, 0, E.tracking_mode_local)
    E.set_entity_flags   (head_L, E.entity_flag_track_pos, true)
    E.set_entity_flags   (head_L, E.entity_flag_track_rot, true)

    E.set_entity_tracking(head_W, 0, E.tracking_mode_world)
    E.set_entity_flags   (head_W, E.entity_flag_track_pos, true)
    E.set_entity_flags   (head_W, E.entity_flag_track_rot, true)

    E.set_entity_tracking(wand_L, 1, E.tracking_mode_local)
    E.set_entity_flags   (wand_L, E.entity_flag_track_pos, true)
    E.set_entity_flags   (wand_L, E.entity_flag_track_rot, true)

    E.set_entity_tracking(wand_W, 1, E.tracking_mode_world)
    E.set_entity_flags   (wand_W, E.entity_flag_track_pos, true)
    E.set_entity_flags   (wand_W, E.entity_flag_track_rot, true)

    if stereo_mode then
        E.set_camera_stereo(camera, stereo_mode, -O / 2, 0, 0, O / 2, 0, 0)
    end

    E.enable_timer(true)
end

function do_timer(dt)
    local joy_x, joy_y = E.get_joystick(0)

    local speed    = 8.0
    local deadzone = 0.1

    if joy_x < -deadzone or deadzone < joy_x then
        E.turn_entity(camera, 0, -joy_x * dt * 90, 0)
    end

    if joy_y < -deadzone or deadzone < joy_y then
        local mov_x, mov_y, mov_z = E.get_entity_z_vector(wand_L)
        E.move_entity(camera, mov_x * joy_y * dt * speed,
                              mov_y * joy_y * dt * speed,
                              mov_z * joy_y * dt * speed)
    end
--[[
    print("head_L = ", E.get_entity_position(head_L))
    print("head_W = ", E.get_entity_position(head_W))
    print("wand_L = ", E.get_entity_position(wand_L))
    print("wand_W = ", E.get_entity_position(wand_W))
]]--
    return true
end

function do_keyboard(k, s)

    if varrier_keyboard then
        if varrier_keyboard(k, s, camera) then
            return true
        end
    end

    if s and k == E.key_return then
        E.set_entity_rotation(pivot,  0.0, 0.0, 0.0)
        E.set_entity_position(camera, 0.0, 0.0, 0.0)
        E.set_entity_rotation(camera, 0.0, 0.0, 0.0)
        return true
    end

    return false
end

-------------------------------------------------------------------------------

E.set_background(0.1, 0.2, 0.4, 0.8, 0.8, 1.0)
do_start()
