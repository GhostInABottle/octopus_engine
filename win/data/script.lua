function print_table(tt, indent, done)
  done = done or {}
  indent = indent or 0
  if type(tt) == "table" then
    local result = string.rep(" ", indent) .. '{\n'
    indent = indent + 2
    for key, value in pairs (tt) do
      result = result .. string.rep(" ", indent) -- indent it
      if type (value) == "table" and not done [value] then
        done [value] = true
        result = result ..  key .. " =\n";
        result = result ..  print_table(value, indent + 2, done)
        result = result ..  string.rep(" ", indent) -- indent it
        result = result ..  "\n";
      elseif "number" == type(key) then
        result = result ..  string.format("\"%s\"\n", tostring(value))
      else
        result = result ..  string.format(
            "%s = \"%s\"\n", tostring(key), tostring(value))
       end
    end
    indent = indent - 2
    return result .. string.rep(" ", indent) ..  '}'
  else
    return tostring(tt) .. "\n"
  end
end
local tbl = {
    a = 'aa',
    b = 'ba',
    c = 'ca',
    d = 6.5,
    e = false,
    f = {
        x = 1,
        y = true,
        z = 'w',
    },
    g = nil,
    x = function() end
}
print(print_table(tbl))
game:save('data/test_save.txt', tbl)
tbl2 = game:load('data/test_save.txt')
print(print_table(tbl2))
player.disabled = true
print("Objects:\n")
local objects = current_map:get_objects()
for i,v in ipairs(objects) do
    print(i,": ", v.name)
end
local music = game:playing_music()
music:pause()
local o = current_map:get_object("jimbo")
local text = Canvas(100, 100, "{typewriter}{rainbow}Hello, world!{/rainbow}{/typewriter}")
text:show()
wait("a")
text:hide()
camera:tint_screen(Color(0.4, 0.2, 0.6, 0.3), 1000)
local canvas = Canvas("data/player.png", 100, 100, "#FF00FF")
canvas.magnification = Vec2(0, 0)
canvas:show()
canvas:update(200, 100, 1, 1, 180, 1, 1500):wait()
wait(500)
canvas:update(100, 100, 1, 1, 0, 0, 1500):wait()
music:play()
o:move(UP, 32, true, false):wait()
camera:move(UP, 120, 1):wait()
camera:move_to(player, 1):wait()
camera:track_object(player)
o:move(LEFT, 32):wait()
o:move(UP, 32):wait()
o:move(RIGHT, 64):wait()
o:move(DOWN, 100):wait()
o:face(RIGHT)
o:move(BACKWARD, 50):wait()
player.disabled = false
camera:tint_screen(Color(0.4, 0.2, 0.6, 0), 1000):wait()
o:show_pose("Pose Test"):wait()
wait(2000)
o:face(LEFT)