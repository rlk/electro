dofile("examples/box.lua")

tumble  = false
scale   = false
objects = { }
speed   = 0
steer   = 0
rot_y   = 0
pos_x   = 0
pos_y   = 5
pos_z   = 0

body = nil
tire = { }

function add_box()
    local object = E.create_object("lil_box.obj")
    E.parent_entity(object, pivot)

    E.set_entity_body_type(object, true)
    E.set_entity_geom_type(object, E.geom_type_box, 2, 2, 2)

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

    E.set_entity_body_type(object, true)
    E.set_entity_geom_type(object, E.geom_type_sphere, 1)

    E.set_entity_position(object, 0.0, 20.0, 0.0)
    E.set_entity_rotation(object, math.random(-180, 180),
                                  math.random(-180, 180),
                                  math.random(-180, 180))

    table.insert(objects, object)
    return object
end

function add_thing()
    local object = E.create_object("thing.obj")
    local part1  = E.create_pivot()
    local part2  = E.create_pivot()
    local part3  = E.create_pivot()
    
    E.parent_entity(object, pivot)
    E.parent_entity(part1, object)
    E.parent_entity(part2, object)
    E.parent_entity(part3, object)

    E.set_entity_body_type(object, true)
    E.set_entity_geom_type(part1, E.geom_type_box, 3, 1, 1)
    E.set_entity_geom_type(part2, E.geom_type_box, 1, 3, 1)
    E.set_entity_geom_type(part3, E.geom_type_box, 1, 1, 3)

    E.set_entity_position(object, 0.0, 20.0, 0.0)
    E.set_entity_rotation(object, math.random(-180, 180),
                                  math.random(-180, 180),
                                  math.random(-180, 180))

    E.set_entity_position(part1, 1.5, 0.5, 0.5)
    E.set_entity_position(part2, 0.5, 1.5, 0.5)
    E.set_entity_position(part3, 0.5, 0.5, 1.5)
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

    E.parent_entity(body, pivot)

    E.set_entity_body_type(body, true)
    E.set_entity_geom_type(body, E.geom_type_box, 2, 1.25, 4)
    E.set_entity_geom_attr(body, E.geom_attr_category, 2)
    E.set_entity_geom_attr(body, E.geom_attr_collides, 2)
    E.set_entity_position (body, 0.00, 1.10 + height, 0.00)

    for i = 1, 4 do
        local x = pos[i][1]
        local y = pos[i][2] + height
        local z = pos[i][3]

        tire[i] = E.create_object("tire.obj")
        E.parent_entity(tire[i], pivot)

        E.set_entity_body_type(tire[i], true)
        E.set_entity_geom_type(tire[i], E.geom_type_sphere, 0.35)
        E.set_entity_geom_attr(tire[i], E.geom_attr_category, 4)
        E.set_entity_geom_attr(tire[i], E.geom_attr_collides, 4)
        E.set_entity_geom_attr(tire[i], E.geom_attr_friction, 25)

        E.set_entity_position(tire[i], x, y, z)

        E.set_entity_joint_type(body, tire[i], E.joint_type_hinge_2)
        E.set_entity_joint_attr(body, tire[i], E.joint_attr_anchor, x, y, z)
        E.set_entity_joint_attr(body, tire[i], E.joint_attr_axis_1, 0, 1, 0)
        E.set_entity_joint_attr(body, tire[i], E.joint_attr_axis_2, 1, 0, 0)
        E.set_entity_joint_attr(body, tire[i], E.joint_attr_lo_stop, 0)
        E.set_entity_joint_attr(body, tire[i], E.joint_attr_hi_stop, 0)
        E.set_entity_joint_attr(body, tire[i], E.joint_attr_susp_erp, 0.05)
        E.set_entity_joint_attr(body, tire[i], E.joint_attr_susp_cfm, 0.1)
    end

    E.set_entity_joint_attr(body, tire[2], E.joint_attr_lo_stop, -0.3)
    E.set_entity_joint_attr(body, tire[2], E.joint_attr_hi_stop,  0.3)
    E.set_entity_joint_attr(body, tire[3], E.joint_attr_lo_stop, -0.3)
    E.set_entity_joint_attr(body, tire[3], E.joint_attr_hi_stop,  0.3)
end

function change_speed(d)
    if math.abs(speed + d) > math.abs(speed) then
        force = 100
    else
        force = 1
    end

    speed = speed + d

    E.set_entity_joint_attr(body, tire[1], E.joint_attr_velocity_2, 30 * speed)
    E.set_entity_joint_attr(body, tire[4], E.joint_attr_velocity_2, 30 * speed)

    E.set_entity_joint_attr(body, tire[1], E.joint_attr_force_max_2, force)
    E.set_entity_joint_attr(body, tire[4], E.joint_attr_force_max_2, force)
end

function change_steer(d)
    steer = steer + d

    E.set_entity_joint_attr(body, tire[2], E.joint_attr_velocity, steer)
    E.set_entity_joint_attr(body, tire[2], E.joint_attr_force_max, 100.0)
    E.set_entity_joint_attr(body, tire[3], E.joint_attr_velocity, steer)
    E.set_entity_joint_attr(body, tire[3], E.joint_attr_force_max, 100.0)
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

    E.set_entity_geom_type(plane, E.geom_type_plane, 0, 1, 0, 0)

    E.set_entity_position(light, 0.0,  16.0,  0.0)
    E.set_entity_position(pivot, 0.0, -16.0,  0.0)
--  E.set_entity_rotation(pivot, rot_x, rot_y, 0)
--  E.set_entity_scale   (pivot, zoom, zoom, zoom)
    E.set_entity_scale   (plane, 8, 8, 8)

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
            add_thing()
        end
        if k == string.byte("4") then
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
    if body then
        local px, py, pz = E.get_entity_position(body)
        local yx, yy, yz = E.get_entity_y_vector(body)
        local zx, zy, zz = E.get_entity_z_vector(body)

        local x = px + yx * 5 + zx * 10
        local y = 5
        local z = pz + yz * 5 + zz * 10

        local a = math.atan2(zx, zz) * 180 / math.pi
        local k = 50

        pos_x = ((k - 1) * pos_x + x) / k
        pos_y = ((k - 1) * pos_y + y) / k
        pos_z = ((k - 1) * pos_z + z) / k

        E.set_entity_position(camera, pos_x, pos_y, pos_z)
        E.set_entity_rotation(camera, -20, a, 0)

        return true
    end
    return false
end

function do_click(b, s)
    return false
end

function do_point(dx, dy)
    return false
end

E.set_background(0.8, 0.8, 1.0, 0.2, 0.4, 1.0)
do_start()
