-- Type annotations for engine types for use with lua-language-server
-- Reference: https://github.com/sumneko/lua-language-server/wiki/Annotations

---@meta

-- Global variables

---@alias Engine_Direction 0|1|2|3|4|6|8|9|12|16|32

---@type Engine_Direction
NONE = 0
---@type Engine_Direction
UP = 1
---@type Engine_Direction
RIGHT = 2
---@type Engine_Direction
DOWN = 4
---@type Engine_Direction
LEFT = 8
---@type Engine_Direction
FORWARD = 16
---@type Engine_Direction
BACKWARD = 32

---@type 'GLOBAL'|'MAP'
SCRIPT_CONTEXT = nil

-- Enumerations

---@enum Engine_Draw_Order
Draw_Order = {
    below = 0,
    normal = 1,
    above = 2
}

---@enum Engine_Input_Type
Input_Type = {
    keyboard = 0,
    mouse = 1,
    gamepad = 2
}

---@enum Engine_Script_Context
Script_Context = {
    map = 0,
    global = 1
}

---@enum Engine_Passthrough_Type
Passthrough_Type = {
    initiator = 1,
    receiver = 2,
    both = 3
}

---@enum Engine_Text_Position_Type
Text_Position_Type = {
    none = 0,
    -- Interpret position literally, X refers to left of text
    exact_x = 1,
    -- The text is centered around the X coordinate
    centered_x = 2,
    -- Interpret position literally, Y refers to the top of text
    exact_y = 4,
    -- Text sits above the Y coordinate instead of under it
    bottom_y = 8,
    -- The supplied coordinates are relative to the camera instead of the map
    camera_relative = 16,
    -- The text should always be visible, makes map coordinates camera relative
    always_visible = 32
}

---@enum Engine_Token_Type
Token_Type = {
    text = 0,
    opening_tag = 1,
    closing_tag = 2
}

---@enum Engine_Outline_Condition
Outline_Condition = {
    -- Always outlined
    none = 0,
    -- Outline if touched by player
    touched = 1,
    -- Outline if object is not passthrough
    solid = 2,
    -- Outline if object has a script
    script = 4,
    -- Never outline
    never = 8
}

---@enum Engine_Open_Page_Mode
Open_Page_Mode = {
    -- The environment's preferred mode
    any = 0,
    -- Launch the default browser
    external = 1,
    -- Open through the environment's app/client
    internal = 2,
    -- Open as an overlay on top of the game
    overlay = 3,
}

-- Global functions

---@overload fun(function : fun() : boolean)
---@param duration integer
function wait(duration) end

-- Bit operations

bit = {}

---@param a integer
---@param b integer
---@return integer
---@diagnostic disable-next-line: duplicate-set-field
function bit.band(a, b) end

---@param a integer
---@param b integer
---@return integer
---@diagnostic disable-next-line: duplicate-set-field
function bit.bor(a, b) end

---@param a integer
---@param b integer
---@return integer
---@diagnostic disable-next-line: duplicate-set-field
function bit.bxor(a, b) end

---@param a integer
---@param b integer
---@return integer
---@diagnostic disable-next-line: duplicate-set-field
function bit.rshift(a, b) end

---@param a integer
---@param b integer
---@return integer
---@diagnostic disable-next-line: duplicate-set-field
function bit.lshift(a, b) end

---@param a integer
---@return integer
---@diagnostic disable-next-line: duplicate-set-field
function bit.bnot(a) end

-- File system

---@class (exact) Engine_Calendar_Time
---@field second integer # 0-60
---@field minute integer # 0-59
---@field hour integer # 0-23
---@field month_day integer # 1-31
---@field month integer # 0-11
---@field year integer
---@field week_day integer # 0-6
---@field year_day integer # 0-365
---@field is_dst? boolean # if nil, no info is available

---@class (exact) Engine_Path_Info
---@field name string
---@field is_regular boolean
---@field is_directory boolean
---@field timestamp integer
---@field calendar_time Engine_Calendar_Time

---@class (exact) Engine_Readable_Filesystem
local Engine_Readable_Filesystem = {}

---@param path string
---@return boolean
function Engine_Readable_Filesystem:exists(path) end

---@param path string
---@return boolean
function Engine_Readable_Filesystem:is_regular_file(path) end

---@param path string
---@return boolean
function Engine_Readable_Filesystem:is_directory(path) end

---@param path string
---@return boolean
function Engine_Readable_Filesystem:is_absolute(path) end

---@param path string
---@return string
function Engine_Readable_Filesystem:get_basename(path) end

---@param path string
---@return string
function Engine_Readable_Filesystem:get_stem(path) end

---@param path string
---@return integer ms_timestamp, Engine_Calendar_Time calendar_time
function Engine_Readable_Filesystem:last_write_time(path) end

---@param path string
---@return string[]
function Engine_Readable_Filesystem:list_directory(path) end

---@param path string
---@return Engine_Path_Info[]
function Engine_Readable_Filesystem:list_detailed_directory(path) end

---@class (exact) Engine_Writable_Filesystem : Engine_Readable_Filesystem
local Engine_Writable_Filesystem = {}

---@param source string
---@param destination string
---@return boolean copied
function Engine_Writable_Filesystem:copy(source, destination) end

---@param path string
---@return boolean removed
function Engine_Writable_Filesystem:remove(path) end

---@param old_path string
---@param new_path string
---@return boolean renamed
function Engine_Writable_Filesystem:rename(old_path, new_path) end

---@class (exact) Engine_User_Data_Folder
---@field base_path string # readonly
---@field game_path string # readonly
---@field user_path string # readonly
---@field version_path string # readonly
local Engine_User_Data_Folder = {}

---@param filename string
---@param data Save_Data
---@param header? Save_Data_Header
---@param compact? boolean
---@return boolean success
function Engine_User_Data_Folder:save(filename, data, header, compact) end

---@param filename string
---@param compact? boolean
---@return Save_Data save_data, Save_Data_Header header
function Engine_User_Data_Folder:load(filename, compact) end

---@param filename string
---@param compact? boolean
---@return Save_Data_Header header
function Engine_User_Data_Folder:load_header(filename, compact) end

function Engine_User_Data_Folder:save_config_file() end

function Engine_User_Data_Folder:save_keymap_file() end

-- Text and Choices

-- All properties are read only once created
---@class (exact) Engine_Text_Options
---@field text string?
---@field choices string[]?
---@field object Engine_Map_Object?
---@field position Engine_Vec2?
---@field position_type Engine_Text_Position_Type?
---@field duration integer?
---@field centered boolean?
---@field show_dashes boolean?
---@field cancelable boolean?
---@field choice_indent integer?
---@field canvas_priority integer?
---@field fade_in_duration integer?
---@field fade_out_duration integer?
---@field background_visible boolean?
---@field background_color Engine_Color?
---@field typewriter_on boolean?
---@field typewriter_delay integer?
---@field typewriter_sound string?
---@field typewriter_sound_volume number?
---@field typewriter_sound_pitch number?
---@field typewriter_sound_max_pitch number?
---@field typewriter_skippable boolean?
local Engine_Text_Options = {}

---@param text string
---@return Engine_Text_Options
function Engine_Text_Options:set_text(text) end

---@param object Engine_Map_Object
---@return Engine_Text_Options
function Engine_Text_Options:set_object(object) end

---@param position Engine_Vec2
---@return Engine_Text_Options
function Engine_Text_Options:set_position(position) end

---@param duration integer
---@return Engine_Text_Options
function Engine_Text_Options:set_duration(duration) end

---@param centered boolean
---@return Engine_Text_Options
function Engine_Text_Options:set_centered(centered) end

---@param show_dashes boolean
---@return Engine_Text_Options
function Engine_Text_Options:set_show_dashes(show_dashes) end

---@param cancelable boolean
---@return Engine_Text_Options
function Engine_Text_Options:set_cancelable(cancelable) end

---@param choice_indent integer
---@return Engine_Text_Options
function Engine_Text_Options:set_choice_indent(choice_indent) end

---@param priority integer
---@return Engine_Text_Options
function Engine_Text_Options:set_canvas_priority(priority) end

---@param duration integer
---@return Engine_Text_Options
function Engine_Text_Options:set_fade_in_duration(duration) end

---@param duration integer
---@return Engine_Text_Options
function Engine_Text_Options:set_fade_out_duration(duration) end

---@param visible boolean
---@return Engine_Text_Options
function Engine_Text_Options:set_background_visible(visible) end

---@param color Engine_Color
---@return Engine_Text_Options
function Engine_Text_Options:set_background_color(color) end

---@param position_type integer|Engine_Text_Position_Type
---@return Engine_Text_Options
function Engine_Text_Options:set_position_type(position_type) end

---@param typewriter_on boolean
---@return Engine_Text_Options
function Engine_Text_Options:set_typewriter_on(typewriter_on) end

---@param typewriter_delay integer
---@return Engine_Text_Options
function Engine_Text_Options:set_typewriter_delay(typewriter_delay) end

---@param typewriter_sound string
---@return Engine_Text_Options
function Engine_Text_Options:set_typewriter_sound(typewriter_sound) end

---@param volume number
---@return Engine_Text_Options
function Engine_Text_Options:set_typewriter_sound_volume(volume) end

---@param pitch number
---@return Engine_Text_Options
function Engine_Text_Options:set_typewriter_sound_pitch(pitch) end

---@param pitch number
---@return Engine_Text_Options
function Engine_Text_Options:set_typewriter_sound_max_pitch(pitch) end

---@param typewriter_skippable boolean
---@return Engine_Text_Options
function Engine_Text_Options:set_typewriter_skippable(typewriter_skippable) end

---@param choices string[]
---@return Engine_Text_Options
function Engine_Text_Options:set_choices(choices) end

---@overload fun(object : Engine_Map_Object) : Engine_Text_Options
---@param position Engine_Vec2
---@return Engine_Text_Options
function Text_Options(position) end

---@overload fun(text_options : Engine_Text_Options) : Engine_Command_Result
---@overload fun(position : Engine_Vec2, text : string, duration? : integer) : Engine_Command_Result
---@param object Engine_Map_Object
---@param text string
---@param duration integer?
---@return Engine_Command_Result
function text(object, text, duration) end

---@param y number
---@param text string
---@param duration? integer
---@return Engine_Command_Result
function centered_text(y, text, duration) end

---@overload fun(text_options : Engine_Text_Options) : Engine_Choice_Command_Result
---@overload fun(position : Engine_Vec2, text : string, choice_list : string[], cancelable? : boolean) : Engine_Choice_Command_Result
---@param object Engine_Map_Object
---@param text string
---@param choice_list string[]
---@param cancelable? boolean
---@return Engine_Choice_Command_Result
function choices(object, text, choice_list, cancelable) end

lutf8 = {}

---@param s string
---@param i? integer
---@param j? integer
---@return integer, ...
---@nodiscard
function lutf8.byte(s, i, j) end

---@param byte integer
---@param ... integer
---@return string, ...
---@nodiscard
function lutf8.char(byte, ...) end

---@param s string
---@param pattern string
---@param init? integer
---@param plain? boolean
---@return integer start, integer end, ... captured
---@nodiscard
function lutf8.find(s, pattern, init, plain) end

---@param s string
---@param pattern string
---@return fun() : string, ...
---@nodiscard
function lutf8.gmatch(s, pattern) end

---@param s string
---@param pattern string
---@param repl string|table|function
---@param n? integer
---@return string, integer count
---@nodiscard
function lutf8.gsub(s, pattern, repl, n) end

---@param s string
---@return integer
---@nodiscard
function lutf8.len(s) end

---@param s string
---@return string
---@nodiscard
function lutf8.lower(s) end

---@param s string
---@param pattern string
---@param init? integer
---@return ... captured
---@nodiscard
function lutf8.match(s, pattern, init) end

---@param s string
---@return string
---@nodiscard
function lutf8.reverse(s) end

---@param s string
---@param i integer
---@param j? integer
---@return string
---@nodiscard
function lutf8.sub(s, i, j) end

---#DES 'string.upper'
---@param s string
---@return string
---@nodiscard
function lutf8.upper(s) end

---@param s string
---@param lax? boolean
---@return fun(s: string, p: integer):integer, integer
function lutf8.codes(s, lax) end

---@param s string
---@param i? integer
---@param j? integer
---@param lax? boolean
---@return integer code
---@return ...
---@nodiscard
function lutf8.codepoint(s, i, j, lax) end

---@param s string
---@param n integer
---@param i integer
---@return integer p
---@nodiscard
function lutf8.offset(s, n, i) end

---@overload fun(s : string, substring : string) : string
---@param s string
---@param idx integer
---@param substring string
---@return string new_string
function lutf8.insert(s, idx, substring) end

---@param s string
---@param start? integer
---@param stop? integer
---@return string new_string
function lutf8.remove(s, start, stop) end

---@param s string
---@param ambi_is_double? boolean
---@param default_width? integer
---@return integer width
function lutf8.width(s, ambi_is_double, default_width) end

-- This is the reverse operation of lutf8.width
---@param s string
---@param location integer
---@param ambi_is_double? boolean
---@param default_width? integer
function lutf8.width_index(s, location, ambi_is_double, default_width) end

-- Convert to title case
---@param s string
---@return string new_string
function lutf8.title(s) end

-- Convert to folded case for comparison
---@param s string
---@return string new_string
function lutf8.fold(s) end

-- Compare a and b without case, -1 means a < b, 0 means a == b and 1 means a > b.
---@param a string
---@param b string
---@return integer
function lutf8.ncasecmp(a, b) end

-- Direction utility functions

direction = {}

---@param direction Engine_Direction
---@param component Engine_Direction
---@return boolean
function direction.contains(direction, component) end

---@param direction Engine_Direction
---@return Engine_Direction opposite_direction
function direction.opposite(direction) end

---@param direction Engine_Direction
---@return Engine_Vec2 vector
function direction.to_vector(direction) end

---@param vector Engine_Vec2
---@return Engine_Direction direction
function direction.from_vector(vector) end

---@param direction Engine_Direction
---@return string str
function direction.to_string(direction) end

---@param str string
---@return Engine_Direction direction
function direction.from_string(str) end

---@param pos1 Engine_Vec2
---@param pos2 Engine_Vec2
---@param diagonal? boolean
---@return Engine_Direction direction
function direction.facing_direction(pos1, pos2, diagonal) end

---@param direction Engine_Direction
---@return boolean
function direction.is_diagonal(direction) end

-- Prefers directions in this order: UP, DOWN, LEFT, RIGHT
---@param direction Engine_Direction
---@return Engine_Direction
function direction.to_four_directions(direction) end

-- Logging

---@class (exact) Engine_Logger
---@field info fun(message : string)
---@field debug fun(message : string)
---@field warning fun(message : string)
---@field error fun(message : string)
logger = {}

-- Vec2

---@class (exact) Engine_Vec2
---@field x number
---@field y number
---@operator add(Engine_Vec2) : Engine_Vec2
---@operator sub(Engine_Vec2) : Engine_Vec2
---@operator mul(number) : Engine_Vec2
---@operator div(number) : Engine_Vec2
local Engine_Vec2 = {}

---@overload fun() : Engine_Vec2
---@overload fun(other : Engine_Vec2) : Engine_Vec2
---@param x number
---@param y number
---@return Engine_Vec2
function Vec2(x, y) end

---@return number
function Engine_Vec2:length() end

---@return Engine_Vec2
function Engine_Vec2:normal() end

-- Vec3

---@class (exact) Engine_Vec3
---@field x number
---@field y number
---@field z number
---@operator add(Engine_Vec3) : Engine_Vec3
---@operator sub(Engine_Vec3) : Engine_Vec3
---@operator mul(number) : Engine_Vec3
---@operator div(number) : Engine_Vec3
local Engine_Vec3 = {}

---@overload fun() : Engine_Vec3
---@overload fun(other : Engine_Vec3) : Engine_Vec3
---@param x number
---@param y number
---@param z number
---@return Engine_Vec3
function Vec3(x, y, z) end

---@return number
function Engine_Vec3:length() end

---@return Engine_Vec3
function Engine_Vec3:normal() end

-- Vec4 / Color
---@class (exact) Engine_Vec4
---@field x number
---@field y number
---@field z number
---@field w number
---@operator add(Engine_Vec4) : Engine_Vec4
---@operator sub(Engine_Vec4) : Engine_Vec4
---@operator mul(number) : Engine_Vec4
---@operator div(number) : Engine_Vec4
local Engine_Vec4 = {}

---@overload fun() : Engine_Vec4
---@overload fun(other : Engine_Vec4) : Engine_Vec4
---@param x number
---@param y number
---@param z number
---@param w number
---@return Engine_Vec4
function Vec4(x, y, z, w) end

---@return number
function Engine_Vec4:length() end

---@return Engine_Vec3
function Engine_Vec4:normal() end

---@class (exact) Engine_Color
---@field r number
---@field g number
---@field b number
---@field a number
local Engine_Color = {}

---@return string
function Engine_Color:to_hex() end

---@alias Engine_Color_Name
---|'clear'
---|'none'
---|'black'
---|'red'
---|'green'
---|'blue'
---|'yellow'
---|'white'
---|'gray'
---|'purple'
---|'cyan'

---@overload fun(color : Engine_Color) : Engine_Color
---@overload fun(color_name : string|Engine_Color_Name) : Engine_Color
---@param r number
---@param g number
---@param b number
---@param a? number
---@return Engine_Color
function Color(r, g, b, a) end

-- Rect

---@class (exact) Engine_Rect
---@field x number
---@field y number
---@field w number
---@field h number
---@field position Engine_Vec2
---@field size Engine_Vec2
---@field center Engine_Vec2
local Engine_Rect = {}

---@overload fun() : Engine_Rect
---@overload fun(other : Engine_Rect) : Engine_Rect
---@overload fun(position : Engine_Vec2, size : Engine_Vec2) : Engine_Rect
---@overload fun(position : Engine_Vec2, w : number, h : number) : Engine_Rect
---@overload fun(vec4 : Engine_Vec4) : Engine_Rect
---@param x number
---@param y number
---@param w number
---@param h number
---@return Engine_Rect
function Rect(x, y, w, h) end

-- Check if this rectangle partially overlaps with another (excluding the sides)
---@param other Engine_Rect
---@return boolean
function Engine_Rect:intersects(other) end

-- Check if the sides of this rectangle and the other one touch
---@param other Engine_Rect
---@return boolean
function Engine_Rect:touches(other) end

-- Check if a point falls within this rectangle (including the sides)
---@overload fun(x : number, y : number) : boolean
---@param point Engine_Vec2
---@return boolean
function Engine_Rect:contains(point) end

-- Change the position of this rectangle
---@param displacement Engine_Vec2
---@return boolean
function Engine_Rect:move(displacement) end

-- Circle

---@class (exact) Engine_Circle
---@field x number
---@field y number
---@field radius number
---@field center Engine_Vec2
local Engine_Circle = {}

---@overload fun() : Engine_Circle
---@overload fun(other : Engine_Circle) : Engine_Circle
---@overload fun(center : Engine_Vec2, radius: number) : Engine_Circle
---@param x number
---@param y number
---@param radius number
---@return Engine_Circle
function Circle(x, y, radius) end

-- Check if this circle partially overlaps with another circle
-- or rectangle (excluding when the perimeters/sides touch)
---@param other Engine_Circle|Engine_Rect
---@return boolean
function Engine_Circle:intersects(other) end

-- Check if the perimeters of this circle and the other one touch
---@param other Engine_Circle
---@return boolean
function Engine_Circle:touches(other) end

-- Check if a point falls within this circle (including along the perimeters)
---@overload fun(x : number, y : number) : boolean
---@param point Engine_Vec2
---@return boolean
function Engine_Circle:contains(point) end

-- Change the position of this circle
---@param displacement Engine_Vec2
---@return boolean
function Engine_Circle:move(displacement) end

-- Create a rectangle that covers the area of this circle
---@return Engine_Rect
function Engine_Circle:to_rect() end

-- Command_Result

---@class (exact) Engine_Command_Result
---@field completed boolean # readonly
---@field stopped boolean # readonly
---@field paused boolean # readonly
local Engine_Command_Result = {}

function Engine_Command_Result:wait() end

function Engine_Command_Result:pause() end

function Engine_Command_Result:resume() end

-- Completes command to desired state
function Engine_Command_Result:stop() end

-- Stops the command immediately
function Engine_Command_Result:force_stop() end

---@param ticks? integer
function Engine_Command_Result:execute(ticks) end

---@param ticks? integer
---@return boolean
function Engine_Command_Result:is_complete(ticks) end

---@class (exact) Engine_Choice_Command_Result : Engine_Command_Result
---@field selected integer # 1-based choice index, -1 if canceled

---@param ms_duration integer
---@param seconds_start_time integer
---@return Engine_Command_Result
function Wait_Command(ms_duration, seconds_start_time) end

---@param object Engine_Map_Object
---@param x number
---@param y number
---@param keep_trying boolean
---@param tile_only boolean
---@return Engine_Command_Result
function Move_To_Command(object, x, y, keep_trying, tile_only) end

---@param object Engine_Map_Object
---@param pose string
---@param state string
---@param direction Engine_Direction
---@return Engine_Command_Result
function Pose_Command(object, pose, state, direction) end

---@overload fun(text_options : Engine_Text_Options, start_time : integer) : Engine_Command_Result
---@param object Engine_Map_Object
---@param text string
---@param duration integer
---@param start_time integer
---@return Engine_Command_Result
function Text_Command(object, text, duration, start_time) end

-- Token

---@class (exact) Engine_Token
---@field type Engine_Token_Type
---@field tag string
---@field value string
---@field unmatched boolean
---@field start_index integer # 0-based
---@field end_index integer # 0-based
---@field self_closing boolean

---@class Engine_Text_Parser
---@overload fun() : Engine_Text_Parser
Text_Parser = {}

---@param text string
---@param permissive boolean
---@return Engine_Token[]
function Text_Parser:parse(text, permissive) end

-- Shared canvas properties and methods

---@class Engine_Base_Canvas
---@field data table
---@field id integer
---@field name string
---@field position Engine_Vec2
---@field x number
---@field y number
---@field priority integer
---@field visible boolean
---@field camera_relative boolean # defaults to true
---@field opacity number
---@field child_count integer # readonly
---@field scissor_box Engine_Rect
---@field has_background boolean
---@field background_color Engine_Color
---@field background_rect Engine_Rect
---@field color Engine_Color
local Engine_Base_Canvas = {}

function Engine_Base_Canvas:show() end

function Engine_Base_Canvas:hide() end

---@overload fun(self : Engine_Base_Canvas, pos : Engine_Vec2, duration : integer) : Engine_Command_Result
---@param x number
---@param y number
---@param duration integer
---@return Engine_Command_Result
function Engine_Base_Canvas:move(x, y, duration) end

---@param opacity number
---@param duration integer
---@return Engine_Command_Result
function Engine_Base_Canvas:update_opacity(opacity, duration) end

---@param color Engine_Color
---@param duration integer
---@return Engine_Command_Result
function Engine_Base_Canvas:update_color(color, duration) end

---@overload fun(self : Engine_Base_Canvas, child_name : string, filename : string, pos : Engine_Vec2, transparent_color : Engine_Color|string?) : Engine_Image_Canvas
---@param child_name string
---@param image_filename string
---@param x number
---@param y number
---@param transparent_color Engine_Color|string?
---@return Engine_Image_Canvas
function Engine_Base_Canvas:add_child_image(child_name, image_filename, x, y, transparent_color) end

---@overload fun(self : Engine_Base_Canvas, child_name : string, sprite_filename : string, position : Engine_Vec2, pose? : string) : Engine_Sprite_Canvas
---@param child_name string
---@param image_filename string
---@param x number
---@param y number
---@param pose string?
---@return Engine_Sprite_Canvas
function Engine_Base_Canvas:add_child_sprite(child_name, image_filename, x, y, pose) end

---@overload fun(self : Engine_Base_Canvas, child_name : string, position : Engine_Vec2, text : string) : Engine_Text_Canvas
---@param child_name string
---@param x number
---@param y number
---@param text string
---@return Engine_Text_Canvas
function Engine_Base_Canvas:add_child_text(child_name, x, y, text) end

---@param child_name string
function Engine_Base_Canvas:remove_child(child_name) end

---@overload fun(self : Engine_Base_Canvas, index : integer) : Engine_Text_Canvas
---@param child_name string
---@return Engine_Text_Canvas
function Engine_Base_Canvas:get_text_child(child_name) end

---@overload fun(self : Engine_Base_Canvas, index : integer) : Engine_Image_Canvas
---@param child_name string
---@return Engine_Image_Canvas
function Engine_Base_Canvas:get_image_child(child_name) end

---@overload fun(self : Engine_Base_Canvas, index : integer) : Engine_Sprite_Canvas
---@param child_name string
---@return Engine_Sprite_Canvas
function Engine_Base_Canvas:get_sprite_child(child_name) end

-- Text canvas

---@class (exact) Engine_Text_Canvas : Engine_Base_Canvas
---@field text string
---@field font_size number
---@field line_height number
---@field outline_width number
---@field outline_color Engine_Color
---@field shadow_offset Engine_Vec2
---@field shadow_color Engine_Color
---@field font_type string
---@field permissive_tag_parsing boolean
local Engine_Text_Canvas = {}

---@param font_filename string
function Engine_Text_Canvas:set_font(font_filename) end

---@param type string
---@param font_filename string
function Engine_Text_Canvas:link_font(type, font_filename) end

---@param text string
---@return number
function Engine_Text_Canvas:text_width(text) end

function Engine_Text_Canvas:reset_outline() end

function Engine_Text_Canvas:reset_shadow() end

function Engine_Text_Canvas:reset_font_type() end

---@overload fun(position : Engine_Vec2, text : string) : Engine_Text_Canvas
---@param x number
---@param y number
---@param text string
---@return Engine_Text_Canvas
function Text_Canvas(x, y, text) end

-- Shared image/sprite canvas properties and methods

---@class (exact) Engine_Base_Image_Canvas : Engine_Base_Canvas
---@field filename string # readonly
---@field magnification Engine_Vec2
---@field angle integer # in degrees
---@field origin Engine_Vec2? # defaults to (0, 0) for sprites, (0.5, 0.5) for images
---@field outline_color Engine_Color?
local Engine_Base_Image_Canvas = {}

---@overload fun(self : Engine_Base_Image_Canvas, pos : Engine_Vec2, mag : Engine_Vec2, angle : integer, opacity : number, duration : integer) : Engine_Command_Result
---@param x number
---@param y number
---@param mag_x number
---@param mag_y number
---@param angle integer
---@param opacity number
---@param duration integer
---@return Engine_Command_Result
function Engine_Base_Image_Canvas:update(x, y, mag_x, mag_y, angle, opacity, duration) end

---@overload fun(self : Engine_Base_Image_Canvas, mag : number, duration : integer) : Engine_Command_Result
---@overload fun(self : Engine_Base_Image_Canvas, mag : Engine_Vec2, duration : integer) : Engine_Command_Result
---@param mag_x number
---@param mag_y number
---@param duration integer
---@return Engine_Command_Result
function Engine_Base_Image_Canvas:resize(mag_x, mag_y, duration) end

---@param angle integer # in degrees
---@param duration integer
---@return Engine_Command_Result
function Engine_Base_Image_Canvas:rotate(angle, duration) end

-- Image canvas

---@class (exact) Engine_Image_Canvas : Engine_Base_Image_Canvas
---@field width integer # readonly
---@field height integer # readonly
local Engine_Image_Canvas = {}

---@param filename string
---@param transparent? Engine_Color|string
function Engine_Image_Canvas:set_image(filename, transparent) end

---@overload fun(image_filename : string, position : Engine_Vec2, transparent_color : Engine_Color|string?) : Engine_Image_Canvas
---@param image_filename string
---@param x number
---@param y number
---@param transparent_color Engine_Color|string?
---@return Engine_Image_Canvas
function Image_Canvas(image_filename, x, y, transparent_color) end

-- Sprite canvas

---@class (exact) Engine_Sprite_Canvas : Engine_Base_Image_Canvas
---@field pose_name string # readonly
---@field pose_state string # readonly
---@field pose_direction Engine_Direction # readonly
---@field sprite string
local Engine_Sprite_Canvas = {}

---@param filename string
---@param pose? string
function Engine_Sprite_Canvas:set_sprite(filename, pose) end

---@param pose string
---@param state? string
---@param direction? string
---@return Engine_Command_Result
function Engine_Sprite_Canvas:show_pose(pose, state, direction) end

-- Reset the sprite
function Engine_Sprite_Canvas:reset() end

---@overload fun(sprite_filename : string, position : Engine_Vec2, pose? : string) : Engine_Sprite_Canvas
---@param sprite_filename string
---@param x number
---@param y number
---@param pose string?
---@return Engine_Sprite_Canvas
function Sprite_Canvas(sprite_filename, x, y, pose) end

-- Map_Object

---@class (exact) Engine_Map_Object
---@field id integer
---@field name string
---@field type string
---@field position Engine_Vec2
---@field real_position Engine_Vec2 # position + bounding_box.position
---@field centered_position Engine_Vec2 # real_position + bounding_box.size / 2
---@field bounding_box Engine_Rect
---@field bounding_circle Engine_Circle?
---@field positioned_bounding_box Engine_Rect # bounding box positioned at real_position
---@field positioned_bounding_circle Engine_Circle?
---@field x number
---@field y number
---@field size Engine_Vec2
---@field sprite string
---@field pose string
---@field state string
---@field direction Engine_Direction
---@field walk_state string
---@field face_state string
---@field visible boolean
---@field frozen boolean # prevents pose state from changing
---@field disabled boolean
---@field stopped boolean # stops and prevents movement
---@field passthrough boolean
---@field passthrough_type Engine_Passthrough_Type
---@field speed number
---@field fps_independent_speed number # readonly
---@field collision_object Engine_Map_Object?
---@field triggered_object Engine_Map_Object?
---@field collision_area Engine_Map_Object?
---@field magnification Engine_Vec2
---@field sprite_magnification Engine_Vec2
---@field color Engine_Color
---@field opacity number
---@field angle number
---@field uses_layer_color boolean
---@field script string
---@field touch_script string
---@field trigger_script string
---@field leave_script string
---@field script_context Engine_Script_Context
---@field overrides_tile_collision boolean
---@field strict_multidirectional_movement boolean
---@field player_facing boolean
---@field outline_color Engine_Color
---@field outlined boolean?
---@field outlined_object Engine_Map_Object?
---@field outlining_object Engine_Map_Object?
---@field outline_conditions Engine_Outline_Condition
---@field sfx_attenuation boolean
---@field draw_order Engine_Draw_Order
local Engine_Map_Object = {}

---@param pose string
---@param state? string
---@param direction? Engine_Direction
---@return Engine_Command_Result
function Engine_Map_Object:show_pose(pose, state, direction) end

---@param filename string
---@param pose? string
function Engine_Map_Object:set_sprite(filename, pose) end

function Engine_Map_Object:reset() end

---@overload fun(self : Engine_Map_Object, x : number, y : number)
---@overload fun(self : Engine_Map_Object, position : Engine_Vec2)
---@overload fun(self : Engine_Map_Object, other_object : Engine_Map_Object)
---@param direction Engine_Direction
function Engine_Map_Object:face(direction) end

---@param direction Engine_Direction
---@param pixels number
---@param skip? boolean
---@param change_facing? boolean
---@return Engine_Command_Result
function Engine_Map_Object:move(direction, pixels, skip, change_facing) end

---@param new_opacity number
---@param duration integer
---@return Engine_Command_Result
function Engine_Map_Object:update_opacity(new_opacity, duration) end

---@param color Engine_Color
---@param duration integer
---@return Engine_Command_Result
function Engine_Map_Object:update_color(color, duration) end

---@overload fun(position : Engine_Vec2, keep_trying? : boolean) : Engine_Command_Result
---@param x number
---@param y number
---@param keep_trying? boolean
---@return Engine_Command_Result
function Engine_Map_Object:move_to(x, y, keep_trying) end

---@param object Engine_Map_Object
function Engine_Map_Object:add_linked_object(object) end

---@param object Engine_Map_Object
function Engine_Map_Object:remove_linked_object(object) end

---@param name string
---@return string
function Engine_Map_Object:get_property(name) end

---@param name string
---@param value string
function Engine_Map_Object:set_property(name, value) end

function Engine_Map_Object:run_script() end

function Engine_Map_Object:run_trigger_script() end

function Engine_Map_Object:run_touch_script() end

function Engine_Map_Object:run_leave_script() end

---@type Engine_Map_Object
---@diagnostic disable-next-line: missing-fields
player = {}

-- Layer

---@class (exact) Engine_Layer
---@field visible boolean
---@field opacity number
local Engine_Layer = {}

---@param new_opacity number
---@param duration integer
---@return Engine_Command_Result
function Engine_Layer:update_opacity(new_opacity, duration) end

---@param name string
---@return string
function Engine_Layer:get_property(name) end

---@param name string
---@param value string
function Engine_Layer:set_property(name, value) end

-- Image layer

---@class (exact) Engine_Image_Layer : Engine_Layer
---@field velocity Engine_Vec2
---@field sprite string
---@field color Engine_Color
local Engine_Image_Layer = {}

---@param pose string
---@param state? string
---@param direction? string
---@return Engine_Command_Result
function Engine_Image_Layer:show_pose(pose, state, direction) end

---@param filename string
---@param pose? string
function Engine_Image_Layer:set_sprite(filename, pose) end

function Engine_Image_Layer:reset() end

---@param new_color Engine_Color
---@param duration integer
---@return Engine_Command_Result
function Engine_Image_Layer:update_color(new_color, duration) end

---@param new_velocity Engine_Vec2
---@param duration integer
---@return Engine_Command_Result
function Engine_Image_Layer:update_velocity(new_velocity, duration) end

-- Object Layer

---@class (exact) Engine_Object_Layer : Engine_Layer
---@field tint_color Engine_Color
---@field objects Engine_Map_Object[]
local Engine_Object_Layer = {}

---@param new_color Engine_Color
---@param duration integer
---@return Engine_Command_Result
function Engine_Object_Layer:update_color(new_color, duration) end

-- Map

---@class (exact) Engine_Map
---@field data table
---@field name string # from TMX file
---@field filename string # full filename including folders
---@field filename_stem string # last part of filename without the extension
---@field width integer
---@field height integer
---@field tile_height integer
---@field tile_width integer
---@field draw_outlines boolean
---@field objects table<integer, Engine_Map_Object> # Object ID to Map_Object
---@field object_count integer
---@field layer_count integer
---@field script_scheduler_paused boolean
current_map = {}

---@overload fun(self : Engine_Map, id : integer) : Engine_Map_Object?
---@param name string
---@return Engine_Map_Object?
function current_map:get_object(name) end

---@param name? string
---@param sprite? string
---@param position? Engine_Vec2
---@param direction? Engine_Direction
---@param layer? Engine_Layer
---@return Engine_Map_Object
function current_map:add_new_object(name, sprite, position, direction, layer) end

---@param object Engine_Map_Object
function current_map:delete_object(object) end

---@overload fun(self : Engine_Map, id : integer) : Engine_Layer
---@param name string
---@return Engine_Layer
function current_map:get_layer(name) end

---@overload fun(self : Engine_Map, id : integer) : Engine_Layer
---@param name string
---@return Engine_Object_Layer
function current_map:get_object_layer(name) end

---@overload fun(self : Engine_Map, id : integer) : Engine_Layer
---@param name string
---@return Engine_Image_Layer
function current_map:get_image_layer(name) end

---@param object Engine_Map_Object
---@param direction Engine_Direction
---@return boolean
function current_map:passable(object, direction) end

---@param object Engine_Map_Object
---@return Engine_Map_Object
function current_map:colliding_object(object) end

---@param name string
---@return string
function current_map:get_property(name) end

---@param name string
---@param value string
function current_map:set_property(name, value) end

---@param func function
function current_map:run_function(func) end

---@param script string
function current_map:run_script(script) end

---@param filename string
function current_map:run_script_file(filename) end

-- Camera

---@class (exact) Engine_Camera
---@field position Engine_Vec2
---@field position_bounds Engine_Rect
---@field screen_tint Engine_Color # Drawn above everything
---@field map_tint Engine_Color # Drawn above map/objects but under text/canvases
---@field tracked_object Engine_Map_Object
---@field is_shaking boolean
camera = {}

---@param direction Engine_Direction
---@param pixels number
---@param speed number
---@return Engine_Command_Result
function camera:move(direction, pixels, speed) end

---@overload fun(self : Engine_Camera, object : Engine_Map_Object, speed : number) : Engine_Command_Result
---@overload fun(self : Engine_Camera, position : Engine_Vec2, speed : number) : Engine_Command_Result
---@param x number
---@param y number
---@param speed number
---@return Engine_Command_Result
function camera:move_to(x, y, speed) end

---@overload fun(self : Engine_Camera, object : Engine_Map_Object)
---@overload fun(self : Engine_Camera, position : Engine_Vec2)
---@param x number
---@param y number
function camera:center_at(x, y) end

---@param object Engine_Map_Object
function camera:track_object(object) end

function camera:stop_tracking() end

---@param color Engine_Color|string
---@param duration integer
---@return Engine_Command_Result
function camera:tint_map(color, duration) end

---@param color Engine_Color|string
---@param duration integer
---@return Engine_Command_Result
function camera:tint_screen(color, duration) end

---@param pos_or_object Engine_Vec2|Engine_Map_Object
---@return Engine_Vec2
function camera:get_centered_position(pos_or_object) end

---@overload fun(self : Engine_Camera, strength : Engine_Vec2, speed : Engine_Vec2, duration : integer) : Engine_Command_Result
---@param strength number
---@param speed number
---@param duration integer
---@return Engine_Command_Result
function camera:shake_screen(strength, speed, duration) end

---@overload fun(self : Engine_Camera, strength : Engine_Vec2, speed : Engine_Vec2)
---@param strength number
---@param speed number
function camera:start_shaking(strength, speed) end

function camera:cease_shaking() end

---@param scale number
---@param duration integer
---@return Engine_Command_Result
function camera:zoom(scale, duration) end

-- Music

---@class (exact) Engine_Music
---@field playing boolean
---@field paused boolean
---@field stopped boolean
---@field offset number
---@field volume number # between 0 and 1
---@field pitch number # defaults to 1.0
---@field looping boolean
---@field filename string
local Engine_Music = {}

---@param target_volume number
---@param duration integer
---@return Engine_Command_Result
function Engine_Music:fade(target_volume, duration) end

function Engine_Music:play() end

function Engine_Music:pause() end

function Engine_Music:stop() end

---@param loop_start number
---@param loop_end number
function Engine_Music:set_loop_points(loop_start, loop_end) end

--- Sound

---@class (exact) Engine_Sound
---@field playing boolean
---@field paused boolean
---@field stopped boolean
---@field offset number
---@field volume number # between 0 and 1
---@field pitch number # defaults to 1.0
---@field looping boolean
---@field filename string
local Engine_Sound = {}

function Engine_Sound:play() end

function Engine_Sound:pause() end

function Engine_Sound:stop() end

---@param loop_start number
---@param loop_end number
function Engine_Sound:set_loop_points(loop_start, loop_end) end

---@param filename string
---@param pausable? boolean
---@return Engine_Sound
function Sound(filename, pausable) end

-- Audio_Player

---@class (exact) Engine_Audio_Player
---@field playing_music Engine_Music?
---@field playing_ambient Engine_Music?
---@field global_music_volume number
---@field global_sound_volume number
local Engine_Audio_Player = {}

---@param filename string
---@param channel_count? integer # defaults to 1
---@param pausable? boolean # defaults to true
---@return Engine_Sound? # nil if filename is '' or channel_count is 0
function Engine_Audio_Player:load_global_sound(filename, channel_count, pausable) end

---@param config_name string
---@param channel_count? integer # defaults to 1
---@param pausable? boolean # defaults to true
---@return Engine_Sound? # nil if config has no value or channel_count is 0
function Engine_Audio_Player:load_global_config_sound(config_name, channel_count, pausable) end

---@param filename string
---@param channel_count? integer # defaults to 1
---@param pausable? boolean # defaults to true
---@return Engine_Sound? # nil if filename is '' or channel_count is 0
function Engine_Audio_Player:load_map_sound(filename, channel_count, pausable) end

---@param config_name string
---@param channel_count? integer # defaults to 1
---@param pausable? boolean # defaults to true
---@return Engine_Sound? # nil if config has no value or channel_count is 0
function Engine_Audio_Player:load_map_config_sound(config_name, channel_count, pausable) end

---@param filename string
---@return Engine_Music? # nil if filename is ''
function Engine_Audio_Player:load_music(filename) end

---@overload fun(self : Engine_Audio_Player, music : Engine_Music, looping? : boolean) : Engine_Music?
---@overload fun(self : Engine_Audio_Player, filename : string, volume : number) : Engine_Music?
---@param filename string
---@param looping? boolean # defaults to true
---@param volume? number # defaults to 1
---@return Engine_Music? # nil if filename is '' or 'false'
function Engine_Audio_Player:play_music(filename, looping, volume) end

---@overload fun(self : Engine_Audio_Player, music : Engine_Music, looping? : boolean) : Engine_Music?
---@overload fun(self : Engine_Audio_Player, filename : string, volume : number) : Engine_Music?
---@param filename string
---@param looping? boolean # defaults to true
---@param volume? number # defaults to 1
---@return Engine_Music? # nil if filename is '' or 'false'
function Engine_Audio_Player:play_ambient(filename, looping, volume) end

---@param filename string
---@param pitch? number
---@param volume? number
---@return Engine_Sound? # nil if filename is ''
function Engine_Audio_Player:play_sound(filename, pitch, volume) end

---@param config_name string
---@param pitch? number
---@param volume? number
---@return Engine_Sound? # nil if config has no value
function Engine_Audio_Player:play_config_sound(config_name, pitch, volume) end

-- Environment

---@class (exact) Engine_Environment
---@field name string # readonly
local Engine_Environment = {}

---@param mode? Engine_Open_Page_Mode
---@return boolean success
function Engine_Environment:can_open_store_page(mode) end

---@param mode? Engine_Open_Page_Mode
---@return boolean success
function Engine_Environment:open_store_page(mode) end

---@param mode? Engine_Open_Page_Mode
---@return boolean success
function Engine_Environment:can_open_url(mode) end

---@param url string
---@param mode? Engine_Open_Page_Mode
---@return boolean success
function Engine_Environment:open_url(url, mode) end

-- Game

---@class (exact) Engine_Game
---@field ticks integer # readonly
---@field window_ticks integer # readonly
---@field seconds integer # readonly
---@field fps integer # readonly
---@field frame_count integer # readonly
---@field character_input string # readonly
---@field triggered_keys string[] # readonly
---@field gamepad_enabled boolean # readonly
---@field gamepad_names table<string, string> # readonly
---@field gamepad_name string # readonly
---@field last_input_type Engine_Input_Type
---@field game_width integer # readonly
---@field game_height integer # readonly
---@field width integer # readonly
---@field height integer # readonly
---@field framebuffer_width integer # readonly
---@field framebuffer_height integer # readonly
---@field monitor_resolution Engine_Vec2 # readonly
---@field monitor_resolutions Engine_Vec2[] # readonly
---@field paused boolean # readonly
---@field pausing_enabled boolean
---@field stopped boolean # readonly
---@field script_scheduler_paused boolean
---@field debug boolean # readonly
---@field command_line_args string[] # readonly
---@field fullscreen boolean
---@field magnification integer
---@field audio_player Engine_Audio_Player # readonly
---@field environment Engine_Environment # readonly
---@field game_data_filesystem Engine_Readable_Filesystem # readonly
---@field user_data_filesystem Engine_Writable_Filesystem # readonly
---@field data_folder Engine_User_Data_Folder # readonly
game = {}

---@param name string
---@return string
function game:get_config(name) end

---@param name string
---@return string
function game:get_string_config(name) end

---@param name string
---@return boolean
function game:get_bool_config(name) end

---@param name string
---@return number
function game:get_float_config(name) end

---@param name string
---@return integer
function game:get_int_config(name) end

---@param name string
---@return integer
function game:get_unsigned_config(name) end

---@param name string
---@param value string
function game:set_string_config(name, value) end

---@param name string
---@param value boolean
function game:set_bool_config(name, value) end

---@param name string
---@param value number
function game:set_float_config(name, value) end

---@param name string
---@param value integer
function game:set_int_config(name, value) end

---@param name string
---@param value integer
function game:set_unsigned_config(name, value) end

---@overload fun(self : Engine_Game, filename : string, direction : Engine_Direction?, music_file : string?)
---@overload fun(self : Engine_Game, filename : string, position : Engine_Vec2, direction : Engine_Direction, music_file : string?)
---@param filename string
---@param x number
---@param y number
---@param direction Engine_Direction
---@param music_file string?
function game:load_map(filename, x, y, direction, music_file) end

function game:pause() end

---@param script? string
function game:resume(script) end

function game:stop_time() end

function game:resume_time() end

---@param key string
---@return boolean
function game:triggered(key) end

---@param key string
---@return boolean
function game:triggered_once(key) end

---@param key string
---@return boolean
function game:pressed(key) end

function game:begin_character_input() end

function game:end_character_input() end

---@param logical_key_name string
---@return string[] physical_key_names
function game:get_bound_keys(logical_key_name) end

---@param physical_key_name string
function game:unbind_physical_key(physical_key_name) end

---@param logical_key_name string
function game:unbind_virtual_key(logical_key_name) end

---@param physical_key_name string
---@param logical_key_name string
function game:bind_key(physical_key_name, logical_key_name) end

---@param physical_key_name string
---@return string printable_name
function game:get_key_name(physical_key_name) end

---@param key? string
function game:wait_for_input(key) end

---@param text string
---@return number
function game:text_width(text) end

---@param func function
function game:run_function(func) end

---@param script string
function game:run_script(script) end

---@param filename string
function game:run_script_file(filename) end

---@param filename string
---@return function
function game:load_lua_file(filename) end

function game:exit() end

function game:reset_scripting() end

---@param width integer
---@param height integer
function game:set_size(width, height) end

---@param url string
---@return boolean
function game:open_url(url) end
