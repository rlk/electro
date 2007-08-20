
-------------------------------------------------------------------------------

function do_start()

    camera = E.create_camera(E.camera_type_perspective)
    light  = E.create_light(E.light_type_directional)
    pivot  = E.create_pivot()
    head_L = E.create_pivot()
    head_W = E.create_pivot()
    wand_L = E.create_pivot()
    wand_W = E.create_pivot()
    hand   = E.create_object("../data/axes.obj")
    floor  = E.create_object("../data/checker.obj")
    model  = E.create_object("../data/cow.obj")

    E.parent_entity(light,  camera)
    E.parent_entity(pivot,  light)
    E.parent_entity(head_L, light)
    E.parent_entity(head_W, light)
    E.parent_entity(wand_L, light)
    E.parent_entity(wand_W, light)

    E.parent_entity(hand, wand_W)
    E.parent_entity(floor, pivot)
    E.parent_entity(model, pivot)

    local x0, y0, z0, x1, y1, z1 = E.get_entity_bound(model)

    E.set_entity_position(light, 0.0, 8.0, 8.0)
    E.set_entity_scale   (floor, 4.0, 4.0, 4.0)
    E.set_entity_position(model, 0.0, -y0,  0.0)

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

    print("head_L = ", E.get_entity_position(head_L))
    print("head_W = ", E.get_entity_position(head_W))
    print("wand_L = ", E.get_entity_position(wand_L))
    print("wand_W = ", E.get_entity_position(wand_W))

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
