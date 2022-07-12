 <b>Project One: File Transfer Protocol-Client-Server</b>

 <b>Done by: Mohammed Arham (maj596)</b>

The program was written and compiled on Catalina Mac OS using GCC compiler. <b></b>

To establish connections between the server and clients, this application employs sockets. Multiple clients can connect to and communicate with the server using the select () function. Once clients have connected, they must authenticate to execute any more commands. Currently, the authentication procedure uses arrays of usernames and passwords that are kept in a separate .txt file. Users don't need to authenticate again after the initial process is finished. 

Another characteristic of this program is data transfer. A new TCP connection is created when clients need to upload or download data to or from the server. pthread() is used to manage these processes so that several data transfers can take place concurrently. Other users can continue access the server while one client waits for a large file to be sent.

Since the server is being used by several clients, it is essential that when one client changes the server directory, others are not impacted. To handle this, the server will record the path for each client whenever they modify the server directory. The server will utilize that recorded path for that client whenever the client needs to transfer data or execute LIST or PWD commands. As a result, each client has a unique server access directory.

<b>Important</b>

Always login first to the FTP client to run commands, use the command ‘USER user’ and then ‘PASS pass’

To add username, please add to the user.txt file. 

Deletion of users.txt will lead to segmentation error when running serverFTP.exe as users.txt is read in the code

 <b>Instructions To Run the Program</b>

Download the zip file.

Use a terminal to change directory to the folder with all the files.

Run ‘make’ in terminal.

Then make a new folder called ‘client’ or whatever name you choose inside the current directory and copy paste the .exe file of the client to that folder. Let the server.exe file be in the main directory.

Now open a new terminal window, and then change directory to the folder. And in this terminal run ‘./serverFTP’

Now open another terminal window, and then change directory to the client folder or whatever you named it, and then type in ‘./clientFTP’

Now you should have two terminals one running the server, another running the client. You can create multiple clients by repeating the process with creating new terminal windows.


