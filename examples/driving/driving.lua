local path = E.path

joy_dev = 0

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

max_angle = 20

base = nil
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

function add_tv(x, y, z)
    local object = E.create_object("../data/tv.obj")
    E.parent_entity(object, pivot)

    E.set_entity_body_type(object, true)
    E.set_entity_geom_type(object, E.geom_type_box, 2, 1.5, 1)
    E.set_entity_geom_attr(object, E.geom_attr_category, category_world)
    E.set_entity_geom_attr(object, E.geom_attr_collider, category_all)
    E.set_entity_geom_attr(object, E.geom_attr_friction, 10)

    if video then
        E.set_brush_image(E.get_mesh(object, 2), video)
    end

    E.set_entity_position(object, x, y, z)

    table.insert(objects, object)
    return object
end

function add_box()
    local object = E.create_object("../data/box.obj")
    E.parent_entity(object, pivot)

    E.set_entity_body_type(object, true)
    E.set_entity_geom_type(object, E.geom_type_box, 2, 2, 2)
    E.set_entity_geom_attr(object, E.geom_attr_category, category_world)
    E.set_entity_geom_attr(object, E.geom_attr_collider, category_all)
    E.set_entity_geom_attr(object, E.geom_attr_friction, 10)

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

    E.set_brush_image(E.get_mesh(object, 2), envmap, 1)
    E.set_brush_flags(E.get_mesh(object, 2), E.brush_flag_env_map_1, true)

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
        { -0.85, 0.05 + alt, -1.25 },
        {  0.85, 0.05 + alt, -1.25 },
        { -0.85, 0.05 + alt,  1.30 },
        {  0.85, 0.05 + alt,  1.30 },
    }

    -- Create entities for all car parts.

    base = E.create_pivot()
    body = E.create_object("../data/body.obj")
    W[1] = E.create_object("../data/tire.obj")
    W[2] = E.create_clone(W[1])
    W[3] = E.create_clone(W[1])
    W[4] = E.create_clone(W[1])

    -- Swap some of the body materials to add a chrome effect.

    E.set_brush_image(E.get_mesh(body, 0), envmap)
    E.set_brush_image(E.get_mesh(body, 5), envmap)

    E.set_brush_flags(E.get_mesh(body, 0), E.brush_flag_env_map_0, true)
    E.set_brush_flags(E.get_mesh(body, 5), E.brush_flag_env_map_0, true)

    -- Parent all car entities to the same coordinate system.

    E.parent_entity(base, later)
    E.parent_entity(body, base)
    E.parent_entity(W[1], later)
    E.parent_entity(W[2], later)
    E.parent_entity(W[3], later)
    E.parent_entity(W[4], later)

    -- Position all car entities.

    E.set_entity_position(base, 0, alt,  0)
    E.set_entity_position(body, 0, 0.75, 0)
    E.set_entity_position(W[1], pos[1][1], pos[1][2], pos[1][3])
    E.set_entity_position(W[2], pos[2][1], pos[2][2], pos[2][3])
    E.set_entity_position(W[3], pos[3][1], pos[3][2], pos[3][3])
    E.set_entity_position(W[4], pos[4][1], pos[4][2], pos[4][3])

    -- Mark all entities as rigid bodies.

    E.set_entity_body_type(base, true)
    E.set_entity_body_type(W[1], true)
    E.set_entity_body_type(W[2], true)
    E.set_entity_body_type(W[3], true)
    E.set_entity_body_type(W[4], true)

    -- Define the geoms of all colliding bodies.

    E.set_entity_geom_type(base, E.geom_type_box, 2, 0.100, 4)
    E.set_entity_geom_type(body, E.geom_type_box, 2, 1.500, 4)
    E.set_entity_geom_type(W[1], E.geom_type_sphere, rad)
    E.set_entity_geom_type(W[2], E.geom_type_sphere, rad)
    E.set_entity_geom_type(W[3], E.geom_type_sphere, rad)
    E.set_entity_geom_type(W[4], E.geom_type_sphere, rad)

    E.set_entity_geom_attr(base, E.geom_attr_mass, 5.0)
    E.set_entity_geom_attr(body, E.geom_attr_mass, 0.0)
    E.set_entity_geom_attr(W[1], E.geom_attr_mass, 0.1)
    E.set_entity_geom_attr(W[2], E.geom_attr_mass, 0.1)
    E.set_entity_geom_attr(W[3], E.geom_attr_mass, 0.1)
    E.set_entity_geom_attr(W[4], E.geom_attr_mass, 0.1)

    -- Attach the wheels.

    E.set_entity_joint_type(base, W[1], E.joint_type_hinge_2)
    E.set_entity_joint_type(base, W[2], E.joint_type_hinge_2)
    E.set_entity_joint_type(base, W[3], E.joint_type_hinge_2)
    E.set_entity_joint_type(base, W[4], E.joint_type_hinge_2)

    E.set_entity_joint_attr(base, W[1], E.joint_attr_anchor,
                            pos[1][1], pos[1][2], pos[1][3])
    E.set_entity_joint_attr(base, W[2], E.joint_attr_anchor,
                            pos[2][1], pos[2][2], pos[2][3])
    E.set_entity_joint_attr(base, W[3], E.joint_attr_anchor,
                            pos[3][1], pos[3][2], pos[3][3])
    E.set_entity_joint_attr(base, W[4], E.joint_attr_anchor,
                            pos[4][1], pos[4][2], pos[4][3])

    E.set_entity_joint_attr(base, W[1], E.joint_attr_axis_1, 0, 1, 0)
    E.set_entity_joint_attr(base, W[2], E.joint_attr_axis_1, 0, 1, 0)
    E.set_entity_joint_attr(base, W[3], E.joint_attr_axis_1, 0, 1, 0)
    E.set_entity_joint_attr(base, W[4], E.joint_attr_axis_1, 0, 1, 0)
    E.set_entity_joint_attr(base, W[1], E.joint_attr_axis_2, 1, 0, 0)
    E.set_entity_joint_attr(base, W[2], E.joint_attr_axis_2, 1, 0, 0)
    E.set_entity_joint_attr(base, W[3], E.joint_attr_axis_2, 1, 0, 0)
    E.set_entity_joint_attr(base, W[4], E.joint_attr_axis_2, 1, 0, 0)

    -- Soften the suspension a little.
    
    local erp = 0.80
    local cfm = 0.05

    E.set_entity_joint_attr(base, W[1], E.joint_attr_susp_erp, erp)
    E.set_entity_joint_attr(base, W[1], E.joint_attr_susp_cfm, cfm)
    E.set_entity_joint_attr(base, W[2], E.joint_attr_susp_erp, erp)
    E.set_entity_joint_attr(base, W[2], E.joint_attr_susp_cfm, cfm)
    E.set_entity_joint_attr(base, W[3], E.joint_attr_susp_erp, erp)
    E.set_entity_joint_attr(base, W[3], E.joint_attr_susp_cfm, cfm)
    E.set_entity_joint_attr(base, W[4], E.joint_attr_susp_erp, erp)
    E.set_entity_joint_attr(base, W[4], E.joint_attr_susp_cfm, cfm)

    E.set_entity_joint_attr(base, W[1], E.joint_attr_cfm, 0)
    E.set_entity_joint_attr(base, W[2], E.joint_attr_cfm, 0)
    E.set_entity_joint_attr(base, W[3], E.joint_attr_cfm, 0)
    E.set_entity_joint_attr(base, W[4], E.joint_attr_cfm, 0)

    -- Prevent the wheels from steering on their own.

    E.set_entity_joint_attr(base, W[1], E.joint_attr_lo_stop,  0.00)
    E.set_entity_joint_attr(base, W[1], E.joint_attr_hi_stop,  0.00)
    E.set_entity_joint_attr(base, W[1], E.joint_attr_stop_cfm, 0.05)
    E.set_entity_joint_attr(base, W[1], E.joint_attr_stop_erp, 0.50)

    E.set_entity_joint_attr(base, W[2], E.joint_attr_lo_stop,  0.00)
    E.set_entity_joint_attr(base, W[2], E.joint_attr_hi_stop,  0.00)
    E.set_entity_joint_attr(base, W[2], E.joint_attr_stop_cfm, 0.05)
    E.set_entity_joint_attr(base, W[2], E.joint_attr_stop_erp, 0.50)

    E.set_entity_joint_attr(base, W[3], E.joint_attr_lo_stop,  0.00)
    E.set_entity_joint_attr(base, W[3], E.joint_attr_hi_stop,  0.00)
    E.set_entity_joint_attr(base, W[3], E.joint_attr_stop_cfm, 0.00)
    E.set_entity_joint_attr(base, W[3], E.joint_attr_stop_erp, 1.00)

    E.set_entity_joint_attr(base, W[4], E.joint_attr_lo_stop,  0.00)
    E.set_entity_joint_attr(base, W[4], E.joint_attr_hi_stop,  0.00)
    E.set_entity_joint_attr(base, W[4], E.joint_attr_stop_cfm, 0.00)
    E.set_entity_joint_attr(base, W[4], E.joint_attr_stop_erp, 1.00)

    -- Assign categories to prevent the wheel from fouling in the base.

    E.set_entity_geom_attr(base, E.geom_attr_category, category_body)
    E.set_entity_geom_attr(body, E.geom_attr_category, category_body)
    E.set_entity_geom_attr(W[1], E.geom_attr_category, category_tire)
    E.set_entity_geom_attr(W[2], E.geom_attr_category, category_tire)
    E.set_entity_geom_attr(W[3], E.geom_attr_category, category_tire)
    E.set_entity_geom_attr(W[4], E.geom_attr_category, category_tire)

    E.set_entity_geom_attr(base, E.geom_attr_collider, category_world +
                                                       category_body)
    E.set_entity_geom_attr(body, E.geom_attr_collider, category_world +
                                                       category_body)
    E.set_entity_geom_attr(W[1], E.geom_attr_collider, category_world)
    E.set_entity_geom_attr(W[2], E.geom_attr_collider, category_world)
    E.set_entity_geom_attr(W[3], E.geom_attr_collider, category_world)
    E.set_entity_geom_attr(W[4], E.geom_attr_collider, category_world)

    -- Let everything slide a little on the road.

    E.set_entity_geom_attr(base, E.geom_attr_friction,   0)
    E.set_entity_geom_attr(body, E.geom_attr_friction,  10)
    E.set_entity_geom_attr(W[1], E.geom_attr_friction, 100)
    E.set_entity_geom_attr(W[2], E.geom_attr_friction, 100)
    E.set_entity_geom_attr(W[3], E.geom_attr_friction, 100)
    E.set_entity_geom_attr(W[4], E.geom_attr_friction, 100)
end

function go_car(x, y)

    -- Turn the front wheels to match the joystick.

    E.set_entity_joint_attr(base, W[1], E.joint_attr_lo_stop, max_angle * x)
    E.set_entity_joint_attr(base, W[1], E.joint_attr_hi_stop, max_angle * x)
    E.set_entity_joint_attr(base, W[2], E.joint_attr_lo_stop, max_angle * x)
    E.set_entity_joint_attr(base, W[2], E.joint_attr_hi_stop, max_angle * x)

    -- Apply power to the rear wheels.

    E.set_entity_joint_attr(base, W[3], E.joint_attr_velocity_2, -100 * y)
    E.set_entity_joint_attr(base, W[4], E.joint_attr_velocity_2, -100 * y)

    E.set_entity_joint_attr(base, W[3], E.joint_attr_force_max_2, 10)
    E.set_entity_joint_attr(base, W[4], E.joint_attr_force_max_2, 10)
end

function do_start()
    local L = -0.2083 / 2
    local R =  0.2083 / 2

    camera = E.create_camera(E.camera_type_perspective)
    light  = E.create_light(E.light_type_positional)
    pivot  = E.create_pivot()
    later  = E.create_pivot()
    sky    = E.create_object("../data/sky.obj")

    video = E.create_image(2827, 640, 480, 12)
    plane = E.create_object("../data/checker.obj")
    envmap = E.create_image("../data/sky_nx.png",
                            "../data/sky_px.png",
                            "../data/sky_ny.png",
                            "../data/sky_py.png",
                            "../data/sky_nz.png",
                            "../data/sky_pz.png")

    E.parent_entity(light, camera)
    E.parent_entity(pivot, light)
    E.parent_entity(later, pivot)
    E.parent_entity(plane, pivot)
    E.parent_entity(sky,   plane)

    E.set_entity_geom_type(plane, E.geom_type_plane, 0, 1, 0, 0)
    E.set_entity_geom_attr(plane, E.geom_attr_category, category_world)
    E.set_entity_geom_attr(plane, E.geom_attr_collider, category_all)

    E.set_entity_position(light, 0.0, 16.0,  0.0)
    E.set_entity_scale   (plane, 32, 32, 32)
    E.set_entity_scale   (sky, 8, 8, 8)

    E.set_brush_image(E.get_mesh(sky, 0), envmap)
    E.set_brush_flags(E.get_mesh(sky, 0), E.brush_flag_unlit,     true)
    E.set_brush_flags(E.get_mesh(sky, 0), E.brush_flag_sky_map_0, true)

    add_ramp(0, 0, 0, 10, 0)
    add_ramp(32, 0, 10, 30, 0)
    add_ramp(-32, 0, -10, -15, 180)

    add_tv(-4, 0.75, -20)
    add_tv(-2, 0.75, -20)
    add_tv( 0, 0.75, -20)
    add_tv( 2, 0.75, -20)
    add_tv( 4, 0.75, -20)
    add_tv(-4, 2.25, -20)
    add_tv(-2, 2.25, -20)
    add_tv( 0, 2.25, -20)
    add_tv( 2, 2.25, -20)
    add_tv( 4, 2.25, -20)
    add_tv(-4, 3.75, -20)
    add_tv(-2, 3.75, -20)
    add_tv( 0, 3.75, -20)
    add_tv( 2, 3.75, -20)
    add_tv( 4, 3.75, -20)

    E.enable_timer(true)
    add_car()
end

function do_joystick(d, b, s)
    if d == joy_dev then
        if s then
            if b == 1 then
                add_box()
            end
            if b == 2 then
                add_ball()
            end
            if b == 3 then
                add_car()
            end
            if b == 4 then
                right = true
            end
        else
            if b == 4 then
                right = false
            end
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
        if k == E.key_1 then
            add_box()
        end
        if k == E.key_2 then
            add_ball()
        end
        if k == E.key_3 then
            add_car()
        end
        if k == E.key_4 then
            add_tv(0, 10, 0)
        end
        if k == E.key_tab then
            right = true
        end

        if k == E.key_F6 then
            E.set_entity_flags(camera, E.entity_flag_wireframe, true)
            return true
        end
        if k == E.key_F7 then
            E.set_entity_flags(camera, E.entity_flag_wireframe, false)
            return true
        end

        if k == E.key_F12 then
            E.nuke()
            E.chdir("..")
            dofile("demo.lua")
            return true
        end

        if k == E.key_up    then key_y = key_y - 1 end
        if k == E.key_down  then key_y = key_y + 1 end
        if k == E.key_right then key_x = key_x + 1 end
        if k == E.key_left  then key_x = key_x - 1 end
    else
        if k == E.key_tab then
            right = false
        end

        if k == E.key_up    then key_y = key_y + 1 end
        if k == E.key_down  then key_y = key_y - 1 end
        if k == E.key_right then key_x = key_x - 1 end
        if k == E.key_left  then key_x = key_x + 1 end
    end
    return false
end

total_dt = 0

function do_timer(dt)
    if base then
        local jx, jy = E.get_joystick(joy_dev)

        local px, py, pz = E.get_entity_position(base)
        local xx, xy, xz = E.get_entity_x_vector(base)
        local yx, yy, yz = E.get_entity_y_vector(base)
        local zx, zy, zz = E.get_entity_z_vector(base)

        local kx =  0
        local ky =  2
        local kz = 10

        local x = px + xx * kx + yx * ky + zx * kz
        local y = py + xy * ky + yy * ky + zy * ky
        local z = pz + xz * kx + yz * ky + zz * kz

        local d = math.sqrt((pos_x - px) * (pos_x - px) +
                            (pos_z - pz) * (pos_z - pz));

        local a = math.atan2(pos_x - px, pos_z - pz) * 180 / math.pi
        local b = math.atan2(pos_y - py, d)          * 180 / math.pi
        local k = 20

        pos_x = ((k - 1) * pos_x + x) / k
        pos_y = ((k - 1) * pos_y + y) / k
        pos_z = ((k - 1) * pos_z + z) / k

        pos_y = math.abs(pos_y)

        E.set_entity_position(camera, pos_x, pos_y, pos_z)
        E.set_entity_rotation(camera, -b, a, 0)

        go_car(jx + key_x, jy + key_y)

        if right then
            E.add_entity_force(W[1], 0, 20, 0)
            E.add_entity_force(W[3], 0, 20, 0)
        end

        return true
    end
    return false
end

E.set_background(0.8, 0.8, 1.0, 0.2, 0.4, 1.0)
do_start()
