--!strict
--!native
require('@self/test1')
require('@self/testDir')
require('@self/testDir/test2')
require('./scripts/sandboxcheck') -- Should only return if RayLuau's sandbox setting is false.
print("Require: Sandbox is not being enacted.")