
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
spin     =   0.0

count = 0

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

    return true
end

function do_frame()
    count = count + 1

    local x, y, z = E.get_entity_rotation(camera_hip)

    E.set_entity_rotation(camera_hip, x, y + 0.1, z)
    E.set_entity_rotation(camera_tyc, x, y + 0.1, z)

    if (count > 2000) then
        E.exit()
    end

    return true
end

function do_timer(dt)
    return true
end

-------------------------------------------------------------------------------

