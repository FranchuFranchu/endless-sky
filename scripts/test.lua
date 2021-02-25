ember_waste_systems = {}

for k, v in pairs(endless_sky.systems) do
	if v.attributes:has("ember waste") then
		ember_waste_systems[#ember_waste_systems+1] = k
	end
end

print(table.concat(ember_waste_systems, ", "))
