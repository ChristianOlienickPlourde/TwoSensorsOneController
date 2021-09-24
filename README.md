# TwoSensorsOneController
The exercise outlined below mimics two sensors generating messages and a host controller processing the messages. This creates a system with 3 objects that are acting independently. The two sensors write to one text file, implying both sensors write to the same file. The host controller will monitor/read from this file and process the messages. In this exercise the text file acts as an input buffer to the host controller.

1. The solution shall be written in C/C++

2. The solution shall reside in one executable

3. Each sensor and the host controller objects should be independent threads

4. The application shall run for 1 minute

5. Each sensor will write a line to the text file every 50 milliseconds in the format outlined below

6. 10% of the time a sensor will write a line to the text file that breaks the rules of the format below (emulating corrupted data) in various methods

7. The host controller reads from the file every 125 milliseconds

8. Each time the host controller reads the file it removes any invalid messages from the file

9. The host controller will continually report what percentage of invalid lines were found

10. At the end of the runtime the file should only contain valid lines

File Format

1. Each line contains two fields separated by a ";";

2. The first field is an decimal value ranging from 1 to 200.00;

3. The second field is a valid disk path pointing to a generic file; (Linux OS)

4. The symbol "%" may appear anywhere in a line indicating the beginning of a comment.
