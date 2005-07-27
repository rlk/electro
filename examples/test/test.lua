dofile("examples/box.lua")

tumble = false
scale  = false

zoom  = 0.25
rot_x = 45
rot_y = 45

typeface = "../VeraBd.ttf"

function do_start()
    local L = -0.2083 / 2
    local R =  0.2083 / 2

    camera = E.create_camera(E.camera_type_perspective)
    light  = E.create_light(E.light_type_positional)
    pivot  = E.create_pivot()

    E.set_typeface(typeface, 0.001, 0.04)
    thing1 = make_box("../green.jpg", 16, 1, 16)
    thing2 = make_box("../grey.jpg", 1, 1, 1)

    E.parent_entity(light, camera)
    E.parent_entity(pivot, light)
    E.parent_entity(thing1, pivot)
    E.parent_entity(thing2, pivot)

    E.set_entity_flag(thing, E.entity_flag_line_smooth, true)

    E.set_entity_position(light, 0.0,  8.0,   8.0)
    E.set_entity_position(pivot, 0.0, -8.0, -20.0)
    E.set_entity_rotation(pivot, rot_x, rot_y, 0)
    E.set_entity_scale   (pivot, zoom, zoom, zoom)

    E.set_entity_position(thing2, 0.0, 4.0, 0.0)

    E.enable_timer(true)
end

function do_keyboard(k, s)
    local L = -0.20833 / 2
    local R =  0.20833 / 2

    if s then
        if k == 287 then
            E.set_entity_flag(camera, E.entity_flag_wireframe, true)
            return true
        end
        if k == 288 then
            E.set_entity_flag(camera, E.entity_flag_wireframe, false)
            return true
        end
    end
    return false
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

E.set_background(0.8, 0.8, 1.0, 0.2, 0.4, 1.0)
do_start()
