-------------------------------------------------------------------------------
-- Mars planetary terrain demo
-- Robert Kooima
-- Sept 2006, Feb 2007

-------------------------------------------------------------------------------
-- Data configuration

-- DATA_S = "c:\\rlk\\megr016.raw"
-- DATA_W = 5760
-- DATA_H = 5760 / 2

-- 64-degree-per-pixel data
--DATA_S = "c:\\electro-840\\examples\\mars\\megr064.raw"
--DATA_S = "/home/evl/rlk/src/megdr/megr064.raw"
--DATA_W = 23040
--DATA_H = 11520

DATA_S = "c:\\rlk\\data\\megr064.raw"
DATA_W = 23040 / 4
DATA_H = 11520 / 4

-------------------------------------------------------------------------------
-- Interaction mode configuration

--     mouse  Mouse / keyboard
--     joy2   Simplified 2-axis / 1-button joystick
--     joy4   Full 4-axis joystick
--     wand   Full 6 degree-of-freedom tracker

-- MODE = "mouse"
-- MODE = "joy2"
-- MODE = "joy4"
MODE = "wand"

-------------------------------------------------------------------------------
-- Default configuration

ROT_X  = 25.2     -- Initial X rotation (degrees) (the axial tilt of Mars)
ROT_Y  = 90       -- Initial Y rotation (degrees) (Mariner Valley up front)
POS_Z  = 10000000 -- Initial Z distance (meters)
MAG    = 200      -- Star magnitude     (pixels)
LPF    = 16       -- Input low-pass-filter coefficient

MIN_ALT = 3450000
MAX_ALT = 4450000

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

    return 2000000 * (d - 3300000) / 3300000
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
    local dp = speed() * dt

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

mode.joy2.p0 =  0
mode.joy2.r0 =  0
mode.joy2.p1 =  0
mode.joy2.r1 =  0
mode.joy2.v0 =  0
mode.joy2.v1 =  0
mode.joy2.tx = -1
mode.joy2.ty =  0
mode.joy2.tz =  0

-- Store the planet tangent vector

function mode.joy2.reset()
    rot_x = ROT_X
    rot_y = ROT_Y

    mode.joy2.p0 =  0
    mode.joy2.r0 =  0
    mode.joy2.p1 =  0
    mode.joy2.r1 =  0
    mode.joy2.v0 =  0
    mode.joy2.v1 =  0
    mode.joy2.tx = -1
    mode.joy2.ty =  0
    mode.joy2.tz =  0

    E.set_entity_rotation(pivot, ROT_X, ROT_Y, 0.0)
    E.set_entity_position(camera,  0,  0, (MIN_ALT + MAX_ALT) / 2)
    E.set_entity_rotation(camera, 90,  0, -90)

    return true
end

function mode.joy2.joystick(d, b, s)
    if b == 0 then
        if s then
            mode.joy2.v1 = 1
        else
            mode.joy2.v1 = 0
        end
    end
    if b == 1 then
        mode.joy2.reset()
    end
end

function mode.joy2.tangent()
    -- Compute the planet tanget given the current position and orientation.

    local nx, ny, nz = E.get_entity_position(camera)
    local zx, zy, zz = E.get_entity_z_vector(camera)
    local d = math.sqrt(nx * nx + ny * ny + nz * nz)

    nx = nx / d
    ny = ny / d
    nz = nz / d

    local bx = ny * zz - nz * zy
    local by = nz * zx - nx * zz
    local bz = nx * zy - ny * zx

    local tx = by * nz - bz * ny
    local ty = bz * nx - bx * nz
    local tz = bx * ny - by * nx

    return tx, ty, tz
end

function mode.joy2.timer(dt)

    -- Compute the planet normal and current altitude.

    local nx, ny, nz = E.get_entity_position(camera)
    local d = math.sqrt(nx * nx + ny * ny + nz * nz)

    nx = nx / d
    ny = ny / d
    nz = nz / d

    -- Apply the joystick input to the target pitch and roll.

    local jx, jy = E.get_joystick(0, 0, 1)

    mode.joy2.p1 = mode.joy2.p1 + 30 * jy * dt
    mode.joy2.r1 = mode.joy2.r1 - 30 * jx * dt

    -- Clamp the target pitch and roll.

    if mode.joy2.p1 >  45 then mode.joy2.p1 =  45 end
    if mode.joy2.p1 < -45 then mode.joy2.p1 = -45 end
    if mode.joy2.r1 >  45 then mode.joy2.r1 =  45 end
    if mode.joy2.r1 < -45 then mode.joy2.r1 = -45 end

    -- Avoid the minimum and maximum altitudes.

    if d < MIN_ALT and mode.joy2.p1 < 0 then mode.joy2.p1 = 0 end
    if d > MAX_ALT and mode.joy2.p1 > 0 then mode.joy2.p1 = 0 end

    -- If the input is centered then level out slowly.

    if -0.1 < jx and jx < 0.1 then
       mode.joy2.r1 = mode.joy2.r1 * (1 - 0.50 * dt)
   end
   if  -0.1 < jy and jy < 0.1 then
       mode.joy2.p1 = mode.joy2.p1 * (1 - 0.25 * dt)
   end

    -- Apply the low-pass-filtered inputs to the current orientation.

    mode.joy2.p0 = (mode.joy2.p1 + mode.joy2.p0 * (LPF - 1)) / LPF
    mode.joy2.r0 = (mode.joy2.r1 + mode.joy2.r0 * (LPF - 1)) / LPF
    mode.joy2.v0 = (mode.joy2.v1 + mode.joy2.v0 * (LPF - 1)) / LPF

    -- Temporarily orient the camera parallel to the surface of the planet.

    local tx = mode.joy2.tx
    local ty = mode.joy2.ty
    local tz = mode.joy2.tz

    local bx = ny * tz - nz * ty
    local by = nz * tx - nx * tz
    local bz = nx * ty - ny * tx

    E.set_entity_basis(camera, bx, by, bz, nx, ny, nz, tx, ty, tz)

    -- Apply yaw with rate relative to current roll.

    E.turn_entity(camera, 0, mode.joy2.r0 * dt, 0)

    -- Update the tangent with the new heading.

    mode.joy2.tx, mode.joy2.ty, mode.joy2.tz = mode.joy2.tangent()

    -- Set the camera orientation.

    E.turn_entity(camera, 0, 0, mode.joy2.r0)
    E.turn_entity(camera, mode.joy2.p0, 0, 0)

    -- Move the camera

    E.move_entity(camera, 0, 0, -mode.joy2.v0 * dt * speed())
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

    local k = 0.0
    local n = 32

    local dr =      90 * dt
    local dp = speed() * dt

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
        
        local dp = dt * speed()
        local dr = dt * 50.0

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
