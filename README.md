# raspipe
Here is the set of programs I wrote to stream audio to my Raspberry Pi.
The same functionality can easily be achieved by netcat (or lots of other means).
However, these are my experiments to learn network programming,
forking processes and more system calls in UNIX-like systems.

The server runs on the device that is connected to speakers.
Whenever a client establishes a connection, the server spawns an instance of aplay,
and pipes the PCM audio received from the client to it.
