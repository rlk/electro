
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

    E.parent_entity(galaxy_hip, camera_hip)
    E.parent_entity(galaxy_tyc, camera_tyc)

    E.set_camera_zoom    (camera_hip, zoom)
    E.set_camera_zoom    (camera_tyc, zoom)
    E.set_camera_distance(camera_hip, hip_dist)
    E.set_camera_distance(camera_tyc, tyc_dist)

    E.set_galaxy_magnitude(galaxy_hip, hip_magn)
    E.set_galaxy_magnitude(galaxy_tyc, tyc_magn)

    return true
end

function do_timer(dt)
    local x, y, z = E.get_entity_rotation(camera_hip)

    E.set_entity_rotation(camera_hip, x, y + dt * spin, z)
    E.set_entity_rotation(camera_tyc, x, y + dt * spin, z)
    return true
end

function do_keyboard(k, s)
    if s and k == 284 then -- F3
        spin = spin + 1
    end
    if s and k == 285 then -- F4
        spin = spin - 1
    end

    E.enable_timer(spin ~= 0)

    return true
end

function do_point(dx, dy)
    local shift   = E.get_modifier(E.key_modifier_shift)
    local control = E.get_modifier(E.key_modifier_control)

    if setzoom then      -- Set the camera zoom.

        zoom = zoom + dy * 0.01
        if zoom < 0.001 then
            zoom = 0.001
        end

        E.set_camera_zoom(camera_hip, zoom * zoom)
        E.set_camera_zoom(camera_tyc, zoom * zoom)
        E.set_galaxy_magnitude(galaxy_hip, hip_magn / (zoom * zoom))
        E.set_galaxy_magnitude(galaxy_tyc, tyc_magn / (zoom * zoom))

    elseif setmagn then  -- Set the stellar magnitude multiplier.

        if not shift   then hip_magn = hip_magn - dy * 1.0 end
        if not control then tyc_magn = tyc_magn - dy * 1.0 end

        if hip_magn < 0 then hip_magn = 0 end
        if tyc_magn < 0 then tyc_magn = 0 end

        E.set_galaxy_magnitude(galaxy_hip, hip_magn / (zoom * zoom))
        E.set_galaxy_magnitude(galaxy_tyc, tyc_magn / (zoom * zoom))

    elseif setdist then  -- Set the camera distance from the center.

        if not shift   then hip_dist = hip_dist + dy * 0.1 end
        if not control then tyc_dist = tyc_dist + dy * 0.1 end

        if hip_dist < 0 then hip_dist = 0 end
        if tyc_dist < 0 then tyc_dist = 0 end

        E.set_camera_distance(camera_hip, hip_dist)
        E.set_camera_distance(camera_tyc, tyc_dist)

    else                 -- None of the above.  Just pan the camera

        local x, y, z = E.get_entity_rotation(camera_hip)

        x = x - dy * 0.1 * zoom * zoom
        y = y - dx * 0.1 * zoom * zoom

        if x < -90 then x = -90 end
        if x >  90 then x =  90 end

        if y < -180 then y = y + 360 end
        if y >  180 then y = y - 360 end

        E.set_entity_rotation(camera_hip, x, y, z)
        E.set_entity_rotation(camera_tyc, x, y, z)
    end

    return true
end

function do_click(b, s)
    if b == 1 then setzoom = s end
    if b == 2 then setmagn = s end
    if b == 3 then setdist = s end
    return true
end

-------------------------------------------------------------------------------

