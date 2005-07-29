dofile("examples/box.lua")

tumble  = false
scale   = false
objects = { }
speed   = 0
steer   = 0

body = nil
tire = { }

zoom  = 1
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
    return object
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
    return object
end

function add_car()
    body = E.create_object("body.obj")

    local height = 3

    local pos = {
        {  0.85, 0.35,  1.30 },
        {  0.85, 0.35, -1.25 },
        { -0.85, 0.35, -1.25 },
        { -0.85, 0.35,  1.30 }
    }

    E.parent_entity      (body, pivot)
    E.set_entity_solid   (body, E.solid_type, E.solid_type_box)
    E.set_entity_solid   (body, E.solid_box_param, 2, 1.25, 4)
    E.set_entity_position(body, 0.00, 1.10 + height, 0.00)
    E.set_entity_solid(body, E.solid_category_bits, 2)
    E.set_entity_solid(body, E.solid_collider_bits, 2)

    for i = 1, 4 do
        local x = pos[i][1]
        local y = pos[i][2] + height
        local z = pos[i][3]

        tire[i] = E.create_object("tire.obj")
        E.parent_entity(tire[i], pivot)

        E.set_entity_solid(tire[i], E.solid_type, E.solid_type_sphere)
        E.set_entity_solid(tire[i], E.solid_sphere_param, 0.35)
        E.set_entity_solid(tire[i], E.solid_category_bits, 4)
        E.set_entity_solid(tire[i], E.solid_collider_bits, 4)

        E.set_entity_position(tire[i], x, y, z)

        E.set_entity_joint(body, tire[i], E.joint_type, E.joint_type_hinge_2)
        E.set_entity_joint(body, tire[i], E.joint_anchor, x, y, z)
        E.set_entity_joint(body, tire[i], E.joint_axis_1, 0, 1, 0)
        E.set_entity_joint(body, tire[i], E.joint_axis_2, 1, 0, 0)
        E.set_entity_joint(body, tire[i], E.joint_min_value, 0)
        E.set_entity_joint(body, tire[i], E.joint_max_value, 0)
        E.set_entity_joint(body, tire[i], E.joint_suspension_erp, 0.4)
        E.set_entity_joint(body, tire[i], E.joint_suspension_cfm, 0.8)
    end

    E.set_entity_joint(body, tire[2], E.joint_min_value, -0.3)
    E.set_entity_joint(body, tire[2], E.joint_max_value,  0.3)
    E.set_entity_joint(body, tire[3], E.joint_min_value, -0.3)
    E.set_entity_joint(body, tire[3], E.joint_max_value,  0.3)
end

function change_speed(d)
    speed = speed + d

    E.set_entity_joint(body, tire[1], E.joint_velocity_2, 20 * speed)
    E.set_entity_joint(body, tire[1], E.joint_force_2, 100.0)

    E.set_entity_joint(body, tire[4], E.joint_velocity_2, 20 * speed)
    E.set_entity_joint(body, tire[4], E.joint_force_2, 100.0)
end

function change_steer(d)
    steer = steer + d

    E.set_entity_joint(body, tire[2], E.joint_velocity, steer)
    E.set_entity_joint(body, tire[2], E.joint_force, 100.0)

    E.set_entity_joint(body, tire[3], E.joint_velocity, steer)
    E.set_entity_joint(body, tire[3], E.joint_force, 100.0)
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
        if k == string.byte("3") then
            add_car()
        end

        if k == 287 then
            E.set_entity_flags(camera, E.entity_flag_wireframe, true)
            return true
        end
        if k == 288 then
            E.set_entity_flags(camera, E.entity_flag_wireframe, false)
            return true
        end

        if k == 273 then change_speed(1) end
        if k == 274 then change_speed(-1) end
        if k == 275 then change_steer(1) end
        if k == 276 then change_steer(-1) end
    else
        if k == 273 then change_speed(-1) end
        if k == 274 then change_speed(1) end
        if k == 275 then change_steer(-1) end
        if k == 276 then change_steer(1) end
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
