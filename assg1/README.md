#  Assignment 1

---
- Compile using g++ main.cpp  and then execute using ./a.out
- On executing the filr explorer opens in normal mode
- The application is executed in non-canonical mode and on exit settings are restored
- To exit press `Ctrl+c` , on doing so gracefull exit is done by clearing terminal and restoring all configurations

## Normal Mode

- All the files are displayed in tabs as file permissions, size ,date and filename
- The files are displayed in ascending order except last 2 lines for error message if any and status bar
- k and l command moves up  or down the file system by page up or down in case of overflow
- All files are opened in vi except pdf which is opened in  default interface using xdg-open


## Command Mode
- All the paths are considered with respect to the home i.e. in our case the place from where the application is started.
- In case of any invalid command or invalid command structure appropriate messages are provided

