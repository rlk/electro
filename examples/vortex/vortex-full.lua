
button = 0

dx = 0
dy = 0
dz = 0
wx = 0
wy = 0
wz = 0

magnitude = 100

-------------------------------------------------------------------------------

function reset()
    dx = 0
    dy = 0
    dz = 0
    wx = 0
    wy = 0
    tz = 0
    E.set_entity_position(camera, 0, 0, 1)
    E.set_entity_rotation(camera, 0, 0, 0)
end

-------------------------------------------------------------------------------

function do_start()
    local x, y, w, h = E.get_display_union()

    magnitude = w / 5

    -- Prepare star and sign brushes.

    local star_brush = E.create_brush()
    E.set_brush_frag_prog(star_brush, "../data/star.fp")
    E.set_brush_vert_prog(star_brush, "../data/star.vp")

    local sign_image = E.create_image("../data/sunsign.png")
    local sign_brush = E.create_brush()
    E.set_brush_image(sign_brush, sign_image)
    E.set_brush_flags(sign_brush, E.brush_flag_unlit, true)

    -- Build the scene graph.

    camera   = E.create_camera(E.camera_type_perspective)
    galaxy_H = E.create_galaxy("../data/galaxy_hip.gal", star_brush)
    galaxy_T = E.create_galaxy("../data/galaxy_tyc.gal", star_brush)
    orion    = E.create_object("../data/orion.obj")
    sign     = E.create_sprite(sign_brush)
    pointer  = E.create_pivot()

    E.set_galaxy_magnitude(galaxy_H, magnitude)
    E.set_galaxy_magnitude(galaxy_T, magnitude)

    E.parent_entity(sign,     camera)
    E.parent_entity(galaxy_H, camera)
    E.parent_entity(galaxy_T, camera)
    E.parent_entity(orion,    camera)

    E.set_entity_scale(sign,  0.005, 0.005, 0.005)
    E.set_entity_flags(orion, E.entity_flag_line_smooth, true)
    E.set_entity_flags(sign,  E.entity_flag_billboard,   true)

    E.set_entity_position(camera, 0, 0, 1)

    E.set_brush_flags(E.get_mesh(orion, 0), E.brush_flag_unlit, true)
    E.set_brush_line_width(E.get_mesh(orion, 0), 8)

    E.set_background(0, 0, 0)

    E.enable_timer(true)
end

function do_point(x, y)
    if     button == 2 then
        magnitude = magnitude - y
        E.set_galaxy_magnitude(galaxy_H, magnitude)
        E.set_galaxy_magnitude(galaxy_T, magnitude)

    elseif button == 3 then
        dx = dx + x * 0.001
    elseif button == 1 then
        dz = dz + y * 0.001
    else
        wx = wx - y * 0.001
        wy = wy - x * 0.001
    end
end

function do_click(b, s)
    if s then
        button = b
    else
        button = 0
    end
end

function do_timer(dt)
    local x, y, z

    -- If braking, nullify all velocities.

    if braking then
        local k = 1.0 - dt * 2.0

        dx = dx * k
        dy = dy * k
        dz = dz * k
        wx = wx * k
        wy = wy * k
        wz = wz * k
    end

    -- Apply all velocities to the camera.

    E.turn_entity(camera, wx, wy, wz);
    E.move_entity(camera, dx, dy, dz);

    -- Center the Tycho data set about the camera

    x, y, z = E.get_entity_position(camera);

    E.set_entity_position(galaxy_T, x, y, z);

    return true
end

function do_keyboard(k, s)
    if varrier_keyboard then
        if varrier_keyboard(k, s, camera) then
            return true
        end
    end

    -- Space bar applies the brakes

    if k == E.key_space then
        braking = s
    end

    if s then
        -- Recenter the view on Return

        if k == E.key_return then
            reset()
        end

        -- Return to the demo menu on F12

        if k == E.key_F12 then
            E.nuke()
            E.chdir("..")
            dofile("demo.lua")
        end

        return true
    end
end

-------------------------------------------------------------------------------

do_start()
