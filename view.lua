
-------------------------------------------------------------------------------

pos_x  =    0.0
pos_y  =   15.5
pos_z  = 9200.0
rot_x  =    0.0
rot_y  =    0.0
rot_dy =    0.0
dist   =    1.5
magn   =  128.0

btn    = { }
foo    = -1

foo_a  =  0
foo_x  =   global_x + 96
foo_y  =  -global_y + global_h - 96

-------------------------------------------------------------------------------

function do_keybd(k, s)
   if s then
      if k == 282 then -- F1
         magn = magn - 32.0
         camera_magn(magn)
         return true
      end
      if k == 283 then -- F2
         magn = magn + 32.0
         camera_magn(magn)
         return true
      end
      if k == 284 then -- F3
         rot_dy = rot_dy - 1
         enable_idle(rot_dy ~= 0)
         return true
      end
      if k == 285 then -- F4
         rot_dy = rot_dy + 1
         enable_idle(rot_dy ~= 0)
         return true
      end
   end

   return true
end

function do_click(b, s)

   if s then
      if b == 4 then -- mouse wheel down
         dist = dist * 1.5;
         camera_dist(dist)
         return true
      end

      if b == 5 then -- mouse wheel up
         dist = dist / 1.5;
         camera_dist(dist)
         return true
      end
      
      if b == 2 then -- middle button click
         dist  = 1.5
         camera_dist(dist)

         if foo ~= -1 then
            foo_x =   global_x + 96
            foo_y =  -global_y + global_h - 96
            sprite_move(foo, foo_x, foo_y)
         end

         return true
      end

      if b == 3 then
         if foo == -1 then
            foo = sprite_load("head.png")
            sprite_size(foo, 0.5)
            sprite_move(foo, foo_x, foo_y)
         else
            sprite_free(foo)
            foo = -1
         end
         return true
      end
   end

   btn[b] = s

   return false
end

function do_point(dx, dy)

   if btn[1] and foo ~= -1 then
      foo_x = foo_x + dx
      foo_y = foo_y + dy
      sprite_move(foo, foo_x, foo_y)
   else
      rot_x = rot_x - 180 * dy / 500.0
      rot_y = rot_y - 180 * dx / 500.0

      if rot_x >   90 then rot_x =  90 end
      if rot_x <  -90 then rot_x = -90 end
      if rot_y >  180 then rot_y = rot_y - 360 end
      if rot_y < -180 then rot_y = rot_y + 360 end

      camera_turn(rot_x, rot_y, 0)
   end
   return true
end

function do_timer(dt)

   rot_y = rot_y + dt * rot_dy
   foo_a = foo_a + dt * rot_dy * 100

   if rot_y >  180 then rot_y = rot_y - 360 end
   if rot_y < -180 then rot_y = rot_y + 360 end
   if foo_a >  180 then foo_a = foo_a - 360 end
   if foo_a < -180 then foo_a = foo_a + 360 end

   camera_turn(rot_x, rot_y, 0)

   if (foo ~= -1) then sprite_turn(foo, foo_a) end

   return true
end

-------------------------------------------------------------------------------
