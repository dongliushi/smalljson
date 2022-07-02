add_library(smalljson SHARED smalljson.cc)

install(TARGETS smalljson 
    LIBRARY DESTINATION lib
    )

install(FILES smalljson.h DESTINATION include/smalljson)