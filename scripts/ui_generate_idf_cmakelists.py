#!/usr/bin/env python

parent_path = "../components/ui"
filelist_path = f"{parent_path}/filelist.txt"
cmakelists_path = f"{parent_path}/CMakeLists.txt"

# Read files from filelist.txt
with open(filelist_path, "r") as filelist:
    source_files = filelist.read().splitlines()

# Create content of CMakeLists.txt
# - create the UI_SOURCES variable
cmakelists_content = f"SET(UI_SOURCES\n"
for source_file in source_files:
    cmakelists_content += f"    {source_file}\n"
cmakelists_content += f")\n\n"

# - add remaining CMakeLists.txt content
cmakelists_content += \
"""
idf_component_register(
    SRCS ${UI_SOURCES}
    INCLUDE_DIRS \".\"
    REQUIRES
    PRIV_REQUIRES lvgl
    )
"""

# Save CMakeLists.txt
with open(cmakelists_path, "w") as cmakelists:
    cmakelists.write(cmakelists_content)

print("File CMakeLists.txt has been created")
