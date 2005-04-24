dofile("../keyboard.lua")

view    = nil
camera  = nil
sprite  = nil
light   = nil
pivot   = nil
floor   = nil

stereo = false
tumble = false
scale  = false

zoom  = 1
rot_x = 0
rot_y = 0

function add_object(i, s)
    local object = E.create_object(s)

    E.parent_entity(object, pivot)
end

function do_start(arg)
    local x, y, w, h = E.get_viewport()

    nearby = E.create_camera(E.camera_type_perspective)
    camera = E.create_camera(E.camera_type_perspective)
    light  = E.create_light(E.light_type_directional)
    scene  = E.create_pivot()
    pivot  = E.create_pivot()
    hand   = E.create_object("box.obj")

    E.parent_entity(light, camera)
    E.parent_entity(scene, light)
    E.parent_entity(pivot, scene)
    E.parent_entity(hand,  nearby)

    E.set_entity_position(light,  0.0,  8.0,   8.0)
    E.set_entity_position(scene,  0.0, -8.0,  -8.0)
    E.set_entity_position(pivot,  0.0,  0.0, -10.0)

    E.set_entity_scale(hand, 0.25, 0.25, 0.25)

    E.set_entity_flag(hand, E.entity_flag_pos_tracked_1, true)
    E.set_entity_flag(hand, E.entity_flag_rot_tracked_1, true)

    table.foreach(arg, add_object)

    E.enable_timer(true)
end

function do_timer(dt)
    local joy_x, joy_y = E.get_joystick(0)

    local mov_x, mov_y, mov_z = E.get_entity_z_vector(hand)

    if joy_x < -0.1 or 0.1 < joy_x then
        E.turn_entity(camera, 0, -joy_x * dt * 90, 0)
    end

    if joy_y < -0.1 or 0.1 < joy_y then
        E.move_entity(camera, -mov_x * joy_y * dt * 10,
                              -mov_y * joy_y * dt * 10,
                              -mov_z * joy_y * dt * 10)
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
        zoom = zoom + dy / 100
        E.set_entity_scale(pivot, zoom, zoom, zoom)
        return true
    end

    return false
end

function do_keyboard(k, s)
    local d = 0.0125
    
    if s and k == 284 then
        stereo = not stereo

        if stereo then
            E.set_camera_stereo(camera, E.stereo_mode_red_blue, d, -d, d)
            E.set_camera_stereo(nearby, E.stereo_mode_red_blue, d, -d, d)
            E.set_background(0.0, 0.0, 0.0)
        else
            E.set_camera_stereo(camera, E.stereo_mode_none, 0, 0, 0)
            E.set_camera_stereo(nearby, E.stereo_mode_none, 0, 0, 0)
            E.set_background(0.0, 0.0, 0.0, 0.1, 0.2, 0.4)
        end
        return true
    else
        return false
    end
end
