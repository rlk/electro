dofile("../keyboard.lua")

camera  = nil
light   = nil
pivot   = nil
thing   = nil
floor   = nil

tumble = false

function dump(entity, depth)
    local i = 0

    print(string.rep("   ", depth), entity, E.get_entity_parent(entity))

    while E.get_entity_child(entity, i) do
        dump(E.get_entity_child(entity, i), depth + 1)
        i = i + 1
    end
end

function do_start()

    camera  = E.create_camera(E.camera_type_perspective)
    light   = E.create_light(E.light_type_positional)
    pivot   = E.create_pivot()
    thing   = E.create_object("box.obj")
    floor   = E.create_object("checker.obj")

    E.parent_entity(light, camera)
    E.parent_entity(pivot, light)
    E.parent_entity(thing, pivot)
    E.parent_entity(floor, pivot)

    E.set_entity_rotation(light, 30, 0, 0)
    E.set_entity_position(light, 0,  2.0,  2.0)
    E.set_entity_position(pivot, 0, -2.0, -2.0)
    E.set_entity_position(thing, 0,  1.0,  0.0)

    E.set_camera_distance(camera, 10)

    dump(camera, 1)
end

function do_click(b, s)
    if b == 1 then
        tumble = s
    end
end

function do_point(dx, dy)
    if tumble then
        local x, y, z = E.get_entity_rotation(pivot)
        E.set_entity_rotation(pivot, x + dy, y + dx, z)
        return true
    else
        return false
    end
end

function do_keyboard(k, s)
    local shift   = E.get_modifier(1)
    local control = E.get_modifier(64)

    if s then
        if k == keycode.k then
            E.delete_entity(thing)
            thing = E.create_object("box.obj")
            E.parent_entity(thing, pivot)
            E.set_entity_position(thing, 0,  1.0,  0.0)
        end

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
