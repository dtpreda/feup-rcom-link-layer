# Data Link Layer Protocol and File Transfer Application
As proposed by the 2021/2022 edition of the RC course @FEUP, this project implements a communication protocol for the `RS-232` device driver and a file transfer application which acts upon the same device.

# Usage

By using the provided Makefile, a `application` binary is obtained as a result of the compilation of the project. We can then execute it with the following input:

    For receiving: ./app -p <port_number> -r <path_for_new_file> [-n <new_file_name>]
    For transmitting: ./app -p <port_number> -s <path_of_file>

A pinguim.gif file is also included, as a simple for users to test the application developed.

# Credits

Kudos to @bdmendes and @Sirze01 for providing the Makefile for this project. You can find their version of the project at https://github.com/Sirze01/feup-rc-data-link
