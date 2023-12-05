
idf_component_register(
                       SRCS "temperatures.cpp"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES bsp onewire_bus ds18b20
                       )
