add_library(sdk
        # not required if linked against -lgcc
        # math.c
        sdk.c
        start.S
        std.c)
