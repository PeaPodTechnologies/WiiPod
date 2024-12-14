# WiiPod
 I2CIP Nintendo Nunchuck + Nokia 5110 graphics on FSM

 Note to self: make all `./lib/I2CIP_*` source files start all `#define` macro and `typedef` type names with `I2CIP_*`/`i2cip_*_t` to avoid conflicts with other libraries.

 <!-- Note to self: should class declarations be namespaced inside `I2CIP::`? -->

 Note: K30 needs hot-swap buffer (i.e. TCA9548A) to fix subnet glitching

 Note: Nunchuck glitching is still TODO

 Note: At init, MCP Bank A is configured to be output (1), and Bank B is configured to be input (0)

 Note: I2CIP setbus/resetbus = false; should be the default in some (most?) cases