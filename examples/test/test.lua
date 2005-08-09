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
key_x   = 0
key_y   = 0

body = nil
tire = { }

category_body  = 1
category_tire  = 2
category_world = 4
category_all = category_body + category_tire + category_world

function add_ramp(x, y, z, a, r)
    local ramp = E.create_object("ramp.obj")

    E.parent_entity(ramp, pivot)

    E.set_entity_geom_type(ramp, E.geom_type_box, 8, 2, 16)
    E.set_entity_geom_attr(ramp, E.geom_attr_category, category_world)
    E.set_entity_geom_attr(ramp, E.geom_attr_collider, category_all)
    E.set_entity_position (ramp, x, y, z)
    E.set_entity_rotation (ramp, a, r, 0)
end

function add_box()
    local object = E.create_object("lil_box.obj")
    E.parent_entity(object, pivot)

    E.set_entity_body_type(object, true)
    E.set_entity_geom_type(object, E.geom_type_box, 2, 2, 2)
    E.set_entity_geom_attr(object, E.geom_attr_category, category_world)
    E.set_entity_geom_attr(object, E.geom_attr_collider, category_all)

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
    E.set_entity_geom_attr(object, E.geom_attr_category, category_world)
    E.set_entity_geom_attr(object, E.geom_attr_collider, category_all)

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

    E.set_entity_geom_attr(part1, E.geom_attr_category, category_world)
    E.set_entity_geom_attr(part1, E.geom_attr_collider, category_all)
    E.set_entity_geom_attr(part2, E.geom_attr_category, category_world)
    E.set_entity_geom_attr(part2, E.geom_attr_collider, category_all)
    E.set_entity_geom_attr(part3, E.geom_attr_category, category_world)
    E.set_entity_geom_attr(part3, E.geom_attr_collider, category_all)

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
        {  0.85, 0.55,  1.30 },
        {  0.85, 0.55, -1.25 },
        { -0.85, 0.55, -1.25 },
        { -0.85, 0.55,  1.30 }
    }

    part1 = E.create_pivot()
    part2 = E.create_pivot()

    E.parent_entity(body, pivot)
    E.parent_entity(part1, body)
    E.parent_entity(part2, body)
    E.parent_entity(part3, body)

    E.set_entity_body_type(body, true)
    E.set_entity_geom_type(part1, E.geom_type_box, 2,    0.625, 4)
    E.set_entity_geom_type(part2, E.geom_type_box, 1.35, 0.625, 1)
    E.set_entity_geom_attr(part1, E.geom_attr_mass, 4.0)
    E.set_entity_geom_attr(part2, E.geom_attr_mass, 0.1)
    E.set_entity_geom_attr(part1, E.geom_attr_category, category_body)
    E.set_entity_geom_attr(part2, E.geom_attr_category, category_body)
    E.set_entity_geom_attr(part1, E.geom_attr_collider, category_world)
    E.set_entity_geom_attr(part2, E.geom_attr_collider, category_world)
    E.set_entity_geom_attr(part1, E.geom_attr_callback, category_world)
    E.set_entity_geom_attr(part2, E.geom_attr_callback, category_world)
    E.set_entity_position (body, 0.00, 1.10 + height, 0.00)
    E.set_entity_position (part1, 0, -0.3125, 0)
    E.set_entity_position (part2, 0,  0.3125, 0)
    E.set_entity_geom_attr(part1, E.geom_attr_friction, 100)
    E.set_entity_geom_attr(part2, E.geom_attr_friction, 100)

    for i = 1, 4 do
        local x = pos[i][1]
        local y = pos[i][2] + height
        local z = pos[i][3]

        tire[i] = E.create_object("tire.obj")
        E.parent_entity(tire[i], pivot)

        E.set_entity_body_type(tire[i], true)
        E.set_entity_geom_type(tire[i], E.geom_type_sphere, 0.35)
        E.set_entity_geom_attr(tire[i], E.geom_attr_category, category_tire)
        E.set_entity_geom_attr(tire[i], E.geom_attr_collider, category_world)
        E.set_entity_geom_attr(tire[i], E.geom_attr_friction, 50)

        E.set_entity_position(tire[i], x, y, z)

        E.set_entity_joint_type(body, tire[i], E.joint_type_hinge_2)
        E.set_entity_joint_attr(body, tire[i], E.joint_attr_anchor, x, y, z)
        E.set_entity_joint_attr(body, tire[i], E.joint_attr_axis_1, 0, 1, 0)
        E.set_entity_joint_attr(body, tire[i], E.joint_attr_axis_2, 1, 0, 0)
        E.set_entity_joint_attr(body, tire[i], E.joint_attr_lo_stop, 0)
        E.set_entity_joint_attr(body, tire[i], E.joint_attr_hi_stop, 0)
        E.set_entity_joint_attr(body, tire[i], E.joint_attr_susp_erp, 0.10)
        E.set_entity_joint_attr(body, tire[i], E.joint_attr_susp_cfm, 0.04)
    end

    E.set_entity_joint_attr(body, tire[1], E.joint_attr_stop_cfm, 0.00)
    E.set_entity_joint_attr(body, tire[1], E.joint_attr_stop_erp, 0.80)
    E.set_entity_joint_attr(body, tire[2], E.joint_attr_stop_cfm, 0.02)
    E.set_entity_joint_attr(body, tire[2], E.joint_attr_stop_erp, 0.10)
    E.set_entity_joint_attr(body, tire[3], E.joint_attr_stop_cfm, 0.02)
    E.set_entity_joint_attr(body, tire[3], E.joint_attr_stop_erp, 0.10)
    E.set_entity_joint_attr(body, tire[4], E.joint_attr_stop_cfm, 0.00)
    E.set_entity_joint_attr(body, tire[4], E.joint_attr_stop_erp, 0.80)
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
    E.set_entity_geom_attr(plane, E.geom_attr_category, category_world)
    E.set_entity_geom_attr(plane, E.geom_attr_collider, category_all)

    E.set_entity_position(light, 0.0,  16.0,  0.0)
    E.set_entity_position(pivot, 0.0, -16.0,  0.0)
    E.set_entity_scale   (plane, 16, 16, 16)

    add_ramp(0, 0, 0, 10, 0)
    add_ramp(32, 0, 10, 30, 0)
    add_ramp(-32, 0, -10, -15, 180)

    E.enable_timer(true)
end

function do_joystick(d, b, s)
    if d == 0 and s then
        if b == 0 then
            add_box()
        end
        if b == 1 then
            add_ball()
        end
        if b == 2 then
            add_thing()
        end
        if b == 3 then
            add_car()
        end
    end
    return true
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

        if k == 273 then key_y = key_y - 1 end
        if k == 274 then key_y = key_y + 1 end
        if k == 275 then key_x = key_x + 1 end
        if k == 276 then key_x = key_x - 1 end
    else
        if k == 273 then key_y = key_y + 1 end
        if k == 274 then key_y = key_y - 1 end
        if k == 275 then key_x = key_x - 1 end
        if k == 276 then key_x = key_x + 1 end
    end
    return false
end

total_dt = 0

function do_timer(dt)
    if body then
        local jx, jy = E.get_joystick(0)

        local px, py, pz = E.get_entity_position(body)
        local xx, xy, xz = E.get_entity_x_vector(body)
        local yx, yy, yz = E.get_entity_y_vector(body)
        local zx, zy, zz = E.get_entity_z_vector(body)

        local kx = 10
        local ky = 5
        local kz = 10

        local x = px + xx * kx + yx * ky + zx * kz
        local y = py + xy * ky + yy * ky + zy * ky
        local z = pz + xz * kx + yz * ky + zz * kz

        local a = math.atan2(pos_x - px, pos_z - pz) * 180 / math.pi
        local k = 50

        if y < 1 then y = 1 end

        jx = jx + key_x
        jy = jy + key_y

        pos_x = ((k - 1) * pos_x + x) / k
        pos_y = ((k - 1) * pos_y + y) / k
        pos_z = ((k - 1) * pos_z + z) / k

        E.set_entity_position(camera, pos_x, pos_y, pos_z)
        E.set_entity_rotation(camera, -20, a, 0)


        E.add_entity_torque(tire[2], yx, -yy * 100 * jx, yz)
        E.add_entity_torque(tire[3], yx, -yy * 100 * jx, yz)

        E.set_entity_joint_attr(body, tire[1], E.joint_attr_velocity_2, -100 * jy)
        E.set_entity_joint_attr(body, tire[4], E.joint_attr_velocity_2, -100 * jy)

        E.set_entity_joint_attr(body, tire[1], E.joint_attr_force_max_2, 15)
        E.set_entity_joint_attr(body, tire[4], E.joint_attr_force_max_2, 15)

        return true
    end
    return false
end

E.set_background(0.8, 0.8, 1.0, 0.2, 0.4, 1.0)
do_start()
