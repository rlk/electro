
-------------------------------------------------------------------------------

camera = nil
galaxy = nil
hilite = nil
marker = nil

zoom =   1.0
magn = 100.0
dist =   0.0
spin =   0.0

setzoom = false
setmagn = false
setdist = false

vert_point = 0
vert_count = 1
vert_start = 1
serial = 1

constellation = { }

-------------------------------------------------------------------------------

function add_constellation(name)
    constellation[serial] = E.create_object(name)

    E.set_entity_alpha(constellation[serial], 0.8)
    E.set_entity_flag(constellation[serial], E.entity_flag_unlit,       true)
    E.set_entity_flag(constellation[serial], E.entity_flag_line_smooth, true)
    E.parent_entity  (constellation[serial], camera)

    serial = serial + 1
end

-------------------------------------------------------------------------------

function do_start()
    E.set_background(0, 0, 0, 0, 0, 0)

    camera = E.create_camera(E.camera_type_perspective)
    galaxy = E.create_galaxy("../galaxy_hip.gal")
    hilite = E.create_sprite("hilite.png")
    marker = E.create_sprite("here.png")

    E.parent_entity(galaxy, camera)
    E.parent_entity(hilite, galaxy)
    E.parent_entity(marker, galaxy)

    add_constellation("cassiopeia.obj")
    add_constellation("orion.obj")
    
    E.set_camera_zoom    (camera, zoom)
    E.set_camera_distance(camera, dist)
    E.set_galaxy_magnitude(galaxy, magn)

    E.set_entity_flag(hilite, E.entity_flag_billboard, true)
    E.set_entity_flag(marker, E.entity_flag_billboard, true)
    E.set_entity_scale(hilite, 1 / 256, 1 / 256, 1 / 256)
    E.set_entity_scale(marker, 1 / 128, 1 / 128, 1 / 128)

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

function do_frame()
    vert_point = E.get_star_index(galaxy, camera)

    local x, y, z = E.get_star_position(galaxy, vert_point)

    E.set_entity_position(hilite, x, y, z)
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

    return true
end

function do_click(b, s)
    if b == 1 then
        if E.get_modifier(E.key_modifier_shift) then
            if s then
                local x, y, z = E.get_star_position(galaxy, vert_point)
                print(string.format("v %f %f %f", x, y, z))
            else
                local x, y, z = E.get_star_position(galaxy, vert_point)
                print(string.format("v %f %f %f", x, y, z))

                print(string.format("e %d// %d//", vert_count, vert_count + 1))
                vert_count = vert_count + 2
            end
        else
            setzoom = s
        end
    end

    if b == 2 then setmagn = s end
    if b == 3 then setdist = s end
    return true
end

-------------------------------------------------------------------------------

