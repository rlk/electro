
button = 0

dx = 0
dy = 0
dz = 0
wx = 0
wy = 0
wz = 0

show_lines = true
magnitude = 500
track = true

-------------------------------------------------------------------------------

fly = false

fly_px = 0
fly_py = 0
fly_pz = 0

fly_xx = 0
fly_xy = 0
fly_xz = 0

fly_yx = 0
fly_yy = 0
fly_yz = 0

fly_zx = 0
fly_zy = 0
fly_zz = 0

function fly_begin()
    fly_px, fly_py, fly_pz = E.get_entity_position(wand)
    fly_xx, fly_xy, fly_xz = E.get_entity_x_vector(wand)
    fly_yx, fly_yy, fly_yz = E.get_entity_y_vector(wand)
    fly_zx, fly_zy, fly_zz = E.get_entity_z_vector(wand)

    fly = true
end

function fly_step(dt)
    if fly then
        local px, py, pz = E.get_entity_position(wand)
        local xx, xy, xz = E.get_entity_x_vector(wand)
        local yx, yy, yz = E.get_entity_y_vector(wand)
        local zx, zy, zz = E.get_entity_z_vector(wand)
        
        local tspeed =  10.0
        local rspeed =  50.0

        local dx = (px - fly_px) * dt * tspeed;
        local dy = (py - fly_py) * dt * tspeed;
        local dz = (pz - fly_pz) * dt * tspeed;

        local vx = fly_zx - zx
        local vy = fly_zy - zy
        local vz = fly_zz - zz

        local wx = fly_yx - yx
        local wy = fly_yy - yy
        local wz = fly_yz - yz

        dX =  (vx * fly_yx + vy * fly_yy + vz * fly_yz) * dt * rspeed
        dY = -(vx * fly_xx + vy * fly_xy + vz * fly_xz) * dt * rspeed
        dZ =  (wx * fly_xx + wy * fly_xy + wz * fly_xz) * dt * rspeed

        E.move_entity(camera, dx, dy, dz)
        E.turn_entity(camera, dX, dY, dZ)
    end
end

function fly_end()
    fly = false
end

-------------------------------------------------------------------------------

function reset()
    local x, y, z = E.get_entity_position(head)
    dx = 0
    dy = 0
    dz = 0
    wx = 0
    wy = 0
    tz = 0
    E.set_entity_position(camera, -x, -y, -z + 3)
    E.set_entity_rotation(camera, 0,  0, 0)
end

function constellation(name, parent)
    local object = E.create_object(name);

    E.parent_entity(object, parent)
    E.set_entity_flags(object, E.entity_flag_line_smooth, true)
    E.set_brush_line_width(E.get_mesh(object, 0), 8)
end

function do_constellations(parent)
    constellation("constellations/And.obj", parent)
    constellation("constellations/Ant.obj", parent)
    constellation("constellations/Aps.obj", parent)
    constellation("constellations/Aql.obj", parent)
    constellation("constellations/Aqr.obj", parent)
    constellation("constellations/Ara.obj", parent)
    constellation("constellations/Ari.obj", parent)
    constellation("constellations/Aur.obj", parent)
    constellation("constellations/Boo.obj", parent)
    constellation("constellations/Cae.obj", parent)
    constellation("constellations/Cam.obj", parent)
    constellation("constellations/Cap.obj", parent)
    constellation("constellations/Car.obj", parent)
    constellation("constellations/Cas.obj", parent)
    constellation("constellations/Cen.obj", parent)
    constellation("constellations/Cep.obj", parent)
    constellation("constellations/Cet.obj", parent)
    constellation("constellations/Cha.obj", parent)
    constellation("constellations/Cir.obj", parent)
    constellation("constellations/CMa.obj", parent)
    constellation("constellations/CMi.obj", parent)
    constellation("constellations/Cnc.obj", parent)
    constellation("constellations/Col.obj", parent)
    constellation("constellations/Com.obj", parent)
    constellation("constellations/CrA.obj", parent)
    constellation("constellations/CrB.obj", parent)
    constellation("constellations/Crt.obj", parent)
    constellation("constellations/Cru.obj", parent)
    constellation("constellations/Crv.obj", parent)
    constellation("constellations/CVn.obj", parent)
    constellation("constellations/Cyg.obj", parent)
    constellation("constellations/Del.obj", parent)
    constellation("constellations/Dor.obj", parent)
    constellation("constellations/Dra.obj", parent)
    constellation("constellations/Equ.obj", parent)
    constellation("constellations/Eri.obj", parent)
    constellation("constellations/For.obj", parent)
    constellation("constellations/Gem.obj", parent)
    constellation("constellations/Gru.obj", parent)
    constellation("constellations/Her.obj", parent)
    constellation("constellations/Hor.obj", parent)
    constellation("constellations/Hya.obj", parent)
    constellation("constellations/Hyi.obj", parent)
    constellation("constellations/Ind.obj", parent)
    constellation("constellations/Lac.obj", parent)
    constellation("constellations/Leo.obj", parent)
    constellation("constellations/Lep.obj", parent)
    constellation("constellations/Lib.obj", parent)
    constellation("constellations/LMi.obj", parent)
    constellation("constellations/Lup.obj", parent)
    constellation("constellations/Lyn.obj", parent)
    constellation("constellations/Lyr.obj", parent)
    constellation("constellations/Men.obj", parent)
    constellation("constellations/Mic.obj", parent)
    constellation("constellations/Mon.obj", parent)
    constellation("constellations/Mus.obj", parent)
    constellation("constellations/Nor.obj", parent)
    constellation("constellations/Oct.obj", parent)
    constellation("constellations/Oph.obj", parent)
    constellation("constellations/Ori.obj", parent)
    constellation("constellations/Pav.obj", parent)
    constellation("constellations/Peg.obj", parent)
    constellation("constellations/Per.obj", parent)
    constellation("constellations/Phe.obj", parent)
    constellation("constellations/Pic.obj", parent)
    constellation("constellations/PsA.obj", parent)
    constellation("constellations/Psc.obj", parent)
    constellation("constellations/Pup.obj", parent)
    constellation("constellations/Pyx.obj", parent)
    constellation("constellations/Ret.obj", parent)
    constellation("constellations/Scl.obj", parent)
    constellation("constellations/Sco.obj", parent)
    constellation("constellations/Sct.obj", parent)
    constellation("constellations/Ser.obj", parent)
    constellation("constellations/Sex.obj", parent)
    constellation("constellations/Sge.obj", parent)
    constellation("constellations/Sgr.obj", parent)
    constellation("constellations/Tau.obj", parent)
    constellation("constellations/Tel.obj", parent)
    constellation("constellations/TrA.obj", parent)
    constellation("constellations/Tri.obj", parent)
    constellation("constellations/Tuc.obj", parent)
    constellation("constellations/UMa.obj", parent)
    constellation("constellations/UMi.obj", parent)
    constellation("constellations/Vel.obj", parent)
    constellation("constellations/Vir.obj", parent)
    constellation("constellations/Vol.obj", parent)
    constellation("constellations/Vul.obj", parent)
end

-------------------------------------------------------------------------------

function do_start()
    local x, y, w, h = E.get_display_union()

    -- Prepare star and sign brushes.

    local star_n_brush = E.create_brush()
    E.set_brush_frag_prog(star_n_brush, "../data/star.fp")
    E.set_brush_vert_prog(star_n_brush, "../data/star.vp")

    local star_f_brush = E.create_brush()
    E.set_brush_frag_prog(star_f_brush, "../data/star.fp")
    E.set_brush_vert_prog(star_f_brush, "../data/star.vp")

    local sign_image = E.create_image("../data/sunsign.png")
    local sign_brush = E.create_brush()
    E.set_brush_image(sign_brush, sign_image)
    E.set_brush_flags(sign_brush, E.brush_flag_unlit, true)

    -- Build the scene graph.

    camera   = E.create_camera(E.camera_type_perspective)
    galaxy_H = E.create_galaxy("../data/galaxy_hip.gal", star_n_brush)
    galaxy_T = E.create_galaxy("../data/galaxy_tyc.gal", star_f_brush)
    sign     = E.create_sprite(sign_brush)
    lines    = E.create_pivot()
    head     = E.create_pivot()
    wand     = E.create_pivot()

    E.set_galaxy_magnitude(galaxy_H, magnitude)
    E.set_galaxy_magnitude(galaxy_T, magnitude)

    E.parent_entity(sign,     camera)
    E.parent_entity(galaxy_H, camera)
    E.parent_entity(galaxy_T, camera)
    E.parent_entity(lines,    camera)
    E.parent_entity(wand,     camera)

    E.set_entity_scale(sign,     0.0050, 0.0050, 0.0050)
    E.set_entity_flags(sign,     E.entity_flag_billboard, true)
    E.set_entity_scale(galaxy_T, 1000.0, 1000.0, 1000.0)
    E.set_entity_flags(galaxy_T, E.entity_flag_ballboard, true)

    E.set_entity_tracking(wand, 1, E.tracking_mode_local)
    E.set_entity_flags   (wand, E.entity_flag_track_pos, true)
    E.set_entity_flags   (wand, E.entity_flag_track_rot, true)

    E.set_entity_tracking(head, 0, E.tracking_mode_local)
    E.set_entity_flags   (head, E.entity_flag_track_pos, true)

    E.set_entity_position(camera, 0, 0, 0)
    E.set_camera_range(camera, 1.0, 10000.0)

    E.set_background(0, 0, 0)

    do_constellations(lines);

    E.enable_timer(true)
    reset()
end

function do_point(x, y)
    if     button == 2 then
        magnitude = magnitude - y
        E.set_galaxy_magnitude(galaxy_H, magnitude)
        E.set_galaxy_magnitude(galaxy_T, magnitude)

    elseif button == 3 then
        dx = dx + x * 0.001
        dy = dy - y * 0.001
    elseif button == 1 then
        dz = dz + y * 0.001
    else
        if E.get_modifier(E.key_modifier_shift) then
            wz = wz - x * 0.001
        else
            wy = wy - x * 0.001
        end
        wx = wx - y * 0.001
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
    local joy_x, joy_y = E.get_joystick(0)
    local s = 8

    if joy_x < -0.1 or 0.1 < joy_x then
        E.turn_entity(camera, 0, -joy_x * dt * 90, 0)
    end
    if joy_y < -0.1 or 0.1 < joy_y then
        E.turn_entity(camera, -joy_y * dt * 90, 0, 0)
    end

    if track then
        fly_step(dt)
    else

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

    end
    return true
end

function do_joystick(d, b, s)
    if b == 3 then
        if s then
            fly_begin()
        else
            fly_end()
        end
    end
    if b == 1 then
        if s then
            reset()
        end
    end
    if b == 2 then
        if s then
            show_lines = not show_lines
            E.set_entity_flags(lines, E.entity_flag_hidden, not show_lines)
        end
    end
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
         -- Tab toggles constellations

        if k == E.key_tab then
            show_lines = not show_lines
            E.set_entity_flags(lines, E.entity_flag_hidden, not show_lines)
        end

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

        if k == E.key_1 then track = true  end
        if k == E.key_2 then track = false end

        return true
    end
end

-------------------------------------------------------------------------------

do_start()
