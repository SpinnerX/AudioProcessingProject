__     _____ ____  _____ ___    ____  _____ ____ ___  ____  ____  _____ ____  
\ \   / /_ _|  _ \| ____/ _ \  |  _ \| ____/ ___/ _ \|  _ \|  _ \| ____|  _ \ 
 \ \ / / | || | | |  _|| | | | | |_) |  _|| |  | | | | |_) | | | |  _| | |_) |
  \ V /  | || |_| | |__| |_| | |  _ <| |__| |__| |_| |  _ <| |_| | |___|  _ < 
   \_/  |___|____/|_____\___/  |_| \_\_____\____\___/|_| \_\____/|_____|_| \_\
                                                                              
 ____  ____   ___      _ _____ ____ _____ 
|  _ \|  _ \ / _ \    | | ____/ ___|_   _|
| |_) | |_) | | | |_  | |  _|| |     | |  
|  __/|  _ <| |_| | |_| | |__| |___  | |  
|_|   |_| \_\\___/ \___/|_____\____| |_|  
                                          

Description

This is a project that uses ffmpeg and opengl that implements a video recorder

Phase #1 (Getting OpenGL to work properly)

- Using opengl to render the pixel color red
- How we render the color red is iterating through 100x100 of 3 byte pixels
Example
unsigned char* data = new unsigned char[100 * 100 * 3];
    for(int y = 0; y < 100; y++){
        for(int x = 0; x < 100; x++){
            data[y * 100 * 3 + x * 3] = 0xff; 
            data[y * 100 * 3 + x * 3 + 1] = 0x00;
            data[y * 100 * 3 + x * 3 + 2] = 0x00;
        }
    }

Where y is the coordinate and we multiply by the size and how much pixels we want to move by and add the x coordinate and then multiply by how many bits we want to add further, example is when we do +1 and +2.

OpenGL functions purpose

- In OpenGL, you have to always clear the buffer before rendering any sort of pixels onto the screen
glClear()


glDrawPixels(w, h, rgb, byte, const char*)