
tumble = false
scale  = false

size  = 1
rot_x = 0
rot_y = 0

function do_start()
    local X0, Y0, Z0, X1, Y1, Z1 = E.get_display_bound()

    local XC = (X0 + X1) / 2
    local YC = (Y0 + Y1) / 2
    local ZC = (Z0 + Z1) / 2

    E.set_background(1, 0, 0, 0, 1, 0)

    -- Create the on-screen scene.

    camera0 = E.create_camera(E.camera_type_perspective)
    light0  = E.create_light(E.light_type_positional)
    object0 = E.create_object("../data/box.obj")

    E.parent_entity(light0, camera0)
    E.parent_entity(object0, light0)

    E.set_entity_position(light0,  XC + 1, YC + 4, ZC + 8)
    E.set_entity_position(object0, XC, YC, ZC)

    -- Create the off-screen scene.

    camera1 = E.create_camera(E.camera_type_perspective)
    light1  = E.create_light(E.light_type_positional)
    object1 = E.create_object("../data/cow.obj")

    E.parent_entity(light1, camera1)
    E.parent_entity(object1, light1)

    E.set_entity_position(light1,   1,  1,  1)
    E.set_entity_position(object1,  0,  0, -3)

    -- Scale the box to fit the screen.

    local x0, y0, z0, x1, y1, z1 = E.get_entity_bound(object0)

    size = 0.5 * (Y0 - Y1) / (y0 - y1)

    E.set_entity_scale(object0, size, size, size);
    E.set_entity_scale(object1, 0.20, 0.20, 0.20);

    -- Assign an image to the off-screen camera and the on-screen box.

    image  = E.create_image(256, 256, 3)

    E.set_brush_image(E.get_mesh(object0, 0), image)

    E.set_camera_range(camera1, 1.0, 100.0)
    E.set_camera_image(camera1, image, -0.5, 0.5, -0.5, 0.5)
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

        E.set_entity_rotation(object0, rot_x, rot_y, 0)
        E.set_entity_rotation(object1, rot_x, rot_y, 0)

        return true
    end

    if scale then
        size = size + dy / 500

        E.set_entity_scale(object0, size, size, size)

        return true
    end

    return false
end

function do_keyboard(k, s)
    local d = 0.125

    if varrier_keyboard then
        if varrier_keyboard(k, s, camera0) then
            return true
        end
    end

    return false
end

-------------------------------------------------------------------------------

do_start()

E.enable_timer(true)