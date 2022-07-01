add_library(smalljson SHARED smalljson.cc)

set_target_properties(smalljson 
    PROPERTIES
    PUBLIC_HEADER smalljson.h
    OUTPUT_NAME "smalljson"
    )

install(TARGETS smalljson 
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include/smalljson
    )