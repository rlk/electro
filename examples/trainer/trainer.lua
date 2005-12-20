
track_host = "10.0.0.51"
--track_host = "192.168.0.1"
track_port = 10000
movie_port = 2827

modifier  = false
recording = false
waiting   = false

scale     = 1
ellipse_w = 50 * 2
ellipse_h = 64 * 2
ellipse_d = 0

-------------------------------------------------------------------------------

function dump(sock, s, h, p)
    for i = 1, string.len(s) do
        E.print_console(string.format("%3d ", string.byte(s, i)))
    end
    E.print_console("\n")

    sock:sendto(s, h, p)
end

-------------------------------------------------------------------------------

IDM_VIDEO_PLAY        = 32775
IDM_VIDEO_STOP        = 32777
IDM_RECORD_FACE       = 32809
IDM_SEND_ELECTRO      = 32872
IDM_STOP_ELECTRO      = 32873
IDM_RECORD_TRAIN_FACE = 32874

function htonl(n)
    local D = math.floor(math.mod(n, 2^8))
    local C = math.floor(math.mod(n, 2^16) / 2^8)
    local B = math.floor(math.mod(n, 2^24) / 2^16)
    local A = math.floor(         n        / 2^24)

    return string.char(A, B, C, D)
end

function htons(n)
    local B = math.floor(math.mod(n, 2^8))
    local A = math.floor(math.mod(n, 2^16) / 2^8)

    return string.char(A, B)
end

function send_video_play()
    local sock = socket.udp()

    if sock then
        sock:sendto(htonl(87)..
                    htonl(3)..
                    htonl(IDM_VIDEO_STOP)..
                    htonl(0)..
                    htonl(0)..
                    htonl(IDM_VIDEO_PLAY)..
                    htonl(0)..
                    htonl(0)..
                    htonl(IDM_SEND_ELECTRO)..
                    htonl(0)..
                    htonl(0), track_host, track_port)
    end
end

function send_video_stop()
    local sock = socket.udp()

    if sock then
        sock:sendto(htonl(87)..
                    htonl(1)..
                    htonl(IDM_RECORD_FACE)..
                    htonl(0)..
                    htonl(0), track_host, track_port)
    end
end

function send_train_start(x, y, w, h)
    local sock = socket.udp()

    if sock then
        sock:sendto(htonl(87)..
                    htonl(1)..
                    htonl(IDM_RECORD_TRAIN_FACE)..
                    htonl(0)..
                    string.char(x + 128)..
                    string.char(y + 128)..
                    string.char(w)..
                    string.char(h), track_host, track_port)
    end
end

-------------------------------------------------------------------------------

function create_dot(X, Y, Z, W, H)
    local image  = E.create_image("dot.png")
    local brush  = E.create_brush()
    local sprite = E.create_sprite(brush)

    E.set_brush_image(brush, image)
    E.set_brush_flags(brush, E.brush_flag_unlit, true)

    E.set_entity_alpha(sprite, 0.5)
    E.set_entity_scale(sprite, 0.5, 0.5, 0.5)
    E.set_entity_position(sprite, 0, 0, 0)

    return sprite
end

function create_movie(X, Y, Z, W, H)
    local brush = E.create_brush()
    local image = E.create_image(movie_port)
    local movie = E.create_sprite(brush)

    E.set_brush_image(brush, image)
    E.set_brush_flags(brush, E.brush_flag_unlit, true)

    E.set_entity_position(movie, X, Y, Z)

    return movie
end

function mid_text(text, line, fill)
    local string = E.create_string(text)
    local pivot  = E.create_pivot()

    local x0, y0, z0, x1, y1, z1 = E.get_entity_bound(string)

    E.set_string_line(string, line)
    E.set_string_fill(string, fill)

    E.parent_entity(string, pivot)

    E.set_entity_position(string, -(x1 - x0) / 2, -0.5, 0)

    return pivot
end

function create_text_1(X, Y, Z, W, H)
    local t = H / 12
    local s = H / 20
    local S = H / 16
    local z = 0.01

    local pivot = E.create_pivot()

    local line_1 = "Let's begin the training..."
    local line_2 = "Center your face on the screen."
    local line_3 = "Adjust the ellipse using the joystick."
    local line_4 = "Follow the ellipse with your head, not your eyes."
    local line_5 = "Press the left button to proceed."

    local text_1 = mid_text(line_1, text_line, text_fill)
    local text_2 = mid_text(line_2, text_line, text_fill)
    local text_3 = mid_text(line_3, text_line, text_fill)
    local text_4 = mid_text(line_4, text_line, text_fill)
    local text_5 = mid_text(line_5, text_line, text_fill)

    E.parent_entity(text_1, pivot)
    E.parent_entity(text_2, pivot)
    E.parent_entity(text_3, pivot)
    E.parent_entity(text_4, pivot)
    E.parent_entity(text_5, pivot)

    E.set_entity_position(text_1, X, Y + 6 * S, Z + z)
    E.set_entity_position(text_2, X, Y - 4 * S, Z + z)
    E.set_entity_position(text_3, X, Y - 5 * S, Z + z)
    E.set_entity_position(text_4, X, Y - 6 * S, Z + z)
    E.set_entity_position(text_5, X, Y - 7 * S, Z + z)

    E.set_entity_scale(text_1, t, t, t)
    E.set_entity_scale(text_2, s, s, s)
    E.set_entity_scale(text_3, s, s, s)
    E.set_entity_scale(text_4, s, s, s)
    E.set_entity_scale(text_5, s, s, s)

    return pivot
end

function create_text_2(X, Y, Z, W, H)
    local t = H / 12
    local s = H / 20
    local S = H / 16
    local z = 0.1

    local pivot = E.create_pivot()

    local line_1 = "Training..."
    local line_3 = "Follow the ellipse with your head, not your eyes."

    local text_1 = mid_text(line_1, text_line, text_fill)
    local text_3 = mid_text(line_3, text_line, text_fill)

    E.parent_entity(text_1, pivot)
    E.parent_entity(text_3, pivot)

    E.set_entity_position(text_1, X, Y + 6 * S, Z + z)
    E.set_entity_position(text_3, X, Y - 5 * S, Z + z)

    E.set_entity_scale(text_1, t, t, t)
    E.set_entity_scale(text_3, s, s, s)

    return pivot
end

function create_text_3(X, Y, Z, W, H)
    local t = H / 12
    local s = H / 18
    local S = H / 16
    local z = 0.1

    local pivot = E.create_pivot()

    local line_1 = "Thank you!  Please wait..."
    local line_3 = "while the neural network learns."

    local text_1 = mid_text(line_1, text_line, text_fill)
    local text_3 = mid_text(line_3, text_line, text_fill)

    E.parent_entity(text_1, pivot)
    E.parent_entity(text_3, pivot)

    E.set_entity_position(text_1, X, Y + 6 * S, Z + z)
    E.set_entity_position(text_3, X, Y - 5 * S, Z + z)

    E.set_entity_scale(text_1, t, t, t)
    E.set_entity_scale(text_3, s, s, s)

    return pivot
end

-------------------------------------------------------------------------------

time = 0

function cerp(a, b, t)
    return a + (b - a) * (3 * t * t - 2 * t * t * t)
end

function berp(a, b, s)
    local t = s / 2
    return a + (b - a) * (3 * t * t - 2 * t * t * t)
end

function spot_dot(t)
    local X0, Y0, Z0, X1, Y1, Z1 = E.get_display_bound()
    local XM = (X0 + X1) / 2
    local YM = (Y0 + Y1) / 2
    local ZM = (Z0 + Z1) / 2
    local R0 = -30
    local R1 =  30
    local RM =   0

    if      1 < t and t <=  2 then return XM, cerp(YM, Y1, t - 1), ZM, RM
    elseif  2 < t and t <=  4 then return XM, berp(Y1, Y0, t - 2), ZM, RM
    elseif  4 < t and t <=  5 then return XM, cerp(Y0, YM, t - 4), ZM, RM

    elseif  5 < t and t <=  6 then return cerp(XM, X1, t - 5), YM, ZM, RM
    elseif  6 < t and t <=  8 then return berp(X1, X0, t - 6), YM, ZM, RM
    elseif  8 < t and t <=  9 then return cerp(X0, XM, t - 8), YM, ZM, RM

    elseif  9 < t and t <= 10 then return XM, YM, ZM, cerp(RM, R1, t -  9)
    elseif 10 < t and t <= 12 then return XM, YM, ZM, berp(R1, R0, t - 10)
    elseif 12 < t and t <= 13 then return XM, YM, ZM, cerp(R0, RM, t - 12)

    elseif 13 < t then start_waiting()
    end

    return XM, YM, ZM, RM
end

function init_dot()
    time = 0

    local x, y, z, r = spot_dot(time * 0.75)

    E.set_entity_position(dot, x, y + ellipse_d * scale, z + 0.02)
    E.set_entity_rotation(dot, 0, 0, r)
end

function move_dot(dt)
    time = time + dt

    local x, y, z, r = spot_dot(time * 0.75)

    E.set_entity_position(dot, x, y + ellipse_d * scale, z + 0.02)
    E.set_entity_rotation(dot, 0, 0, r)
end

function bound_dot()
    if  ellipse_w <    0 then ellipse_w =    0 end
    if  ellipse_w >  255 then ellipse_w =  255 end
    if  ellipse_h <    0 then ellipse_h =    0 end
    if  ellipse_h >  255 then ellipse_h =  255 end
    if  ellipse_d < -128 then ellipse_d = -128 end
    if  ellipse_d >  127 then ellipse_d =  127 end
end

-------------------------------------------------------------------------------

function start_position()
    send_video_play()

    E.set_entity_flags(text_1, E.entity_flag_hidden, false)
    E.set_entity_flags(text_2, E.entity_flag_hidden, true)
    E.set_entity_flags(text_3, E.entity_flag_hidden, true)
    E.set_entity_flags(dot,    E.entity_flag_hidden, false)

    init_dot()

    recording = false
    waiting   = false
end

function start_training()
    send_train_start(0, ellipse_d, ellipse_w, ellipse_h)

    E.set_entity_flags(text_1, E.entity_flag_hidden, true)
    E.set_entity_flags(text_2, E.entity_flag_hidden, false)
    E.set_entity_flags(text_3, E.entity_flag_hidden, true)
    E.set_entity_flags(dot,    E.entity_flag_hidden, false)

    recording = true
end

function start_waiting()

    E.set_entity_flags(text_1, E.entity_flag_hidden, true)
    E.set_entity_flags(text_2, E.entity_flag_hidden, true)
    E.set_entity_flags(text_3, E.entity_flag_hidden, false)
    E.set_entity_flags(dot,    E.entity_flag_hidden, true)

    recording = false
    waiting   = true
end

-------------------------------------------------------------------------------

function do_exit()
--  send_video_stop()
    E.nuke()
    E.chdir("..")
    dofile("demo.lua")
end

function do_joystick(d, b, s)
    if b == 1 and s then
        if not recording and not waiting then
            start_training()
        end
    end
    if b == 2 then
        modifier = s
    end
end

function do_keyboard(k, s)
    if varrier_keyboard then
        if varrier_keyboard(k, s, camera) then
            return true
        end
    end

    if s then
        if k == E.key_F12 then
            do_exit()
        end

        if k == E.key_space then
            if not recording and not waiting then
                start_training()
            end
        end
        if k == E.key_left then
            ellipse_w = ellipse_w - 2
            bound_dot()
        end
        if k == E.key_right then
            ellipse_w = ellipse_w + 2
            bound_dot()
        end
        if k == E.key_down then
            ellipse_h = ellipse_h - 2
            bound_dot()
        end
        if k == E.key_up then
            ellipse_h = ellipse_h + 2
            bound_dot()
        end
        if k == E.key_pageup then
            bound_dot()
            ellipse_d = ellipse_d + 2
        end
        if k == E.key_pagedown then
            ellipse_d = ellipse_d - 2
            bound_dot()
        end

        return true
    end

    return false
end

function do_timer(dt)
    local X0, Y0, Z0, X1, Y1, Z1 = E.get_display_bound()
    local x0, y0, z0, x1, y1, z1 = E.get_entity_bound(movie)
    local jx, jy = E.get_joystick(0)

    local W = (X1 - X0)
    local H = (Y1 - Y0)
    local w = (x1 - x0)
    local h = (y1 - y0)

    if modifier then
        if jy < -0.5 or 0.5 < jy then
            ellipse_d = ellipse_d - jy * dt * 25
            bound_dot()
        end
    else
        if jx < -0.5 or 0.5 < jx then
            ellipse_w = ellipse_w + jx * dt * 25
            bound_dot()
        end
        if jy < -0.5 or 0.5 < jy then
            ellipse_h = ellipse_h - jy * dt * 25
            bound_dot()
        end
    end

    if w > 0 and h > 0 then
        scale = math.min(W / w, H / h)

        E.set_sprite_range(movie, 0, w, 0, h)
        E.set_entity_scale(movie, scale, scale, scale)

        E.set_entity_scale(dot,   ellipse_w * scale / 256,
                                  ellipse_h * scale / 256, scale)
        move_dot(0)
    end

    if recording then
        move_dot(dt)
    end
    if waiting and w == 0 and h == 0 then
        do_exit()
    end

    return true
end

function do_start()
    local X0, Y0, Z0, X1, Y1, Z1 = E.get_display_bound()

    local X = (X0 + X1) / 2
    local Y = (Y0 + Y1) / 2
    local Z = (Z0 + Z1) / 2
    local W =  X1 - X0
    local H =  Y1 - Y0

    camera = E.create_camera(E.camera_type_perspective)

    E.set_camera_offset(camera, 0, 4, 0)

    -- Initialize text rendering.

    E.set_typeface("../data/VeraBd.ttf", 0.0001, 0.05)

    text_line = E.create_brush()
    text_fill = E.create_brush()

    E.set_brush_flags(text_line, E.brush_flag_unlit, true)
    E.set_brush_flags(text_fill, E.brush_flag_unlit, true)

    E.set_brush_color(text_line, 0.0, 0.0, 0.0, 0.3)
    E.set_brush_color(text_fill, 1.0, 1.0, 0.0, 0.8)

    -- Initialize the scene graph.

    movie  = create_movie (X, Y, Z, W, H)
    text_1 = create_text_1(X, Y, Z, W, H)
    text_2 = create_text_2(X, Y, Z, W, H)
    text_3 = create_text_3(X, Y, Z, W, H)
    dot    = create_dot   (X, Y, Z, W, H)

    E.set_entity_flags(text_1, E.entity_flag_hidden, true)
    E.set_entity_flags(text_2, E.entity_flag_hidden, true)
    E.set_entity_flags(text_3, E.entity_flag_hidden, true)
    E.set_entity_flags(dot,    E.entity_flag_hidden, true)

    E.parent_entity(dot,    camera)
    E.parent_entity(text_3, camera)
    E.parent_entity(text_2, camera)
    E.parent_entity(text_1, camera)
    E.parent_entity(movie,  camera)

    -- Go!

    E.set_background(0, 0, 0)
    E.enable_timer(true)

    start_position()
end

-------------------------------------------------------------------------------

do_start()
