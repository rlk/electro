dofile("examples/box.lua")

tumble  = false
scale   = false
objects = { }

zoom  = 0.25
rot_x = 45
rot_y = 45

function add_box()
    local object = E.create_object("lil_box.obj")
    E.parent_entity(object, pivot)

    E.set_entity_solid(object, E.solid_type, E.solid_type_box)
    E.set_entity_solid(object, E.solid_box_param, 2, 2, 2)
    E.set_entity_solid(object, E.solid_restitution, 0.0)

    E.set_entity_position(object, 0.0, 20.0, 0.0)
    E.set_entity_rotation(object, math.random(-180, 180),
                                  math.random(-180, 180),
                                  math.random(-180, 180))

    table.insert(objects, object)
end

function add_ball()
    local object = E.create_object("ball.obj")
    E.parent_entity(object, pivot)

    E.set_entity_solid(object, E.solid_type, E.solid_type_sphere)
    E.set_entity_solid(object, E.solid_sphere_param, 1)
    E.set_entity_solid(object, E.solid_restitution, 0.8)

    E.set_entity_position(object, 0.0, 20.0, 0.0)
    E.set_entity_rotation(object, math.random(-180, 180),
                                  math.random(-180, 180),
                                  math.random(-180, 180))

    table.insert(objects, object)
end

function do_start()
    local L = -0.2083 / 2
    local R =  0.2083 / 2

    camera = E.create_camera(E.camera_type_perspective)
    light  = E.create_light(E.light_type_positional)
    pivot  = E.create_pivot()

    plane = E.create_object("checker.obj")

    E.parent_entity(light, camera)
    E.parent_entity(pivot, light)
    E.parent_entity(plane, pivot)

    E.set_entity_solid(plane, E.solid_type, E.solid_type_plane)
    E.set_entity_solid(plane, E.solid_restitution, 0.0)
    E.set_entity_scale(plane, 8, 8, 8)

    E.set_entity_position(light, 0.0,  8.0,   8.0)
    E.set_entity_position(pivot, 0.0, -8.0, -20.0)
    E.set_entity_rotation(pivot, rot_x, rot_y, 0)
    E.set_entity_scale   (pivot, zoom, zoom, zoom)

    E.enable_timer(true)
end

function do_keyboard(k, s)
    local L = -0.20833 / 2
    local R =  0.20833 / 2

    if varrier_keyboard then
        if varrier_keyboard(k, s, camera) then
            return true
        end
    end

    if s then
        if k == string.byte("1") then
            add_box()
        end
        if k == string.byte("2") then
            add_ball()
        end
        if k == 287 then
            E.set_entity_flags(camera, E.entity_flag_wireframe, true)
            return true
        end
        if k == 288 then
            E.set_entity_flags(camera, E.entity_flag_wireframe, false)
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
