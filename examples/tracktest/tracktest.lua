
-------------------------------------------------------------------------------

function do_start()

    camera = E.create_camera(E.camera_type_perspective)
    light  = E.create_light(E.light_type_directional)
    pivot  = E.create_pivot()
    wand   = E.create_pivot()
    head   = E.create_pivot()
    hand   = E.create_object("../data/axes.obj")
    floor  = E.create_object("../data/checker.obj")

    E.parent_entity(light, camera)
    E.parent_entity(pivot, light)
    E.parent_entity(wand,  light)
    E.parent_entity(hand,  light)
    
    E.set_entity_position(light, 0.0, 8.0, 8.0)
    E.set_entity_scale   (floor, 4.0, 4.0, 4.0)

    E.set_entity_tracking(head, 0, E.tracking_mode_local)
    E.set_entity_flags   (head, E.entity_flag_track_pos, true)
    E.set_entity_flags   (head, E.entity_flag_track_rot, true)

    E.set_entity_tracking(hand, 1, E.tracking_mode_world)
    E.set_entity_flags   (hand, E.entity_flag_track_pos, true)
    E.set_entity_flags   (hand, E.entity_flag_track_rot, true)

    E.set_entity_tracking(wand, 1, E.tracking_mode_local)
    E.set_entity_flags   (wand, E.entity_flag_track_pos, true)
    E.set_entity_flags   (wand, E.entity_flag_track_rot, true)

    E.parent_entity(floor, pivot)

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
        local mov_x, mov_y, mov_z = E.get_entity_z_vector(wand)
        E.move_entity(camera, mov_x * joy_y * dt * speed,
                              mov_y * joy_y * dt * speed,
                              mov_z * joy_y * dt * speed)
    end

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
