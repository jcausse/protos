# Final Assignment for Communication Protocols class
June, 2024.

* **61105** - Causse, Juan Ignacio

* **61590** - De Caro, Guido

* **62774** - Mindlin, Felipe

* **62351** - Sendot, Francisco

* **61374** - Garcia Lauberer, Federico Inti

## Instalation Guide

1. Step 1
```bash:
Run 'build.sh' to compile and create all necessary executable files
(smtpd.bin and manager.bin)
```
## Usage
SMTPD:
1. Step 1
   In 'main.c', change the default smtpd log file in the defined constant CONFIG_LOG_FILE.

2. Step 2
   Run the smtpd.bin executable file with the following parameters:
   
MANAGER:
1. Step 1
   Run the manager.bin executable file with the following parameters:

## Command Line Parameters:

SMTPD:

   -d <domain name>: Domain name for the server.
   -s <SMTP port>: Port for the SMTP server.
   -p <management port>: Port for the manager server.
      
   Optional:
   
   -L <log level>: Minimum log level.
   -t <command path>: The transformation command to use.
   -f <vrfy dir>: The directory where already verified email addresses are stored and where new ones          will be saved.
   -v: Prints version information and exits.
   -h: Prints available flags with their pertinent information.


MANAGER: 

   -i <SMTP server IP>: IP for the SMTP server.
   -p <SMTP server port>: SMTP server port.

   Optional:
   
   -v: Prints version information and exits.
   -h: Prints available flags with their pertinent information.



   
