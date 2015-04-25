#Qt Creator Google Test Integration

##Introduction
Plugin integrates some [Google Test](https://code.google.com/p/googletest/ "Google T") functionality into Qt Creator IDE.

##Features
* Parse test runner's output and show tests' results in pane
* Launches tests from active project
* Translation support

##Usage
###Parsing tests output
1. Write google test application.
2. Uncheck `launch in terminal` option in project's run configuration.
3. Run application. Testing results will be represented in `Google Test` pane.

###Running tests
1. Write google test application.
2. In `Tools->Google Test` menu select appropriate entry.
3. Depending on chosen menu entry, plugin will generate run arguments for active project and launch it.

>Note: Tests from changed files or current file will be run only if they belong to active project.

##Downloads
Built plugin can be downloaded [here](https://sourceforge.net/projects/qtc-gtest/files/bin/ "Sourceforge")
