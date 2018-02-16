local function tokenstream(path)
	local f = assert(io.open(path))
	local str = f:read('*a')
	f:close()
	local pos = 1
	return function()
		if pos < #str do
			local byte = str.byte(pos)
			if
				byte >= string.byte('a') and byte <= string.byte('z')
			then
			end
		end
	end
end
