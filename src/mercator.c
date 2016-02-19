/*
 *	Author:	Karel De Coster
 *	Date:	2016-02-19
 *	Description:	The program samples FRAME_SIZE values from the MCP3008 adc via SPI. It processes these measurements,
 *	If the sampled values are consistent with the working environment of our algorithm, we calculate the position and 
 *	write it to /var/lib/mercator/x & /var/lib/mercator/y . Regardless of success, a message will be posted in the log file
 *	located at /var/log/mercator/mercator.log. The stdout is redirected to /var/log/mercator/my_stdout, stderr is redirected
 *	to /var/log/mercator/my_stderr. In the main loop there is a sleep(1) wich eventually should be removed.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <signal.h>
#include <mercator_processing_unit.h>
#include <mercator_data_acquisition_unit.h>
#include <time.h>

#define FRAME_SIZE 1024
#define PHI (300.0/76800.0)	/*Lowest emited frequency deviced by the sampling frequency*/
#define VREF 3.3

volatile sig_atomic_t done = 0;


int write_log(char message[]){
	time_t timer;
	struct tm* tm_info;
	char timebuff[26];
	char cmd[26 + strlen(message)+ 40];
	
	time(&timer);
	tm_info = localtime(&timer);
	
	strftime(timebuff,26,"%Y-%m-%d %H:%M:%S", tm_info);
	snprintf(cmd ,1000 ,"echo %s : %s >> /var/log/mercator/mercator.log" ,timebuff ,message);
	system(cmd);
	
	return EXIT_SUCCESS;
}

void term(int signum){
	done = 1;
}

unsigned int post_position(double* position){
	char buff[40];
	
	if(position == NULL){	/*Appearently an error occurred somewhere, log it and try again.*/
		write_log("Non-Fatal Error: Failed to calculate position.");
		return EXIT_FAILURE;
	}
	snprintf(buff,40,"echo %lf > /var/lib/mercator/x",position[0]);
	system(buff);
	snprintf(buff,40,"echo %lf > /var/lib/mercator/y",position[1]);
	system(buff);
	snprintf(buff,40,"Position update : [%lf,%lf].",position[0],position[1]);
	write_log(buff);
	free(position);
	
	return EXIT_SUCCESS;
}

double* calculate_position(double* raw_data, mpu_data_t* mpu){
	double* position;
	if(raw_data==NULL){
		write_log("Non-Fatal Error: No raw data received.");
		return NULL;
	}
	if(mpu == NULL){
		write_log("Error: No reference to mercator processing unit resources.");
		done = 1;
		free(raw_data);
		raw_data=NULL;
		return NULL;
	}
	position = mpu_execute(raw_data, mpu);
	return position;

}

int main(int argc, char *argv[]) {
	pid_t pid, sid;
    struct sigaction action;
	mpu_data_t* mpu;
	
	/*---------------// Start Up // -----------------------*/
	/* Fork the Parent Process */
	pid = fork();

	if (pid < 0) { 
		exit(EXIT_FAILURE); 
	} 
	
	/* We got a good pid, Close the Parent Process */
	if (pid > 0) { 
		exit(EXIT_SUCCESS); 
	}
	
	/* Change File Mask */
	umask(0);

	/* Create a new Signature Id for our child */
	sid = setsid();
	if (sid < 0) { 
		exit(EXIT_FAILURE); 
	}
	
	/* Change Directory 
	   If we cant find the directory we exit with failure. */
	if ((chdir("/")) < 0) { 
		exit(EXIT_FAILURE); 
	} 
	
	/* Redirect STDIN, STDOUT & STDERR. */
	int fd = open("/dev/null", O_RDONLY);	/* No input should be required */
	if(fd > -1){
        	dup2(fd, STDIN_FILENO);
            close(fd);
    }
	fd = open("/var/log/mercator/my_stdout", O_WRONLY | O_CREAT | O_APPEND); /* TODO: redirect to /dev/null */
	if(fd > -1){
		dup2(fd, STDOUT_FILENO);
		close(fd);
	}
	fd = open("/var/log/mercator/my_stderr", O_WRONLY | O_CREAT | O_APPEND); /* TODO: redirect to /dev/null */
    if(fd > -1){
        dup2(fd, STDERR_FILENO);
        close(fd);
    }

	/* Set up a handler to break the loop when we want to shutdown. */
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = term;
    sigaction(SIGTERM, &action, NULL);
    
    write_log("Mercator starting up.");
    
    mpu = mpu_create(FRAME_SIZE, PHI);
    if(mpu == NULL){
    	write_log("Error : Failed to allocate Mercator Processing Unit resources.");
    	exit(EXIT_FAILURE);
    }
    write_log("Mercator Processing Unit resources successfully allocated.");
    
   	FREQUENCY_t fs = FREQ_76_8kHz;
   	if(mdau_create(FRAME_SIZE, fs, VREF) != EXIT_SUCCESS){
   		write_log("Error : Failed to allocate Mercator Data Acquisition Unit resources.");
   		mpu_destroy(&mpu);
   		exit(EXIT_FAILURE);
   	}
   	write_log("Mercator Data Acquisition Unit resources successfully allocated.");
	
	/*---------------- //Main Process //---------------- */
	
	double* raw_data;
	double* position; /* {X, Y} */
	mdau_start_sampling();
	while(!done){ 
		sleep(1);	/* TODO: remove this in the final version */
		raw_data = mdau_wait_frame();
		mdau_start_sampling();
		position = calculate_position(raw_data, mpu);
		free(raw_data);
		raw_data=NULL;
		post_position(position);		
	}
	raw_data = mdau_wait_frame();
	free(raw_data);
	raw_data=NULL;
	
	
	/*------------- // Shut Down // -----------------*/
	int shutdown_status = EXIT_SUCCESS;
	write_log("Mercator shutting down.");
	if(mpu_destroy(&mpu)!=EXIT_SUCCESS){
		write_log("Error : Failed to free Mercator Processing Unit resources.");
		shutdown_status = EXIT_FAILURE;
	}
	if(mdau_destroy()!=EXIT_SUCCESS){
		write_log("Error : Failed to free Mercator Data Acquisition Unit resources.");
		shutdown_status = EXIT_FAILURE;
	}
	if(shutdown_status != EXIT_SUCCESS) {
		write_log("Shutdown finished but failed to free all resources.");
	}else{
		write_log("Shutdown successfully completed.");
	}
	
	exit(shutdown_status);
	
}
