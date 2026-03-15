--!native
--!optimize 2
local x = 640
local y = 576
print("Printed via print!")
RayLuau.Print("Printed via RayLuau.Print!")

local buf = buffer.create(640*576*4)

local function drawGraphic(b: buffer)
	local c = 0
	for iY = 1, y do
		local g = math.abs(math.sin((iY / y) + os.clock()))
		g = (g * 255) // 1
		for iX = 1, x do
			local r = math.abs(math.sin((iX / x) + os.clock()))
			r = (r * 255) // 1
			buffer.writeu32(b, c, 0xFF000000 + bit32.lshift(g, 8) + r)
			c += 4
		end
	end
end

local a, b = 3, 7
print("Going to multiply", a, "by", b)
print(RayLuau.Multiply(a, b))

RayLuau.Loop(function()
	drawGraphic(buf)
	RayLuau.DrawFramebuffer(buf)
end)
