
-------------------------------------------------------------------------------

rot_x  = 0.0
rot_y  = 0.0

camera = nil
object = nil
light  = nil

-------------------------------------------------------------------------------

function do_start()
    camera = camera_create(2)
    light  =  light_create(2)
    object = object_create("ball.obj")

    entity_parent(light,  camera)
    entity_parent(object, light)

    camera_zoom(camera, 0.001)
    camera_dist(camera, 5.000)

    entity_position(light, 1, 1, 1)

    return true
end

function do_point(dx, dy)
    rot_x = rot_x + dy
    rot_y = rot_y + dx

    entity_rotation(object, rot_x, rot_y, 0)

    return true
end

function do_keyboard(k, s)
    return true
end

-------------------------------------------------------------------------------
