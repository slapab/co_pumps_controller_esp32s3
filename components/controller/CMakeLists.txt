
idf_component_register(
                       SRCS "controller.cpp"
                       INCLUDE_DIRS "."
                       PRIV_INCLUDE_DIRS "priv_include"
                       REQUIRES tinyfsm temperatures pumps
                       PRIV_REQUIRES config display
                       )
