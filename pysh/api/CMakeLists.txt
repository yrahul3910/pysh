execute_process(COMMAND python3 -m pip install -r requirements.txt)
execute_process(COMMAND python3 -m pip install -e . WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})