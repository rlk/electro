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

max_angle = 30

body = nil
tire = { }

category_body  = 1
category_tire  = 2
category_world = 4
category_all = category_body + category_tire + category_world

function add_ramp(x, y, z, a, r)
    local ramp = E.create_object("../data/ramp.obj")

    E.parent_entity(ramp, pivot)

    E.set_entity_geom_type(ramp, E.geom_type_box, 8, 2, 16)
    E.set_entity_geom_attr(ramp, E.geom_attr_category, category_world)
    E.set_entity_geom_attr(ramp, E.geom_attr_collider, category_all)
    E.set_entity_position (ramp, x, y, z)
    E.set_entity_rotation (ramp, a, r, 0)
end

function add_box()
    local object = E.create_object("../data/box.obj")
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
    local object = E.create_object("../data/ball.obj")
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
    local object = E.create_object("../data/thing.obj")
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
    W = { }

    local alt = 5.00
    local len = 0.50
    local rad = 0.35

    local pos = {
        { -0.85, -0.75 + alt, -1.25 },
        {  0.85, -0.75 + alt, -1.25 },
        { -0.85, -0.75 + alt,  1.30 },
        {  0.85, -0.75 + alt,  1.30 },
    }

    -- Create entities for all car parts.

    body = E.create_object("../data/body.obj")
    W[1] = E.create_object("../data/tire.obj")
    W[2] = E.create_clone(W[1])
    W[3] = E.create_clone(W[1])
    W[4] = E.create_clone(W[1])
    Lbar = E.create_pivot()
    Rbar = E.create_pivot()
    Cbar = E.create_pivot()

    -- Swap some of the body materials to add a chrome effect.

    E.set_brush_image(E.get_mesh(body, 1), envmap)
    E.set_brush_image(E.get_mesh(body, 2), envmap)
    E.set_brush_image(E.get_mesh(body, 4), envmap)

    E.set_brush_flags(E.get_mesh(body, 1), E.brush_flag_environment, true)
    E.set_brush_flags(E.get_mesh(body, 2), E.brush_flag_environment, true)
    E.set_brush_flags(E.get_mesh(body, 4), E.brush_flag_environment, true)

    -- Parent all car entities to the same coordinate system.

    E.parent_entity(body, pivot)
    E.parent_entity(W[1], pivot)
    E.parent_entity(W[2], pivot)
    E.parent_entity(W[3], pivot)
    E.parent_entity(W[4], pivot)
    E.parent_entity(Lbar, pivot)
    E.parent_entity(Rbar, pivot)
    E.parent_entity(Cbar, pivot)

    -- Position all car entities.

    E.set_entity_position(body, 0,  alt, 0)
    E.set_entity_position(W[1], pos[1][1], pos[1][2], pos[1][3])
    E.set_entity_position(W[2], pos[2][1], pos[2][2], pos[2][3])
    E.set_entity_position(W[3], pos[3][1], pos[3][2], pos[3][3])
    E.set_entity_position(W[4], pos[4][1], pos[4][2], pos[4][3])
    E.set_entity_position(Lbar, pos[1][1], pos[1][2], pos[1][3])
    E.set_entity_position(Rbar, pos[2][1], pos[2][2], pos[2][3])
    E.set_entity_position(Cbar, 0,         pos[1][2], pos[1][3] - len)

    -- Mark all entities as rigid bodies.

    E.set_entity_body_type(body, true)
    E.set_entity_body_type(Lbar, true)
    E.set_entity_body_type(Rbar, true)
    E.set_entity_body_type(Cbar, true)
    E.set_entity_body_type(W[1], true)
    E.set_entity_body_type(W[2], true)
    E.set_entity_body_type(W[3], true)
    E.set_entity_body_type(W[4], true)

    -- Define the geoms of all colliding bodies.

    E.set_entity_geom_type(body, E.geom_type_box, 2, 1.125, 4)
    E.set_entity_geom_type(W[1], E.geom_type_sphere, rad)
    E.set_entity_geom_type(W[2], E.geom_type_sphere, rad)
    E.set_entity_geom_type(W[3], E.geom_type_sphere, rad)
    E.set_entity_geom_type(W[4], E.geom_type_sphere, rad)

    -- Define the joints of the steering mechanism.

    E.set_entity_joint_type(body, Lbar, E.joint_type_hinge);
    E.set_entity_joint_type(body, Rbar, E.joint_type_hinge);
    E.set_entity_joint_type(Lbar, Cbar, E.joint_type_hinge);
    E.set_entity_joint_type(Rbar, Cbar, E.joint_type_hinge);

    E.set_entity_joint_attr(body, Lbar, E.joint_attr_anchor,
                            pos[1][1], pos[1][2], pos[1][3])
    E.set_entity_joint_attr(body, Rbar, E.joint_attr_anchor,
                            pos[2][1], pos[2][2], pos[2][3])
    E.set_entity_joint_attr(Lbar, Cbar, E.joint_attr_anchor,
                            pos[1][1], pos[1][2], pos[1][3] - len)
    E.set_entity_joint_attr(Rbar, Cbar, E.joint_attr_anchor,
                            pos[2][1], pos[2][2], pos[2][3] - len)

    E.set_entity_joint_attr(body, Lbar, E.joint_attr_axis_1, 0, 1, 0)
    E.set_entity_joint_attr(body, Rbar, E.joint_attr_axis_1, 0, 1, 0)
    E.set_entity_joint_attr(Lbar, Cbar, E.joint_attr_axis_1, 0, 1, 0)
    E.set_entity_joint_attr(Rbar, Cbar, E.joint_attr_axis_1, 0, 1, 0)

    -- Attach the wheels.

    E.set_entity_joint_type(Lbar, W[1], E.joint_type_hinge_2)
    E.set_entity_joint_type(Rbar, W[2], E.joint_type_hinge_2)
    E.set_entity_joint_type(body, W[3], E.joint_type_hinge_2)
    E.set_entity_joint_type(body, W[4], E.joint_type_hinge_2)

    E.set_entity_joint_attr(Lbar, W[1], E.joint_attr_anchor,
                            pos[1][1], pos[1][2], pos[1][3])
    E.set_entity_joint_attr(Rbar, W[2], E.joint_attr_anchor,
                            pos[2][1], pos[2][2], pos[2][3])
    E.set_entity_joint_attr(body, W[3], E.joint_attr_anchor,
                            pos[3][1], pos[3][2], pos[3][3])
    E.set_entity_joint_attr(body, W[4], E.joint_attr_anchor,
                            pos[4][1], pos[4][2], pos[4][3])

    E.set_entity_joint_attr(Lbar, W[1], E.joint_attr_axis_1, 0, 1, 0)
    E.set_entity_joint_attr(Rbar, W[2], E.joint_attr_axis_1, 0, 1, 0)
    E.set_entity_joint_attr(body, W[3], E.joint_attr_axis_1, 0, 1, 0)
    E.set_entity_joint_attr(body, W[4], E.joint_attr_axis_1, 0, 1, 0)
    E.set_entity_joint_attr(Lbar, W[1], E.joint_attr_axis_2, 1, 0, 0)
    E.set_entity_joint_attr(Rbar, W[2], E.joint_attr_axis_2, 1, 0, 0)
    E.set_entity_joint_attr(body, W[3], E.joint_attr_axis_2, 1, 0, 0)
    E.set_entity_joint_attr(body, W[4], E.joint_attr_axis_2, 1, 0, 0)

    -- Soften the suspension a little.
    
    local erp = 0.10
    local cfm = 0.04

    E.set_entity_joint_attr(Lbar, W[1], E.joint_attr_susp_erp, erp)
    E.set_entity_joint_attr(Lbar, W[1], E.joint_attr_susp_cfm, cfm)
    E.set_entity_joint_attr(Rbar, W[2], E.joint_attr_susp_erp, erp)
    E.set_entity_joint_attr(Rbar, W[2], E.joint_attr_susp_cfm, cfm)
    E.set_entity_joint_attr(body, W[3], E.joint_attr_susp_erp, erp)
    E.set_entity_joint_attr(body, W[3], E.joint_attr_susp_cfm, cfm)
    E.set_entity_joint_attr(body, W[4], E.joint_attr_susp_erp, erp)
    E.set_entity_joint_attr(body, W[4], E.joint_attr_susp_cfm, cfm)

    -- Prevent the wheels from steering on their own.

    E.set_entity_joint_attr(Lbar, W[1], E.joint_attr_lo_stop,  0.00)
    E.set_entity_joint_attr(Lbar, W[1], E.joint_attr_hi_stop,  0.00)
    E.set_entity_joint_attr(Lbar, W[1], E.joint_attr_stop_cfm, 0.05)
    E.set_entity_joint_attr(Lbar, W[1], E.joint_attr_stop_erp, 0.50)

    E.set_entity_joint_attr(Rbar, W[2], E.joint_attr_lo_stop,  0.00)
    E.set_entity_joint_attr(Rbar, W[2], E.joint_attr_hi_stop,  0.00)
    E.set_entity_joint_attr(Rbar, W[2], E.joint_attr_stop_cfm, 0.05)
    E.set_entity_joint_attr(Rbar, W[2], E.joint_attr_stop_erp, 0.50)

    E.set_entity_joint_attr(body, W[3], E.joint_attr_lo_stop,  0.00)
    E.set_entity_joint_attr(body, W[3], E.joint_attr_hi_stop,  0.00)
    E.set_entity_joint_attr(body, W[3], E.joint_attr_stop_cfm, 0.00)
    E.set_entity_joint_attr(body, W[3], E.joint_attr_stop_erp, 0.80)

    E.set_entity_joint_attr(body, W[4], E.joint_attr_lo_stop,  0.00)
    E.set_entity_joint_attr(body, W[4], E.joint_attr_hi_stop,  0.00)
    E.set_entity_joint_attr(body, W[4], E.joint_attr_stop_cfm, 0.00)
    E.set_entity_joint_attr(body, W[4], E.joint_attr_stop_erp, 0.80)

    -- Assign categories to prevent the wheel from fouling in the body.

    E.set_entity_geom_attr(body, E.geom_attr_category, category_body)
    E.set_entity_geom_attr(W[1], E.geom_attr_category, category_tire)
    E.set_entity_geom_attr(W[2], E.geom_attr_category, category_tire)
    E.set_entity_geom_attr(W[3], E.geom_attr_category, category_tire)
    E.set_entity_geom_attr(W[4], E.geom_attr_category, category_tire)

    E.set_entity_geom_attr(body, E.geom_attr_collider, category_world +
                                                       category_body)
    E.set_entity_geom_attr(W[1], E.geom_attr_collider, category_world)
    E.set_entity_geom_attr(W[2], E.geom_attr_collider, category_world)
    E.set_entity_geom_attr(W[3], E.geom_attr_collider, category_world)
    E.set_entity_geom_attr(W[4], E.geom_attr_collider, category_world)

    -- Let everything slide a little on the road.

    E.set_entity_geom_attr(body, E.geom_attr_friction, 10)
    E.set_entity_geom_attr(W[1], E.geom_attr_friction, 50)
    E.set_entity_geom_attr(W[2], E.geom_attr_friction, 50)
    E.set_entity_geom_attr(W[3], E.geom_attr_friction, 50)
    E.set_entity_geom_attr(W[4], E.geom_attr_friction, 50)
end

function go_car(x, y)

    -- Turn the front wheels to match the joystick.

    E.set_entity_joint_attr(body, Lbar, E.joint_attr_lo_stop, max_angle * x)
    E.set_entity_joint_attr(body, Lbar, E.joint_attr_hi_stop, max_angle * x)
    E.set_entity_joint_attr(body, Rbar, E.joint_attr_lo_stop, max_angle * x)
    E.set_entity_joint_attr(body, Rbar, E.joint_attr_hi_stop, max_angle * x)

    -- Apply power to the rear wheels.

    E.set_entity_joint_attr(body, W[3], E.joint_attr_velocity_2, -100 * y)
    E.set_entity_joint_attr(body, W[4], E.joint_attr_velocity_2, -100 * y)

    E.set_entity_joint_attr(body, W[3], E.joint_attr_force_max_2, 15)
    E.set_entity_joint_attr(body, W[4], E.joint_attr_force_max_2, 15)
end

function do_start()
    local L = -0.2083 / 2
    local R =  0.2083 / 2

    camera = E.create_camera(E.camera_type_perspective)
    light  = E.create_light(E.light_type_positional)
    pivot  = E.create_pivot()

    plane = E.create_object("../data/checker.obj")
    envmap = E.create_image("../data/flood_nx.jpg",
                            "../data/flood_px.jpg",
                            "../data/flood_ny.jpg",
                            "../data/flood_py.jpg",
                            "../data/flood_nz.jpg",
                            "../data/flood_pz.jpg")

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
    add_car()
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

        if k == 293 then
            E.exec("../demo.lua")
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

        local kx =  0
        local ky =  3
        local kz =  8

        local x = px + xx * kx + yx * ky + zx * kz
        local y = py + xy * ky + yy * ky + zy * ky
        local z = pz + xz * kx + yz * ky + zz * kz

        local d = math.sqrt((pos_x - px) * (pos_x - px) +
                            (pos_z - pz) * (pos_z - pz));

        local a = math.atan2(pos_x - px, pos_z - pz) * 180 / math.pi
        local b = math.atan2(pos_y - py, d)          * 180 / math.pi
        local k = 25

--      if y < 1 then y = 1 end

        pos_x = ((k - 1) * pos_x + x) / k
        pos_y = ((k - 1) * pos_y + y) / k
        pos_z = ((k - 1) * pos_z + z) / k

        pos_y = math.abs(pos_y)

        E.set_entity_position(camera, pos_x, pos_y, pos_z)
        E.set_entity_rotation(camera, -b, a, 0)

        go_car(jx + key_x, jy + key_y)

        return true
    end
    return false
end

E.set_background(0.8, 0.8, 1.0, 0.2, 0.4, 1.0)
do_start()
