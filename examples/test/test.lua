
camera = nil
light  = nil
pivot  = nil
ball1  = nil
ball2  = nil

tumble = false

function do_start()

    camera = E.create_camera(E.camera_type_perspective)
    light  = E.create_light(E.light_type_positional)
    pivot  = E.create_pivot()
    ball1  = E.create_object("box.obj")
    ball2  = E.create_object("box.obj")

    E.parent_entity(light, camera)
    E.parent_entity(pivot, light)
    E.parent_entity(ball1, pivot)
    E.parent_entity(ball2, pivot)

    E.set_entity_position(light, 0, 2, 2)
    E.set_entity_position(ball1, -1.5, 0.0, 0.0)
    E.set_entity_position(ball2,  1.5, 0.0, 0.0)

    E.set_camera_distance(camera, 10)
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