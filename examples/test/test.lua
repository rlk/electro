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
    local L = -0.2083 / 2
    local R =  0.2083 / 2

    camera = E.create_camera(E.camera_type_perspective)
    light  = E.create_light(E.light_type_positional)
    pivot  = E.create_pivot()

    E.set_typeface("VeraSeBd.ttf", 0.0001)
    thing1 = E.create_string("Electro")

    E.set_typeface("VeraSeBd.ttf", 0.0001, true)
    thing2 = E.create_string("Electro")

    E.set_typeface("VeraSeBd.ttf", 0.00001)
    thing3 = E.create_string("Electro")

    E.parent_entity(light, camera)
    E.parent_entity(pivot, light)
    E.parent_entity(thing1, pivot)
    E.parent_entity(thing2, thing1)
    E.parent_entity(thing3, pivot)

    E.set_entity_alpha(thing1, 0.50)
    E.set_entity_alpha(thing2, 4.00)

    E.set_string_color(thing1, 1.0, 1.0, 0.0)
    E.set_string_color(thing2, 1.0, 1.0, 0.0)

    E.set_entity_flag(thing1, E.entity_flag_unlit,       true)
    E.set_entity_flag(thing2, E.entity_flag_unlit,       true)
    E.set_entity_flag(thing3, E.entity_flag_unlit,       true)
    E.set_entity_flag(thing1, E.entity_flag_line_smooth, true)
    E.set_entity_flag(thing2, E.entity_flag_line_smooth, true)
    E.set_entity_flag(thing3, E.entity_flag_line_smooth, true)

    x0, y0, z0, x1, y1, z1 = E.get_entity_bound(thing1)


    E.set_entity_position(light,  0.0,  10.0,   0.0)
    E.set_entity_position(pivot,  0.0, -10.0, -10.0)

    E.set_entity_position(thing1, -(x1 - x0) / 2, -(y1 - y0) / 2 - 0.5, 0.0)
    E.set_entity_position(thing3, -(x1 - x0) / 2, -(y1 - y0) / 2 + 0.5, 0.0)

--  E.set_camera_stereo(camera, E.stereo_mode_quad, L, 0, 0, R, 0, 0)

--  E.enable_timer(true)
end

function do_keyboard(k, s)
    local L = -0.20833 / 2
    local R =  0.20833 / 2

    if s and k == 287 then
        E.set_entity_flag(thing1, E.entity_flag_wireframe, true)
        E.set_entity_flag(thing3, E.entity_flag_wireframe, true)
        return true
    end
    if s and k == 288 then
        E.set_entity_flag(thing1, E.entity_flag_wireframe, false)
        E.set_entity_flag(thing3, E.entity_flag_wireframe, false)
        return true
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

do_start()
