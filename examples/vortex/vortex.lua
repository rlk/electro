
-------------------------------------------------------------------------------

camera_tyc = nil
camera_hip = nil
galaxy_tyc = nil
galaxy_hip = nil

zoom     =   1.0
hip_magn = 100.0
tyc_magn = 100.0
hip_dist =   0.0
tyc_dist =   0.0

d_zoom     = 0.0
d_hip_dist = 0.0
d_tyc_dist = 0.0
d_rot_x    = 0.0
d_rot_y    = 0.0
d_rot_z    = 0.0

setzoom = false
setmagn = false
setdist = false

-------------------------------------------------------------------------------

function do_start()
    E.set_background(0, 0, 0, 0, 0, 0)

    camera_tyc = E.create_camera(E.camera_type_perspective)
    camera_hip = E.create_camera(E.camera_type_perspective)
    galaxy_tyc = E.create_galaxy("../galaxy_tyc.gal")
    galaxy_hip = E.create_galaxy("../galaxy_hip.gal")

    E.set_entity_vert_prog(galaxy_hip, "../star.vp");
    E.set_entity_frag_prog(galaxy_hip, "../star.fp");
    E.set_entity_vert_prog(galaxy_tyc, "../star.vp");
    E.set_entity_frag_prog(galaxy_tyc, "../star.fp");

    E.parent_entity(galaxy_hip, camera_hip)
    E.parent_entity(galaxy_tyc, camera_tyc)

    E.set_camera_zoom    (camera_hip, zoom)
    E.set_camera_zoom    (camera_tyc, zoom)
    E.set_camera_distance(camera_hip, hip_dist)
    E.set_camera_distance(camera_tyc, tyc_dist)

    E.set_galaxy_magnitude(galaxy_hip, hip_magn)
    E.set_galaxy_magnitude(galaxy_tyc, tyc_magn)

    E.enable_timer(true)

    E.print_console("---------- CONTROLS ---------\n")
    E.print_console("F1:          toggle console\n")
    E.print_console("F2:          toggle server draw\n")
    E.print_console("Space bar:   stop motion\n")
    E.print_console("Enter:       re-center camera\n")
    E.print_console("Mouse move:  camera pan\n")
    E.print_console("Left drag:   camera zoom\n")
    E.print_console("Middle drag: star magnitude\n")
    E.print_console("Right drag:  camera distance\n")

    return true
end

function do_timer(dt)
    local rot_x, rot_y, rot_z = E.get_entity_rotation(camera_hip);

    zoom     = zoom     + d_zoom     * dt;
    hip_dist = hip_dist + d_hip_dist * dt;
    tyc_dist = tyc_dist + d_tyc_dist * dt;
    rot_x    = rot_x    + d_rot_x    * dt;
    rot_y    = rot_y    + d_rot_y    * dt;
    rot_z    = rot_z    + d_rot_z    * dt;

    if rot_x <  -90 then rot_x = -90 end
    if rot_x >   90 then rot_x =  90 end

    if rot_y < -180 then rot_y = rot_y + 360 end
    if rot_y >  180 then rot_y = rot_y - 360 end

    E.set_camera_distance(camera_hip, hip_dist)
    E.set_camera_distance(camera_tyc, tyc_dist)
--  E.set_camera_zoom(camera_hip, zoom * zoom)
--  E.set_camera_zoom(camera_tyc, zoom * zoom)

    E.set_galaxy_magnitude(galaxy_hip, hip_magn)
    E.set_galaxy_magnitude(galaxy_tyc, tyc_magn)

    E.set_entity_rotation(camera_hip, rot_x, rot_y, rot_z)
    E.set_entity_rotation(camera_tyc, rot_x, rot_y, rot_z)

    return true
end

function do_keyboard(k, s)
    if s and k == 32 then
        d_zoom     = 0
        d_hip_dist = 0
        d_tyc_dist = 0
        d_rot_x    = 0
        d_rot_y    = 0
        d_rot_z    = 0
    end
    if s and k == 13 then
        hip_dist = 0
        tyc_dist = 0
        E.set_camera_distance(camera_hip, hip_dist)
        E.set_camera_distance(camera_tyc, tyc_dist)
    end
end

function do_point(dx, dy)
    local shift   = E.get_modifier(E.key_modifier_shift)
    local control = E.get_modifier(E.key_modifier_control)

    if setzoom then      -- Set the camera zoom.

        d_zoom = d_zoom + dy * 0.001

    elseif setmagn then  -- Set the stellar magnitude multiplier.

        if not shift   then hip_magn = hip_magn - dy * 1.0 end
        if not control then tyc_magn = tyc_magn - dy * 1.0 end

--      E.set_galaxy_magnitude(galaxy_hip, hip_magn / (zoom * zoom))
--      E.set_galaxy_magnitude(galaxy_tyc, tyc_magn / (zoom * zoom))

    elseif setdist then  -- Set the camera distance from the center.

        if not shift   then d_hip_dist = d_hip_dist + dy * 0.1 end
        if not control then d_tyc_dist = d_tyc_dist + dy * 0.1 end

    else                 -- None of the above.  Just pan the camera

        d_rot_x = d_rot_x - dy * 0.005 * zoom
    end

    d_rot_y = d_rot_y - dx * 0.005 * zoom

    return true
end

function do_click(b, s)
    if b == 1 then setzoom = s end
    if b == 2 then setmagn = s end
    if b == 3 then setdist = s end
    return true
end

-------------------------------------------------------------------------------

