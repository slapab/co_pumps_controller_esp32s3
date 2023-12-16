#!/usr/bin/env python

import glob
import os

parent_path = "../components/ui"
os.chdir(parent_path)

filelist_path = f"./filelist.txt"
cmakelists_path = f"./CMakeLists.txt"
my_ui_events_dir_path = f"./my_ui_events_actions"

# Read files from filelist.txt
with open(filelist_path, "r") as filelist:
    source_files = filelist.read().splitlines()

# Create content of CMakeLists.txt
# - create the UI_SOURCES variable
cmakelists_content = f"SET(UI_SOURCES\n"
for source_file in source_files:
    cmakelists_content += f"    {source_file}\n"

# Add all .c files from the my_ui_events dir
my_ui_events_files_list = glob.glob(f'{my_ui_events_dir_path}/*.c') + glob.glob(f'{my_ui_events_dir_path}/*.cpp')
cmakelists_content += '\n'.join([f'    {file}' for file in my_ui_events_files_list])

cmakelists_content += f")\n\n"

# - add remaining CMakeLists.txt content
cmakelists_content += \
"""
idf_component_register(
    SRCS ${UI_SOURCES}
    INCLUDE_DIRS
        \".\"
        \"components\"
        \"my_ui_events_actions/include\"
    REQUIRES
    PRIV_REQUIRES
        lvgl
        controller
        config
        display
    )
"""

# Save CMakeLists.txt
with open(cmakelists_path, "w") as cmakelists:
    cmakelists.write(cmakelists_content)

print("File CMakeLists.txt has been created")
