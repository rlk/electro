
-------------------------------------------------------------------------------

camera = nil
galaxy = nil

zoom =   1.0
magn = 100.0
dist =   0.0
spin =   0.0

setzoom = false
setmagn = false
setdist = false

-------------------------------------------------------------------------------

function do_start()
    E.set_background(0, 0, 0, 0, 0, 0)

    camera = E.create_camera(E.camera_type_perspective)
    galaxy = E.create_galaxy("../galaxy_hip.gal")

    E.parent_entity(galaxy, camera)

    E.set_camera_zoom    (camera, zoom)
    E.set_camera_distance(camera, dist)
    E.set_galaxy_magnitude(galaxy, magn)

    return true
end

function do_timer(dt)
    local x, y, z = E.get_entity_rotation(camera)

    E.set_entity_rotation(camera, x, y + dt * spin, z)
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

    if setzoom then      -- Set the camera zoom.

        zoom = zoom + dy * 0.01
        if zoom < 0.001 then
            zoom = 0.001
        end

        E.set_camera_zoom(camera, zoom * zoom)
        E.set_galaxy_magnitude(galaxy, magn / (zoom * zoom))

    elseif setmagn then  -- Set the stellar magnitude multiplier.

        magn = magn - dy * 1.0
        if magn < 0 then
                magn = 0
        end

        E.set_galaxy_magnitude(galaxy, magn / (zoom * zoom))

    elseif setdist then  -- Set the camera distance from the center.

        dist = dist + dy * 0.1

        E.set_camera_distance(camera, dist)

    else                 -- None of the above.  Just pan the camera

        local x, y, z = E.get_entity_rotation(camera)

        x = x - dy * 0.1 * zoom * zoom
        y = y - dx * 0.1 * zoom * zoom

        if x < -90 then x = -90 end
        if x >  90 then x =  90 end

        if y < -180 then y = y + 360 end
        if y >  180 then y = y - 360 end

        E.set_entity_rotation(camera, x, y, z)
    end

    local i       = E.get_star_index(galaxy, camera)
    local x, y, z = E.get_star_position(galaxy, i)

--  E.print_console(string.format("%d %f %f %f\n", i, x, y, z))

    return true
end

function do_click(b, s)
    if b == 1 then setzoom = s end
    if b == 2 then setmagn = s end
    if b == 3 then setdist = s end
    return true
end

-------------------------------------------------------------------------------

