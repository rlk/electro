
-------------------------------------------------------------------------------

rot_x = 0
rot_y = 0

camera   = nil
galaxy   = nil

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
    E.set_entity_flags(constellation[serial], E.entity_flag_line_smooth, true)
    E.parent_entity   (constellation[serial], camera)

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
    local brush = E.create_brush()
        
    E.set_brush_frag_prog(brush, "../star.fp")
    E.set_brush_vert_prog(brush, "../star.vp")

    E.set_background(0, 0, 0, 0, 0, 0)

    camera = E.create_camera(E.camera_type_perspective)
    galaxy = E.create_galaxy("../galaxy_hip.gal", brush)

    E.parent_entity(galaxy, camera)

    add_constellation("cassiopeia.obj")
    add_constellation("orion.obj")
    
    E.set_galaxy_magnitude(galaxy, magn)

    E.enable_timer(true)        

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
    
    if s and k == 288 then -- F7
        spin = spin + 1
    end
    if s and k == 289 then -- F8
        spin = spin - 1
    end

    return true
end

function do_frame()
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

do_start()
