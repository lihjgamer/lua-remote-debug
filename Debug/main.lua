print "ok"

local luadebug = require("pipe");
local mt = {__gc = function()  luadebug.stop(); end};
ldebug = {}
setmetatable(ldebug,mt);
local resutl ,info = luadebug.start("luadebug");
print(resutl,info);


local function test(x,y)
	
	local lkey = {};
	local ttable = 
	{
		["abc"] = {1,2,"def",["test"]={1,2}},
		[1] = 1,
		[2] = 2,
		[3] = 3,
		[lkey] = "lkey"
	};
	
	local z = x + y;
	local z1= x * y;
	local z2= x * x;
	local z3= y * y;
	
	return z*z1 + z2*z3;

end

function update()
	local z = test(2,1);
	local z1 = test(2,2);
	
	test(z,z1)
end