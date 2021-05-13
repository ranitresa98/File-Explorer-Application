#  Assignment 1

---
- Compile using g++ main.cpp  and then execute using ./a.out
- On executing the filr explorer opens in normal mode
- The application is executed in non-canonical mode and on exit settings are restored
- To exit press `Ctrl+c` , on doing so gracefull exit is done by clearing terminal and restoring all configurations

## Normal Mode

- All the files are displayed in tabs as file permissions, size ,date and filename
-

## Command Mode
- All the paths are considered with respect to the home i.e. in our case the place from where the application is started.
- In case of any invalid command or invalid command structure appropriate messages are provided


# About the project-
----------------------------------
Building a fully functional file explorer application, albeit with restricted features. The application supports two modes:

- NORMAL MODE
- COMMAND MODE



## Normal Mode-
- Displays list of directories and files present in current folder.
- Informations displayed: file name, file size, ownership, last modified (in human readable format)
- The files are displayed in ascending order except last 2 lines for error message if any and status bar
### Key presses and their functionalities:
- ENTER KEY: When user presses it, the file/directory the cursor is at, opens. All files are opened in vi except pdf which is opened in  default interface using xdg-open
- LEFT ARROW KEY: Goes back to the previously visited directory
- RIGHT ARROW KEY: Goes to next directory
- BACKSPACE KEY: Takes user up to one level
- HOME KEY: Takes user back to root directory
- UP and DOWN keys: used for moving cursor up and down to desired file or directory
- k and l : moves up  or down the file system by page up or down in case of overflow

## Command Mode-
This mode is entered from Normal mode whenever : is pressed.

### Following commands are supported:
- COPY: copy <file_name(s)> <target_directory_path>
- MOVE: move <file_name(s)> <target_directory_path>
- RENAME: rename <old_file_name> <new_file_name>
- CREATE FILE: create_file <file_name> <destination_path>
- CREATE DIRECTORY: create_dir <diectory_name> <destination_path>
- DELETE FILE: delete_file <file_path>
- DELETE DIRECTORY: delete_dir <directory_path>
- SEARCH: search <file_name / directory_name>
- GOTO: goto <directory_path> Pressing ESC KEY takes user back to Normal Mode.

## How to execute the program-
- Open the terminal with root of the application folder as the present working directory
- Compile the cpp file: g++ main.cpp -o main
- Run the executable file: ./main

## Language used-
C++
