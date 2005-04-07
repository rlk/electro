dofile("../keyboard.lua")

tracking = true

camera  = nil
ortho   = nil
sprite  = nil
light   = nil
pivot   = nil
thing   = nil
floor   = nil

tumble = false
dolly  = false

function do_start()
    local x, y, w, h = E.get_viewport()

    ortho  = E.create_camera(E.camera_type_orthogonal)
    camera = E.create_camera(E.camera_type_perspective)
    light  = E.create_light(E.light_type_directional)
    scene  = E.create_pivot()
    pivot  = E.create_pivot()
    thing  = E.create_object("cow.obj")
    floor  = E.create_object("checker.obj")

    E.parent_entity(light, camera)
    E.parent_entity(scene, light)
    E.parent_entity(pivot, scene)
    E.parent_entity(thing, pivot)
    E.parent_entity(floor, pivot)

    E.set_entity_position(light,  0,  8.0,   8.0)
    E.set_entity_position(scene,  0, -8.0,  -8.0)
    E.set_entity_position(pivot,  0,  3.0, -10.0)
    E.set_entity_position(thing,  0,  1.0,   0.0)
    E.set_entity_position(camera, 0,  0.0,   0.0)

    E.set_background(1, 1, 1, 0, 0.5, 1)

    if tracking then 
        E.enable_timer(true)
    end
end

function do_timer(dt)
    if tracking then
        local x, y, z = E.get_tracking()

--      E.set_entity_position(camera, x, y, z)
        E.set_camera_offset(camera, x, y, z)
        
        return true
    else
        return false
    end
end

function do_click(b, s)
    if b == 1 then
        tumble = s
    end
    if b == 3 then
        dolly  = s
    end

    return true
end

function do_point(dx, dy)
    if tumble then
        local x, y, z = E.get_entity_rotation(pivot)
        E.set_entity_rotation(pivot, x + dy, y + dx, z)
        return true
    end

    if dolly then
        local x, y, z = E.get_entity_position(camera)
        E.set_entity_position(camera, x, y, z + dy / 2)
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
