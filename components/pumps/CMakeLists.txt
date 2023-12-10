
idf_component_register(
                       SRCS "pumps.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES tinyfsm bsp
                       PRIV_REQUIRES driver config
                       )
