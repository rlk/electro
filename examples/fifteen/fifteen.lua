
image_file = "moon.png"

camera  = nil
numbers = { }
sprites = { }

curr_i = 4
curr_j = 4
time   = 0

-------------------------------------------------------------------------------

function sprite_position(sprite, i, j)
    local l, r, b, t = E.viewport_get()
    local w = r - l
    local h = t - b

--  E.entity_position(sprite, l + (r - l) * ((i - 1) / 4 + 0.125),
--                            t - (t - b) * ((j - 1) / 4 + 0.125), 0)
    E.entity_position(sprite, l + w * ((i - 1) / 4) + w / 8 + 128,
                              t - h * ((j - 1) / 4) + h / 8 + 128, 0)
end

function solved()
    local k = 1

    for i = 1, 4 do
        for j = 1, 4 do
            if numbers[i][j] == k then
                k = k + 1
            else
                return false
            end
        end
    end
    return true
end

function move_piece(di, dj)
    local next_i = curr_i + di
    local next_j = curr_j + dj

    -- Confirm that the move is valid.

    if 1 <= next_i and next_i <= 4 and
       1 <= next_j and next_j <= 4 then

        -- Swap the moving piece with the blank piece.

        temp                    = numbers[curr_i][curr_j]
        numbers[curr_i][curr_j] = numbers[next_i][next_j]
        numbers[next_i][next_j] = temp

        -- Set the new on-screen locations of the moving sprites.

        sprite_position(sprites[numbers[curr_i][curr_j]], curr_i, curr_j)
        sprite_position(sprites[numbers[next_i][next_j]], next_i, next_j)

        curr_i = next_i
        curr_j = next_j

        -- If the puzzle is solved, display the 16th piece.

        E.entity_flag(sprites[16], E.entity_flag_hidden, not solved())

        return true
    else
        return false
    end
end

-------------------------------------------------------------------------------

function do_start()

    local l, r, b, t = E.viewport_get()

    camera = E.create_camera(E.camera_type_orthogonal)
    E.entity_flag(camera, E.entity_flag_unlit, true);

    -- Initialize an array that maps rows and columns onto puzzle pieces.

    numbers[1] = {  1,  2,  3,  4 }
    numbers[2] = {  5,  6,  7,  8 }
    numbers[3] = {  9, 10, 11, 12 }
    numbers[4] = { 13, 14, 15, 16 }

    -- Add 16 sprites and scale each to 1/16th the size of the display.

    for i = 1, 16 do
        sprites[i] = E.create_sprite(image_file)

        local sw, sh = E.sprite_size(sprites[i])

        E.entity_parent(sprites[i], camera)

        E.entity_scale (sprites[i], 0.25 * (r - l) / sw,
                                    0.25 * (t - b) / sh, 0.25)
    end

    -- Assign 1/16th of the image to each sprite.

    for i = 1, 4 do
        for j = 1, 4 do
            x = j / 4
            y = i / 4

            sprite_position(sprites[numbers[i][j]], i, j)
            E.sprite_bounds(sprites[numbers[i][j]], x - 0.25, x, y - 0.25, y)
        end
    end

    -- Creating the hole in the puzzle by hiding the 16th piece.

    E.entity_flag(sprites[16], E.entity_flag_hidden, true)

    return true
end

function do_keyboard(k, s)
    if s then
        -- If an arrow key has been pressed, move a puzzle piece.

        if k == 276 then move_piece( 1,  0) end  -- Left.
        if k == 275 then move_piece(-1,  0) end  -- Right.
        if k == 273 then move_piece( 0,  1) end  -- Up.
        if k == 274 then move_piece( 0, -1) end  -- Down.

        -- If autoplay has been switched, toggle the idle function.

        if k == 282 then E.enable_idle(true)  end
        if k == 283 then E.enable_idle(false) end
    end
    return true
end

function do_timer(dt)
    time = time + dt

    while time > 0.25 do
        if math.random(0, 1) == 0 then
            if move_piece(math.random(-1, 1), 0) then
                time = 0
                return true
            end
        else
            if move_piece(0, math.random(-1, 1)) then
                time = 0
                return true
            end
        end
    end

    return false
end

-------------------------------------------------------------------------------

