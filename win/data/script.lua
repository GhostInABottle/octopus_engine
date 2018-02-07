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
print ("Saving a table:")
print(print_table(tbl))
game:save('data/test_save.txt', tbl)
tbl2 = game:load('data/test_save.txt')
print("And loading it back:")
print(print_table(tbl2))
player.disabled = true
print("Objects:")
local objects = current_map:get_objects()
for i,v in ipairs(objects) do
    print(i,": ", v.name)
end
print("Pausing music")
local music = game:playing_music()
music:pause()
local o = current_map:get_object("jimbo")
print("Type of Object:", type(o), " - type of Vec2: ", type(Vec2(0,0)))
print("Showing some text")
text(o, "{typewriter}Test? {rainbow}Hello, world!{/rainbow}{/typewriter}\nHow{bold} about {/bold}that?"):wait()
text(Vec2(100, 100), "{typewriter}Slowly now...{/typewriter}", 2000):wait()
centered_text(100, "Some centered {italic}text goes{/italic} here, ha ha\nyeah that's right!"):wait()
local c = choices(o, "Pick a choice", { 'Yes', 'No' })
c:wait()
print("Choice: ", c.selected)
wait(500)
local text_canvas = Canvas(40, 40, "This is a {bold}Canvas{/bold} {italic}test{/italic},\n will it work?")
text_canvas:show()
wait(1500)
print('Font size ' .. text_canvas.font_size)
text_canvas.font_size = 15
function color_to_s(color)
 return '(' .. color.r .. ', ' .. color.g .. ', ' .. color.b .. ', ' .. color.a .. ')'
end
print('Text color ' .. color_to_s(text_canvas.text_color))
text_canvas.text_color = Color('red')
print('Line height ' .. text_canvas.line_height)
text_canvas.line_height = 20
print('Outline width ' .. text_canvas.text_outline_width)
text_canvas.text_outline_width = 2
print('Outline color ' .. color_to_s(text_canvas.text_outline_color))
text_canvas.text_outline_color = Color('yellow')
print('Shadow offset (' .. text_canvas.text_shadow_offset.x .. ', ' .. text_canvas.text_shadow_offset.y .. ')')
text_canvas.text_shadow_offset = Vec2(-18, -18)
print('Shadow color ' .. color_to_s(text_canvas.text_shadow_color))
text_canvas.text_shadow_color = Color('blue')
wait(3000)
print('Setting font')
text_canvas:set_font('data/Roboto-Bold.ttf')
print('Setting linked font')
text_canvas:set_linked_fonts('data/Roboto-Italic.ttf', 'data/Roboto-Regular.ttf')
wait(3000)
text_canvas:hide()
print("Tinting screen")
camera:tint_screen(Color(0.4, 0.2, 0.6, 0.3), 1000)
print("Showing a canvas")
local canvas = Canvas("data/player.png", 100, 100, "#FF00FF")
canvas.magnification = Vec2(0, 0)
canvas:show()
print("Updating it")
canvas:update(200, 100, 1, 1, 180, 1, 1500):wait()
wait(500)
canvas:update(100, 100, 1, 1, 0, 0, 1500):wait()
print("Resuming music")
music:play()
print("Moving object")
o:move(UP, 32, true, false):wait()
print("Moving camera")
camera:move(UP, 120, 1):wait()
camera:move_to(player, 1):wait()
camera:track_object(player)
print("Moving object")
o:move(LEFT, 32):wait()
o:move(UP, 32):wait()
o:move(RIGHT, 64):wait()
o:move(DOWN, 100):wait()
o:face(RIGHT)
o:move(BACKWARD, 50):wait()
print("Tinting screen")
player.disabled = false
camera:tint_screen(Color(0.4, 0.2, 0.6, 0), 1000):wait()
print("Showing a pose")
o:show_pose("Pose Test"):wait()
wait(2000)
o:face(LEFT)