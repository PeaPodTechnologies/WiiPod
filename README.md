# WiiPod

I2CIP Nintendo Nunchuck + Nokia 5110 graphics on FSM

Note to self: make all `./lib/I2CIP_*` source files start all `#define` macro and `typedef` type names with `I2CIP_*`/`i2cip_*_t` to avoid conflicts with other libraries.

<!-- Note to self: should class declarations be namespaced inside `I2CIP::`? -->

TODO: Finish Guarantees for pointer dereferincing (uint32_t match between virtual member function and static template function)
TODO: DebugJson When? (Affects ALL Submodules) virtual cacheToString(G, Json) for InputInterfaces?
Major Changelog: DeviceGroup, Module -> module.h, template Module::<> member functions for device operations i.e. getInput->printCache()

Note: K30 needs hot-swap buffer (i.e. TCA9548A) to fix subnet glitching

Note: Nunchuck glitching is still TODO

Note: At init, MCP Bank A is configured to be output (1), and Bank B is configured to be input (0)

Note: I2CIP setbus/resetbus = false; should be the default in some (most?) cases

TODO: Re-implement MCP23017/MCP23017_Pin more like Seesaw_RotaryEncoder; Implement MCP23017_Byte, reuse MCP23017 for MCP23017_Word