-------------------------------------------------------------------------------
-- Mars planetary terrain demo
-- Robert Kooima
-- Sept 2006, Feb 2007

-------------------------------------------------------------------------------
-- Data configuration

DATA_S = "/home/evl/rlk/data/megr032.raw"
DATA_W = 11520
DATA_H = 5760

-- 64-degree-per-pixel data
-- DATA_S = "/data2/rlk/megr064.raw"
-- DATA_W = 23040
-- DATA_H = 11520

-------------------------------------------------------------------------------
-- Interaction mode configuration

--     mouse  Mouse / keyboard
--     joy2   Simplified 2-axis / 1-button joystick
--     joy4   Full 4-axis joystick
--     wand   Full 6 degree-of-freedom tracker

-- MODE = "mouse"
MODE = "joy2"
-- MODE = "joy4"
-- MODE = "wand"

-------------------------------------------------------------------------------
-- Default configuration

ROT_X  = 25.2     -- Initial X rotation (degrees)
ROT_Y  = 180      -- Initial Y rotation (degrees)
POS_Z  = 10000000 -- Initial Z distance (meters)
MAG    = 200      -- Star magnitude     (pixels)


--=============================================================================
-- Global state

mode = { }
curr = nil

mode.mouse = { }
mode.joy2  = { }
mode.joy4  = { }
mode.wand  = { }

-------------------------------------------------------------------------------

function speed()
    -- Compute the correct speed for the current altitude.

    local cx, cy, cz = E.get_entity_position(camera)
    local d = math.sqrt(cx * cx + cy * cy + cz * cz)

    return (d - 3300000) / 3300000
end

function range()
    -- Set the far clipping plane to always include the planet.

    local cx, cy, cz = E.get_entity_position(camera)
    local d = math.sqrt(cx * cx + cy * cy + cz * cz)

    E.set_camera_range(camera, 1000, d + 1000000)

    return true
end

-------------------------------------------------------------------------------
-- Mouse / keyboard flight model

mode.mouse.turn_camera = false
mode.mouse.turn_planet = false

mode.mouse.dx = 0
mode.mouse.dy = 0
mode.mouse.dz = 0

function mode.mouse.reset()
    rot_x = ROT_X
    rot_y = ROT_Y

    E.set_entity_rotation(pivot,  ROT_X, ROT_Y,   0.0)
    E.set_entity_rotation(camera,   0.0,   0.0,   0.0)
    E.set_entity_position(camera,   0.0,   0.0, POS_Z)

    return true
end

function mode.mouse.timer(dt)
    local dp = 2500000 * speed() * dt

    E.move_entity(camera, mode.mouse.dx * dp,
                          mode.mouse.dy * dp,
                          mode.mouse.dz * dp)
end

function mode.mouse.click(b, s)
    if b == 1 then mode.mouse.turn_camera = s      end
    if b == 3 then mode.mouse.turn_planet = s      end
    if b == 4 then E.turn_entity(camera, 0, 0, -5) end
    if b == 5 then E.turn_entity(camera, 0, 0,  5) end
end

function mode.mouse.point(dx, dy)
    if mode.mouse.turn_planet then
        rot_y = rot_y + dx / 10
        E.set_entity_rotation(pivot, rot_x, rot_y, 0)
        return true
    end

    if mode.mouse.turn_camera then
        E.turn_entity(camera, -dy / 10, -dx / 10, 0)
        return true
    end

    return false
end

function mode.mouse.keyboard(k, s)
    if s then
        if k == E.key_return then
            return mode.mouse.reset()
        end

        if k == E.key_up       then mode.mouse.dz = mode.mouse.dz - 1 end
        if k == E.key_down     then mode.mouse.dz = mode.mouse.dz + 1 end
        if k == E.key_pagedown then mode.mouse.dy = mode.mouse.dy - 1 end
        if k == E.key_pageup   then mode.mouse.dy = mode.mouse.dy + 1 end
        if k == E.key_left     then mode.mouse.dx = mode.mouse.dx - 1 end
        if k == E.key_right    then mode.mouse.dx = mode.mouse.dx + 1 end
    else
        if k == E.key_up       then mode.mouse.dz = mode.mouse.dz + 1 end
        if k == E.key_down     then mode.mouse.dz = mode.mouse.dz - 1 end
        if k == E.key_pagedown then mode.mouse.dy = mode.mouse.dy + 1 end
        if k == E.key_pageup   then mode.mouse.dy = mode.mouse.dy - 1 end
        if k == E.key_left     then mode.mouse.dx = mode.mouse.dx + 1 end
        if k == E.key_right    then mode.mouse.dx = mode.mouse.dx - 1 end
    end

    return true
end
 
-------------------------------------------------------------------------------
-- 2-axis joystick flight model

mode.joy2.pitch = 0
mode.joy2.roll  = 0
mode.joy2.yaw   = 0
mode.joy2.gas   = 0

function mode.joy2.reset()
    rot_x = ROT_X
    rot_y = ROT_Y

    mode.joy2.pitch = 0
    mode.joy2.roll  = 0
    mode.joy2.yaw   = 0

    E.set_entity_rotation(pivot,  ROT_X, ROT_Y,   0.0)
    E.set_entity_rotation(camera,   0.0,   0.0,   0.0)
    E.set_entity_position(camera,   0.0,   0.0, POS_Z)

    return true
end

function mode.joy2.joystick(d, b, s)
    if b == 0 then
        if s then
            mode.joy2.gas = 1
        else
            mode.joy2.gas = 0
        end
    end
end

function mode.joy2.timer(dt)
    local joy_x, joy_y = E.get_joystick(0, 0, 1)


    E.set_entity_rotation(camera, mode.joy2.pitch,
                                  mode.joy2.yaw,
                                  mode.joy2.roll)
end

-------------------------------------------------------------------------------
-- 4-axis joystick flight model

mode.joy4.jx = 0
mode.joy4.jy = 0
mode.joy4.jz = 0
mode.joy4.jw = 0

function mode.joy4.reset()
    rot_x = ROT_X
    rot_y = ROT_Y

    E.set_entity_rotation(pivot,  ROT_X, ROT_Y,   0.0)
    E.set_entity_rotation(camera,   0.0,   0.0,   0.0)
    E.set_entity_position(camera,   0.0,   0.0, POS_Z)

    return true
end

function mode.joy4.joystick(d, b, s)
    if b == 0 then
        if s then
            mode.joy4.jx = 0
            mode.joy4.jy = 0
            mode.joy4.jz = 0
            mode.joy4.jw = 0
            mode.joy4.reset()
        end
    end
end

function mode.joy4.timer(dt)
    local joy_x, joy_y = E.get_joystick(0, 3, 4)
    local joy_z, joy_w = E.get_joystick(0, 0, 1)

    local s = 100 * speed()
    local k = 0.0
    local n = 32

    local dr =      90 * dt
    local dp = 1000000 * dt * speed()

    mode.joy4.jx = ((n - 1) * mode.joy4.jx + joy_x) / n
    mode.joy4.jy = ((n - 1) * mode.joy4.jy + joy_y) / n
    mode.joy4.jz = ((n - 1) * mode.joy4.jz + joy_z) / n
    mode.joy4.jw = ((n - 1) * mode.joy4.jw + joy_w) / n

    local rx =  mode.joy4.jw * dr
    local ry = -mode.joy4.jx * dr
    local rz = -mode.joy4.jz * dr
    local dz =  mode.joy4.jy * dp

    E.turn_entity(camera, rx, ry, rz)
    E.move_entity(camera,  0,  0, dz)

    rot_y = rot_y + E.get_joystick(0, 5, 0) * dt * 30
    E.set_entity_rotation(pivot, rot_x, rot_y, 0);
end

-------------------------------------------------------------------------------
-- 6 degree-of-freedom flight model

mode.wand.fly = false

function mode.wand.reset()
    rot_x = ROT_X
    rot_y = ROT_Y

    E.set_entity_rotation(pivot,  ROT_X, ROT_Y,   0.0)
    E.set_entity_rotation(camera,   0.0,   0.0,   0.0)
    E.set_entity_position(camera,   0.0,   0.0, POS_Z)

    return true
end

function mode.wand.begin()
    mode.wand.px, mode.wand.py, mode.wand.pz = E.get_entity_position(wand)
    mode.wand.xx, mode.wand.xy, mode.wand.xz = E.get_entity_x_vector(wand)
    mode.wand.yx, mode.wand.yy, mode.wand.yz = E.get_entity_y_vector(wand)
    mode.wand.zx, mode.wand.zy, mode.wand.zz = E.get_entity_z_vector(wand)
end

function mode.wand.joystick(d, b, s)
    if b == 1 then
        if s then
            return mode.wand.reset()
        end
    end
    if b == 3 then
        if s then
            mode.wand.begin()
            mode.wand.fly = true
        else
            mode.wand.fly = false
        end
    end
end

function mode.wand.timer(dt)
    if mode.wand.fly then
        local px, py, pz = E.get_entity_position(wand)
        local xx, xy, xz = E.get_entity_x_vector(wand)
        local yx, yy, yz = E.get_entity_y_vector(wand)
        local zx, zy, zz = E.get_entity_z_vector(wand)
        
        local dp = dt * 1500000.0 * speed()
        local dr = dt *      50.0

        local dx = (px - mode.wand.px) * dp;
        local dy = (py - mode.wand.py) * dp;
        local dz = (pz - mode.wand.pz) * dp;

        local vx = mode.wand.zx - zx
        local vy = mode.wand.zy - zy
        local vz = mode.wand.zz - zz

        local wx = mode.wand.yx - yx
        local wy = mode.wand.yy - yy
        local wz = mode.wand.yz - yz

        dX =  (vx * mode.wand.yx + vy * mode.wand.yy + vz * mode.wand.yz) * dr
        dY = -(vx * mode.wand.xx + vy * mode.wand.xy + vz * mode.wand.xz) * dr
        dZ =  (wx * mode.wand.xx + wy * mode.wand.xy + wz * mode.wand.xz) * dr

        E.move_entity(camera, dx, dy, dz)
        E.turn_entity(camera, dX, dY, dZ)
    end

    rot_y = rot_y + E.get_joystick(0, 0, 0) * dt * 30
    E.set_entity_rotation(pivot, rot_x, rot_y, 0);
end

-------------------------------------------------------------------------------

function do_start()

    -- Compute the center of the display volume.

    local X0, Y0, Z0, X1, Y1, Z1 = E.get_display_bound()

    local XC = (X0 + X1) / 2
    local YC = (Y0 + Y1) / 2
    local ZC = (Z0 + Z1) / 2

    -- Initialize the star rendering vertex and fragment programs.

    local star_brush = E.create_brush()
    E.set_brush_frag_prog(star_brush, "star.fp")
    E.set_brush_vert_prog(star_brush, "star.vp")

    -- Create the scene graph

    camera = E.create_camera(E.camera_type_perspective)
    light  = E.create_light(E.light_type_directional)
    back   = E.create_light(E.light_type_directional)
    wand   = E.create_pivot()
    pivot  = E.create_pivot()
    galaxy = E.create_galaxy("../data/galaxy_hip.gal", star_brush)
    planet = E.create_terrain(DATA_S, DATA_W, DATA_H, 45)

    E.parent_entity(light,  camera)
    E.parent_entity(galaxy, camera)
    E.parent_entity(back,   light)
    E.parent_entity(pivot,  back)
    E.parent_entity(planet, pivot)
    
    E.set_entity_position(light,  1.0,  0.0,  1.0)
    E.set_entity_position(back,  -1.0,  0.0, -1.0)
    E.set_entity_position(pivot,  XC, YC, ZC)

    E.set_light_color(back, 0.2, 0.3, 0.5)

    E.set_galaxy_magnitude(galaxy, MAG)
    E.set_entity_scale    (galaxy, 1000, 1000, 1000)
    E.set_entity_flags    (galaxy, E.entity_flag_ballboard, true)

    -- Set up tracking

    E.set_entity_tracking(wand, 1, E.tracking_mode_local)
    E.set_entity_flags   (wand, E.entity_flag_track_pos, true)
    E.set_entity_flags   (wand, E.entity_flag_track_rot, true)

    -- Set up the initial mode.

    if MODE == "mouse" then curr = mode.mouse end
    if MODE == "joy2"  then curr = mode.joy2  end
    if MODE == "joy4"  then curr = mode.joy4  end
    if MODE == "wand"  then curr = mode.wand  end

    -- Go!

    E.set_background(0, 0, 0)
    E.enable_timer(true)

    if curr and curr.reset then
        return  curr.reset()
    end
end

function do_joystick(d, b, s)
    if curr and curr.joystick then
        return  curr.joystick(d, b, s)
    else
        return  false
    end
end

function do_timer(dt)
    if curr and curr.timer then
        return  curr.timer(dt)
    else
        return  false
    end
end

function do_frame()
    range()
end

function do_click(b, s)
    if curr and curr.click then
        return  curr.click(b, s)
    else
        return  false
    end
end

function do_point(dx, dy)
    if curr and curr.point then
        return  curr.point(dx, dy)
    else
        return  false
    end
end

function do_keyboard(k, s)
    if varrier_keyboard then
        if varrier_keyboard(k, s, camera) then
            return true
        end
    end

    if curr and curr.keyboard then
        return  curr.keyboard(k, s)
    else
        return  false
    end
end

-------------------------------------------------------------------------------

do_start()
