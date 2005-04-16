
-------------------------------------------------------------------------------

rot_x = 0
rot_y = 0

camera   = nil
galaxy   = nil
hilite   = nil
marker   = nil
asterism = nil

magn = 100.0
dist =   0.0
spin =   0.0

setrotn = true
setmagn = false
setdist = false
drawing = false

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

function position_camera()
    local T = math.pi * rot_y / 180
    local P = math.pi * rot_x / 180

    local pos_x =  math.cos(P) * math.sin(T) * dist
    local pos_y = -math.sin(P)               * dist
    local pos_z =  math.cos(P) * math.cos(T) * dist

    E.set_entity_position(camera, pos_x, pos_y, pos_z)
    E.set_entity_rotation(camera, rot_x, rot_y, 0)
end

-------------------------------------------------------------------------------

function do_start()
    E.set_background(0, 0, 0, 0, 0, 0)

    camera = E.create_camera(E.camera_type_perspective)
    galaxy = E.create_galaxy("../galaxy_hip.gal")
    hilite = E.create_sprite("hilite.png")
    marker = E.create_sprite("here.png")

    E.set_entity_vert_prog(galaxy, "../star.vp");
    E.set_entity_frag_prog(galaxy, "../star.fp");

    E.parent_entity(galaxy, camera)
    E.parent_entity(hilite, camera)
    E.parent_entity(marker, camera)

    add_constellation("cassiopeia.obj")
    add_constellation("orion.obj")
    
    E.set_galaxy_magnitude(galaxy, magn)

    E.set_entity_flag(hilite, E.entity_flag_billboard, true)
    E.set_entity_flag(marker, E.entity_flag_billboard, true)
    E.set_entity_scale(hilite, 1 / 256, 1 / 256, 1 / 256)
    E.set_entity_scale(marker, 1 / 128, 1 / 128, 1 / 128)

    E.enable_timer(true)        

    io.output("constellation.obj")
    io.write("mtllib constellation.mtl\n")
    io.write("g constellation\n")
    io.write("usemtl blue\n")

    return true
end

function do_timer(dt)
    rot_y = rot_y + dt * spin
    position_camera()
    return true
end

function do_keyboard(k, s)

    if s and k == 13 then
        dist  = 0
        rot_x = 0
        rot_y = 0
        E.set_entity_rotation(camera, 0, 0, 0)
        position_camera()
    end
    
    if s and k == 284 then -- F3
        spin = spin + 1
    end
    if s and k == 285 then -- F4
        spin = spin - 1
    end

    return true
end

function do_frame()
    vert_point = E.get_star_index(galaxy, camera)

    local pnt_x, pnt_y, pnt_z = E.get_star_position(galaxy, vert_point)
    local pos_x, pos_y, pos_z = E.get_entity_position(camera)

    local dx = pnt_x - pos_x
    local dy = pnt_y - pos_y
    local dz = pnt_z - pos_z
    local k  = math.sqrt(dx * dx + dy * dy + dz * dz)

    dx = 20 * dx / k
    dy = 20 * dy / k
    dz = 20 * dz / k

    E.set_entity_position(hilite, pos_x + dx, pos_y + dy, pos_z + dz)
end

function do_point(dx, dy)

    if setmagn then  -- Set the stellar magnitude multiplier.

        magn = magn - dy * 1.0
        if magn < 0 then
            magn = 0
        end
        if magn > 2500 then
            magn = 2500
        end

        E.set_galaxy_magnitude(galaxy, magn)
    end

    if setdist then  -- Set the camera distance from the center.
        dist = dist + dy * 0.1
    end
    
    if setrotn then
        rot_x = rot_x - dy * 0.05
        rot_y = rot_y - dx * 0.05

        if rot_x < -90 then rot_x = -90 end
        if rot_x >  90 then rot_x =  90 end

        if rot_y < -180 then rot_y = rot_y + 360 end
        if rot_y >  180 then rot_y = rot_y - 360 end
    end

    position_camera()

    return true
end

function do_click(b, s)
    if b == 1 then
        if s then
            local x, y, z = E.get_star_position(galaxy, vert_point)
            io.write(string.format("v %f %f %f\n", x, y, z))

            drawing = true
        end

        if not s and drawing then
            local x, y, z = E.get_star_position(galaxy, vert_point)
            io.write(string.format("v %f %f %f\n", x, y, z))

            io.write(string.format("l %d// %d//\n", vert_count, vert_count+1))
            vert_count = vert_count + 2

            if asterism then
                E.delete_entity(asterism)
            end

            io.flush()
            asterism = E.create_object("constellation.obj")

            E.parent_entity(asterism, camera)
            drawing = false
        end
    end

    if b == 2 then
        setmagn = s
        setrotn = not s
    end
    if b == 3 then
        setdist = s
        setrotn = not s
    end
    return true
end

-------------------------------------------------------------------------------

