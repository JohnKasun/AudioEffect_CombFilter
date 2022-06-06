add_library(Utility 
    INTERFACE 
        include/Util.h 
        include/ErrorDef.h
        include/Vector.h
        include/Synthesis.h
)

target_include_directories(Utility INTERFACE include/)