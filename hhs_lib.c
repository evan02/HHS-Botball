/*	hhs_lib.c
	Hampton High School Botball Team
	Team #0141
	DC Region
	
	Adam Farabaugh - adam.farabaugh1@gmail.com
	Evan Wilson - emwilson02@gmail.com
	
	Change Log:
		May 03 2010		- project begun
		Jul 01 2010		- new hhs_run_for() updates global flag for game_time remaining. ohhhh yeahhh.
		Mar 19 2011		- fixed weird bug in hhs_run_for()
		Apr 01 2011		- fixed ssp_smooth() -- will now work for decreasing as well as increasing positions
*/

// Globals
int game_time; // global flag for time left in game
int which_side = 0; // global flag: -1 is left, 1 is right on game board
int _func_over; // flag so hhs_run_for knows when to return

// Prototypes
void hhs_run_for(float delay, void (*func));
void ssp_smooth(int port, int pos, int step_size, int step_delay);
void wait_a_button();
void setup_which_side();

// Misc. Macros
#define ssp(port,pos) (set_servo_position(port,pos))
#define gsp(port) (get_servo_position(port))
#define pause() (msleep(150))

/*  This is the HHS alternative to KIPR's run_for().
	The main difference is that it stores the time it is
	referencing in the global int game_time so other
	functions can access it and make decisions based on
	how close the game is to ending.

	Also KIPR had some funky way of knowing if the process
	was ended, I tried it but it failed epically so I just
	replaced it with another flag thats updated in the end
	of our game function.
*/
void hhs_run_for(float delay,void (*func)){
	int pid=start_process(func);
	_func_over=0;
	game_time=(int)(delay);
	while(delay>=1.0 && _func_over!=1){
		sleep(1.0);
		delay=delay-1.0;
		game_time=(int)(delay);
	}
	sleep(delay);
	game_time=0;
	kill_process(pid);
	create_stop();off(0);off(1);off(2);off(3);
	return;
}

/*  This function actuates a servo in steps.  Hopefully
	it will allow for smooth servo movement, so pieces
	held won't be jerked out of their claws/etc
*/
void ssp_smooth(int port, int pos, int step_size, int step_delay){
	int cp=get_servo_position(port);
	if(cp<pos){	
		while(cp<(pos-step_size)){
			ssp(port,cp+step_size);
			msleep(step_delay);
			cp=gsp(port);
		}
		ssp(port, pos);
	}else{
		while(cp>(pos+step_size)){
			ssp(port,cp-step_size);
			msleep(step_delay);
			cp=gsp(port);
		}
		ssp(port, pos);
	}
}

/*	Blocks program execution until a_button
	is pressed and then released.
*/
void wait_a_button(){
	while(!a_button()){}
	while(a_button()){}
}

/*	Setup function so we can tell the bot which side of 
	the table we're on
	Used during 2011 Botball Game (symmetrical table design)
*/
void setup_which_side(){
	printf("Which side of the board are we on?\n     Press   LEFT   or   RIGHT\n             <--          -->\n");
	while(!which_side){
		if(left_button()){which_side=-1;}
		if(right_button()){which_side=1;}
	}
}
