
idf_component_register(
                       SRCS "pumps.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES tinyfsm
                       PRIV_REQUIRES driver bsp config
                       )
