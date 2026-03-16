--!strict
--!native
local x: number = 640
local y: number = 576
print("Printed via print!")
RayLuau.Print("Printed via RayLuau.Print!")

local buf: buffer = buffer.create(640*576*4)

local function drawGraphic(b: buffer)
	local c: number = 0
	for iY: number = 1, y do
		local g: number = math.abs(math.sin((iY / y) + os.clock()))
		g = (g * 255) // 1
		for iX: number = 1, x do
			local r: number = math.abs(math.sin((iX / x) + os.clock()))
			r = (r * 255) // 1
			buffer.writeu32(b, c, 0xFF000000 + bit32.lshift(g, 8) + r)
			c += 4
		end
	end
end

local a: number, b: number = 3, 7
print("Going to multiply", a, "by", b)
print(RayLuau.Multiply(a, b))

RayLuau.Loop(function()
	drawGraphic(buf)
	RayLuau.DrawFramebuffer(buf)
end)
