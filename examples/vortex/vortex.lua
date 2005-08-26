
-------------------------------------------------------------------------------

camera_tyc = nil
camera_hip = nil
galaxy_tyc = nil
galaxy_hip = nil

hip_magn = 100.0
tyc_magn = 100.0
hip_dist =   0.0
tyc_dist =   0.0
rot_x    =   0.0
rot_y    =   0.0
rot_z    =   0.0
pos_x    =   0.0
pos_y    =   0.0
pos_z    =   0.0

d_hip_dist = 0.0
d_tyc_dist = 0.0
d_rot_x    = 0.0
d_rot_y    = 0.0
d_rot_z    = 0.0

hide_tyc = false
hide_hip = false

setmagn = false
setdist = false

-------------------------------------------------------------------------------

function do_start()
    local brush = E.create_brush()
        
    E.set_brush_frag_prog(brush, "../data/star.fp")
    E.set_brush_vert_prog(brush, "../data/star.vp")

    E.set_background(0, 0, 0, 0, 0, 0)

    camera_tyc = E.create_camera(E.camera_type_perspective)
    camera_hip = E.create_camera(E.camera_type_perspective)
    galaxy_tyc = E.create_galaxy("../data/galaxy_tyc.gal", brush)
    galaxy_hip = E.create_galaxy("../data/galaxy_hip.gal", brush)

    E.parent_entity(galaxy_hip, camera_hip)
    E.parent_entity(galaxy_tyc, camera_tyc)

    E.set_galaxy_magnitude(galaxy_hip, hip_magn)
    E.set_galaxy_magnitude(galaxy_tyc, tyc_magn)

    E.enable_timer(true)

--  E.print_console("---------- CONTROLS ---------\n")
--  E.print_console("F1:          toggle console\n")
--  E.print_console("F2:          toggle server draw\n")
--  E.print_console("Space bar:   stop motion\n")
--  E.print_console("Enter:       re-center camera\n")
--  E.print_console("Mouse move:  camera pan\n")
--  E.print_console("Left drag:   camera zoom\n")
--  E.print_console("Middle drag: star magnitude\n")
--  E.print_console("Right drag:  camera distance\n")

    return true
end

function do_timer(dt)

    hip_dist = hip_dist + d_hip_dist * dt;
    tyc_dist = tyc_dist + d_tyc_dist * dt;
    rot_x    = rot_x    + d_rot_x    * dt;
    rot_y    = rot_y    + d_rot_y    * dt;
    rot_z    = rot_z    + d_rot_z    * dt;

    if rot_x <  -90 then rot_x = -90 end
    if rot_x >   90 then rot_x =  90 end

    if rot_y < -180 then rot_y = rot_y + 360 end
    if rot_y >  180 then rot_y = rot_y - 360 end

    local T = math.pi * rot_y / 180
    local P = math.pi * rot_x / 180

    -- set the new magnitudes

    E.set_galaxy_magnitude(galaxy_hip, hip_magn)
    E.set_galaxy_magnitude(galaxy_tyc, tyc_magn)

    -- set the hip position and rotation

    pos_x =  math.cos(P) * math.sin(T) * hip_dist
    pos_y = -math.sin(P)               * hip_dist
    pos_z =  math.cos(P) * math.cos(T) * hip_dist

    E.set_entity_position(camera_hip, pos_x, pos_y, pos_z)
    E.set_entity_rotation(camera_hip, rot_x, rot_y, rot_z)

    -- set the tyc position and rotation

    pos_x =  math.cos(P) * math.sin(T) * tyc_dist
    pos_y = -math.sin(P)               * tyc_dist
    pos_z =  math.cos(P) * math.cos(T) * tyc_dist

    E.set_entity_position(camera_tyc, pos_x, pos_y, pos_z)
    E.set_entity_rotation(camera_tyc, rot_x, rot_y, rot_z)

    return true
end

function do_keyboard(k, s)

    if s and k == E.key_space then
        d_hip_dist = 0
        d_tyc_dist = 0
        d_rot_x    = 0
        d_rot_y    = 0
        d_rot_z    = 0
    end
    if s and k == E.key_return then
        hip_dist = 0
        tyc_dist = 0
        rot_x    = 0
        rot_y    = 0
        rot_z    = 0

        E.set_entity_rotation(camera_hip, rot_x, rot_y, rot_z)
        E.set_entity_rotation(camera_tyc, rot_x, rot_y, rot_z)
    end

    if s and k == E.key_F12 then
        E.exec("../demo.lua")
    end

    return true
end

function do_point(dx, dy)
    local shift   = E.get_modifier(E.key_modifier_shift)
    local control = E.get_modifier(E.key_modifier_control)

    if setmagn then  -- Set the stellar magnitude multiplier.

        if not shift   then hip_magn = hip_magn - dy * 1.0 end
        if not control then tyc_magn = tyc_magn - dy * 1.0 end

    elseif setdist then  -- Set the camera distance from the center.

        d_hip_dist = d_hip_dist + dy * 0.1

    else                 -- None of the above.  Just pan the camera

        d_rot_x = d_rot_x - dy * 0.005
    end

    d_rot_y = d_rot_y - dx * 0.005

    return true
end

function do_click(b, s)
    if b == 2 then setmagn = s end
    if b == 3 then setdist = s end
    return true
end

-------------------------------------------------------------------------------

do_start()
