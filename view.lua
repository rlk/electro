
-------------------------------------------------------------------------------

pos_x  =    0.0
pos_y  =   15.5
pos_z  = 9200.0
rot_x  =    0.0
rot_y  =    0.0
rot_dy =    0.0
dist   =    0.0
magn   =  128.0

btn    = { }

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

   if b == 4 then -- mouse wheel down
      dist = dist + 16.0
      camera_dist(dist)
      return true
   end

   if b == 5 then -- mouse wheel up
      dist = dist - 16.0
      camera_dist(dist)
      return true
   end
   
   if b == 2 then -- middle button click
      dist = 0
      camera_dist(dist)
      return true
   end

   btn[b] = s

   return false
end

function do_point(dx, dy)

   rot_x = rot_x - 180 * dy / 500.0
   rot_y = rot_y - 180 * dx / 500.0

   if rot_x >   90 then rot_x =  90 end
   if rot_x <  -90 then rot_x = -90 end
   if rot_y >  180 then rot_y = rot_y - 360 end
   if rot_y < -180 then rot_y = rot_y + 360 end

   camera_turn(rot_x, rot_y, 0)

   return true
end

function do_timer(dt)

   rot_y = rot_y + dt * rot_dy

   if rot_y >  180 then rot_y = rot_y - 360 end
   if rot_y < -180 then rot_y = rot_y + 360 end

   camera_turn(rot_x, rot_y, 0)

   return true
end

-------------------------------------------------------------------------------
