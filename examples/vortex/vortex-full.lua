
button = 0

dx = 0
dy = 0
dz = 0
wx = 0
wy = 0
wz = 0

magnitude = 1000

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

function constellation(name, parent)
    local object = E.create_object(name);

    E.parent_entity(object, parent)
    E.set_entity_flags(object, E.entity_flag_line_smooth, true)
    E.set_brush_line_width(E.get_mesh(object, 0), 2)
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
    sign     = E.create_sprite(sign_brush)
    pointer  = E.create_pivot()

    E.set_galaxy_magnitude(galaxy_H, magnitude)
    E.set_galaxy_magnitude(galaxy_T, magnitude)

    E.parent_entity(sign,     camera)
    E.parent_entity(galaxy_H, camera)
    E.parent_entity(galaxy_T, camera)

    E.set_entity_scale(sign,  0.005, 0.005, 0.005)
    E.set_entity_flags(sign,  E.entity_flag_billboard,   true)

    E.set_entity_position(camera, 0, 0, 1)

    E.set_background(0, 0, 0)

    do_constellations(galaxy_H);

    E.enable_timer(true)
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
