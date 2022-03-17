## TODO

### RISC-V

- use compressed instructions
  - reduce code size
  - better compatibility with compiler, libs, etc.
- add tests for each instruction

### VM

- implement settings ui
- loading improvement (ticker?)

### MISC

- tests over BLE
- out of band `io_exchange()` which reply to special APDUs such as `APDU_IS_APP_RUNNING` or `APDU_QUIT_APP`
- better clients
  - weird "plugins" for now
