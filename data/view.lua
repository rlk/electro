
-------------------------------------------------------------------------------

rot_x  = 0.0
rot_y  = 0.0

camera = nil
obj1   = nil
obj2   = nil
light  = nil

-------------------------------------------------------------------------------

function do_start()
    camera = camera_create(2)
    light  =  light_create(2)

    pivot  =  pivot_create()

    obj1 = object_create("ball.obj")
    obj2 = object_create("ball.obj")

    entity_parent(light, camera)
    entity_parent(pivot, light)
    entity_parent(obj1,  pivot)
    entity_parent(obj2,  pivot)

    camera_zoom(camera, 0.001)
    camera_dist(camera, 10.000)

    entity_position(light, 1, 1, 1)
    entity_position(obj1, -1, 0, 0)
    entity_position(obj2,  1, 0, 0)

    return true
end

function do_point(dx, dy)
    rot_x = rot_x + dy
    rot_y = rot_y + dx

    entity_rotation(pivot, rot_x, rot_y, 0)

    return true
end

function do_keyboard(k, s)
    if s then
        print(k, "down")
    else
        print(k, "up")
    end
    return false
end

-------------------------------------------------------------------------------
