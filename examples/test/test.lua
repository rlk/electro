dofile("../keyboard.lua")

view    = nil
camera  = nil
sprite  = nil
light   = nil
pivot   = nil
thing1  = nil
thing2  = nil
floor   = nil

tumble = false
scale  = false

zoom  = 1
rot_x = 0
rot_y = 0

function do_start(arg)
    local x, y, w, h = E.get_viewport()

    nearby = E.create_camera(E.camera_type_perspective)
    camera = E.create_camera(E.camera_type_perspective)
    light  = E.create_light(E.light_type_directional)
    scene  = E.create_pivot()
    pivot  = E.create_pivot()
    thing  = E.create_object("box.obj")
    floor  = E.create_object("checker.obj")
    hand   = E.create_object("box.obj")

    E.parent_entity(light, camera)
    E.parent_entity(scene, light)
    E.parent_entity(pivot, scene)
    E.parent_entity(thing, pivot)
    E.parent_entity(floor, pivot)
    E.parent_entity(hand,  nearby)

    E.set_entity_position(light,  0.0,  8.0,   8.0)
    E.set_entity_position(scene,  0.0, -8.0,  -8.0)
    E.set_entity_position(pivot,  0.0,  0.0, -10.0)
    E.set_entity_position(thing,  0.0,  0.0,   0.0)

    E.set_entity_scale(floor, 2, 2, 2)
    E.set_entity_scale(hand, 0.25, 0.25, 0.25)

    E.set_entity_flag(floor, E.entity_flag_hidden, true)
    E.set_entity_flag(hand, E.entity_flag_pos_tracked_1, true)
    E.set_entity_flag(hand, E.entity_flag_rot_tracked_1, true)

--  E.set_camera_stereo(camera, E.camera_stereo_red_blue, 0.125, -0.125, 0.125)
--  E.set_camera_stereo(nearby, E.camera_stereo_red_blue, 0.125, -0.125, 0.125)

--  E.set_background(0, 0, 0)

    table.foreach(arg, function (i, s) print(i, s) end)

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
    local shift   = E.get_modifier(1)
    local control = E.get_modifier(64)

    if s then

        if k == keycode.up        then E.move_entity(thing,  0, 0, -1) end
        if k == keycode.down      then E.move_entity(thing,  0, 0,  1) end
        if k == keycode.right     then E.move_entity(thing,  1, 0,  0) end
        if k == keycode.left      then E.move_entity(thing, -1, 0,  0) end

        if shift then
            if k == keycode.page_up   then E.turn_entity(thing, -10, 0, 0) end
        elseif control then
            if k == keycode.page_up   then E.turn_entity(thing, 0, -10, 0) end
        else
            if k == keycode.page_up   then E.turn_entity(thing, 0, 0, -10) end
        end

        if shift then
            if k == keycode.page_down then E.turn_entity(thing,  10, 0, 0) end
        elseif control then
            if k == keycode.page_down then E.turn_entity(thing, 0,  10, 0) end
        else
            if k == keycode.page_down then E.turn_entity(thing, 0, 0,  10) end
        end

        return true
    else
        return false
    end
end
