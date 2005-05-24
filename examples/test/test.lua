camera  = nil
light   = nil
pivot   = nil
thing   = nil

tumble = false
scale  = false

zoom  = 1
rot_x = 0
rot_y = 0

function add_object(i, s)
    local object = E.create_object(s)

    E.parent_entity(object, pivot)
end

function do_start()
    camera = E.create_camera(E.camera_type_perspective)
    light  = E.create_light(E.light_type_positional)
    pivot  = E.create_pivot()
    thing  = E.create_object("box.obj")

    E.parent_entity(light, camera)
    E.parent_entity(pivot, light)
    E.parent_entity(thing, pivot)

    E.set_entity_position(light,  0.0,  10.0,   0.0)
    E.set_entity_position(pivot,  0.0, -10.0,   0.0)
    E.set_entity_position(thing,  0.0,   0.0, -10.0)

    E.enable_timer(true)
end

function do_timer(dt)
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

        E.set_entity_rotation(thing, rot_x, rot_y, 0)
        return true
    end

    if scale then
        zoom = zoom + dy / 100
        E.set_entity_scale(thing, zoom, zoom, zoom)
        return true
    end

    return false
end

do_start()
