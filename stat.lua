local doc = [[Usage: run stat.lua < file    
    -- report summary statistics of each line
    -- any blank lines will be skipped]]

if select('#', ...) > 0 then print(doc); return end

local a, sum, NR = {}, 0, 0
local tonumber = tonumber

for x in io.lines() do
    x = tonumber(x)
    if x then
        sum = sum + x
        NR = NR + 1
        a[NR] = x
    end
end

print("Items", NR)
if NR==0 then return end
local mu = sum / NR
print("Average", mu)
if NR==1 then return end

local s2, n = 0, NR-1
for i = 1, NR do local x=a[i]-mu; s2 = s2 + x*x end
print("Std Dev", math.sqrt(s2/n), "\n-------")

function index(a, i)
    local idx = math.floor(i)
    local ai = a[idx]
    if (i == idx) then return ai end        -- return a[i]
    return ai + (i-idx) * (a[idx+1] - ai)   -- interpolate for a[i]
end
table.sort(a)

print("Min", a[1])
print("Q1 ", index(a, 1 + 0.25*n))
print("Med", index(a, 1 + 0.50*n))
print("Q3 ", index(a, 1 + 0.75*n))
print("Max", a[NR])
