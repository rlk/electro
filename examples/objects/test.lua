
camera = nil
pivot  = nil
ball1  = nil
ball2  = nil

tumble = false

function do_start()

    camera = E.create_camera(E.camera_type_perspective)
    pivot  = E.create_pivot()
    ball1  = E.create_object("ball.obj")
    ball2  = E.create_object("ball.obj")

    E.parent_entity(pivot, camera)
    E.parent_entity(ball1, pivot)
    E.parent_entity(ball2, pivot)

    E.set_entity_position(ball1, -1.0, 0.0, 0.0)
    E.set_entity_position(ball2,  1.0, 0.0, 0.0)
end

function do_click(b, s)
    if b == 1 then
        tumble = s
    end
end

function do_point(dx, dy)
    if tumble then
        local x, y, z = E.get_entity_rotation(pivot)
        E.set_entity_rotation(x + dy, y + dx, z)
    end
end