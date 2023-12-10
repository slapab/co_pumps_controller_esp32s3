
idf_component_register(
                       SRCS "temperatures.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES onewire_bus ds18b20
                       PRIV_REQUIRES bsp config controller
                       )
