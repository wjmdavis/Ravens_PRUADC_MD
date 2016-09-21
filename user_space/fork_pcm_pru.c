 // create_sinusoidal_pcm_file.c
 // Copyright 2016 Greg
 // Generate sinusoidal PCM data and write to a named pipe.
 // Play the data from the pipe using ALSA aplay via a
 // fork and execvp.

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
//	int8_t makefifo;
	int soundfifo, pru_data, pru_clock; // file descriptors
	pid_t forkit;
	//  Open a file in write mode.
        uint8_t sinebuf[490];
	
	//  Create a named pipe.
//	makefifo = mkfifo("soundfifo", O_RDWR);
//	if(makefifo < 0) printf("Named pipe soundfifo creation failed.");
	
	//  The sample rate is 44100 samples per second.
	//  Compute delta-t, a single sample interval.

        //  Split into two processes.
        //  The parent will consume the data.
        //  The child will generate data and write to a named pipe.
	forkit = fork();
	
	if(!forkit) {  //  This is the child process.
        int result;
        char *arguments[] = {"aplay", "--format=S16_LE", "-Dplughw:1,0", "--rate=8000", "soundfifo", NULL};
        result = execvp("aplay", arguments);
        printf("The return value of execvp(aplay) is %d.\n", result);
	}
	else if (forkit > 0) {  //  This is the parent process.
	
        //  Now, open the PRU character device.
        //  Read data from it in chunks and write to the named pipe.
        ssize_t readpru, writepipe, prime_char, pru_clock_command;
	soundfifo = open("soundfifo", O_RDWR);  // Open named pipe.
        if(soundfifo < 0) printf("Failed to open soundfifo.");
	pru_data  = open("/dev/rpmsg_pru30", O_RDWR);  // Open char device.
        if(pru_data < 0) printf("Failed to open pru character device rpmsg_pru30.");
        //  The character device must be "primed".
        prime_char = write(pru_data, "prime", 6);
        if(prime_char < 0) printf("Failed to prime the PRU0 char device.");
        //  Now open the PRU1 clock control char device and start the clock.
        pru_clock = open("/dev/rpmsg_pru31", O_RDWR);
        pru_clock_command = write(pru_clock, "g", 2);
        if(pru_clock_command < 0) printf("The pru clock start command failed.");
        //  This is the main data transfer loop.
        for(int i=0; i < 20000000; i++) {
                readpru = read(pru_data, sinebuf, 490);
		writepipe = write(soundfifo, sinebuf, 490);
	}
        printf("The last value of readpru is %d and the last value of writepipe is %d.\n", readpru, writepipe);
	}
        else if (forkit == -1) perror("fork error");
        }


