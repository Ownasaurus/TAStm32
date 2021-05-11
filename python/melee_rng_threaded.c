#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <float.h>
// run this in background : 
// "ffmpeg -i /dev/video2 -update 1 -atomic_writing 1 -y current.bmp"
// Square a number
double S(double i)
{
	return i * i;
}

double ImageDiff(uint8_t *buf1, uint8_t *buf2){
	double totalscore = 0;
	
	/*for (uint32_t pix = 450054; pix < 921654; pix+=3){
		double diff = sqrt(
			S((double)buf1[pix] - (double)(buf2[pix])) + 
			S((double)buf1[pix+1] - (double)(buf2[pix+1])) + 
			S((double)buf1[pix+2] - (double)(buf2[pix+2]))
			);
		if (diff > 300)
			totalscore++;
	}*/
	
	for (uint32_t pix = 0; pix < 450000; pix++){
		double diff = sqrt(
			S((double)buf1[pix] - (double)(buf2[pix]))
			);
		if (diff > 150)
			totalscore++;
	}
	
	
	return totalscore;
}

#define PROCESS(x) process_input(x##_orig, x)

#define PAL
#define NUMTHREADS 4
//#define NO_PROGRESSIVE

//------CODE COURTESY OF SAVESTATE------

#define CHARACTERS_NUM 25
#define MANUAL_THRESHOLD 2400
#define POLLS_PER_SECOND 4800
#define POLLS_PER_MINUTE (POLLS_PER_SECOND*60)
#define POLLS_PER_HOUR (POLLS_PER_MINUTE*60)
#define RESET_THRESHOLD (POLLS_PER_MINUTE*15)

static const char* CHARACTERS[CHARACTERS_NUM] =
{
    "doc", // 0
    "mro", // 1
    "lui", // 2
    "bow", // 3
    "pea", // 4
    "yos", // 5
    "dk",  // 6
    "cfa", // 7
    "gan", // 8
    "fal", // 9
    "fox", // 10
    "nes", // 11
    "ics", // 12
    "kir", // 13
    "sam", // 14
    "zel", // 15
    "lin", // 16
    "yli", // 17
    "pic", // 18
    "pik", // 19
    "jig", // 20
    "mew", // 21
    "gnw", // 22
    "mrt", // 23
    "roy" // 24
};

static uint8_t CHARBUFS[CHARACTERS_NUM][921654];

void loadpics (){

	FILE *f;
	char fname[100];
	int result;

	for (uint32_t i=0; i < CHARACTERS_NUM; i++){
		sprintf(fname, "%s.bmp", CHARACTERS[i]);
		f = fopen (fname, "r");
		if ((result = fread (&CHARBUFS[i][0], 1, 921654, f)) != 921654){
			printf("%d\n", result);
			 exit(1);
		}
		
		fclose(f);
	}
	
}

/*{ 11, 10, 12, 5, 16, 19, 21, 12, 22 };

{ nes, fox, ics, yos, lin, pik, mew, ics, gnw };*/

void rng_adv(uint32_t *seed)
{
    // integer overflow automatically accounted for
    *seed = ((*seed * 214013) + 2531011);
}

uint32_t rng_int(uint32_t *seed, uint32_t max_val)
{
    uint32_t top_bits = *seed >> 16;
    return (max_val * top_bits) >> 16;
}

//--------------------------------------

char input_blank_orig[] = {0x00, 0x40, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};
char input_up_orig[] = {0x40, 0x40, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};
char input_down_orig[] = {0x80, 0x40, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};
char input_left_orig[] = {0x00, 0x41, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};
char input_right_orig[] = {0x00, 0x42, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};
char input_start_orig[] = {0x01, 0x40, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};
char input_a_orig[] = {0x02, 0x40, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};
char input_b_orig[] = {0x04, 0x40, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};
char input_analog_up_orig[] = {0x00, 0x40, 0x00, 0x00, 0x80, 0xFF, 0x80, 0x80};
char input_analog_down_orig[] = {0x00, 0x40, 0x00, 0x00, 0x80, 0x01, 0x80, 0x80};
char input_analog_left_orig[] = {0x00, 0x40, 0x00, 0x00, 0x01, 0x80, 0x80, 0x80};
char input_analog_right_orig[] = {0x00, 0x40, 0x00, 0x00, 0xFF, 0x80, 0x80, 0x80};

char input_blank[8];
char input_up[8];
char input_down[8];
char input_left[8];
char input_right[8];
char input_start[8];
char input_a[8];
char input_b[8];
char input_analog_up[8];
char input_analog_down[8];
char input_analog_left[8];
char input_analog_right[8];

typedef struct action
{
    char *action;
    int time;
    char imagename[16];
} action;

/*struct action GetToVsModeCPU[] =
{
    {input_b, 1000}, // loading
#ifdef NO_PROGRESSIVE
    {input_right, 30}, // move to "No"
#endif
    {input_a, 30}, // enable PAL60 or NTSC progressive mode
    {input_blank, 400},
#ifdef NTSC
    {input_blank, 400},
#endif
#ifdef NO_MEMCARD
    {input_start, 60}, // no memory card1
    {input_blank, 60}, 
    {input_start, 60}, // no memory card2
    {input_blank, 300}, // wait for intro vid to start
#endif
    {input_start, 8},   // skip intro vid
    {input_blank, 100},  // wait for start screen
    {input_start, 20},   // press start on start screen
    {input_blank, 400}, // wait for menu
    {input_down, 10},    // select VS
    {input_blank, 50},  // wait for VS menu to load
    {input_start, 50},   // go into VS menu
    {input_blank, 50},  // wait for VS menu to load
    //{input_b, 300},   // go back to previous menu, uncomment if skipping rest
    {input_start, 50},   // choose MELEE
    {input_blank, 300}, // wait for CSS to load
    {input_analog_up, 42},   // GO STRAIGHT UP TO RANDOM AREA
    {input_blank, 10}
};*/

struct action GetToVsModeCPU[] =
{
/*    {input_b, 300, "-"}, // loading
    {input_a, 30, "1p60sel"}, // enable PAL60 or NTSC progressive mode
    {input_start, 8, "2intro"},   // skip intro vid
    {input_blank, 15, "3start"},   // wait for start screen to fully load
    {input_start, 50, "-"},   // press start on start screen
    {input_blank, 20, "4mainmenu"},   // wait for menu to fully load
    {input_down, 20, "-"},    // select VS
    {input_start, 50, "-"},   // go into VS menu*/
    {input_b, 300, "-"},   // go back to previous menu, uncomment if skipping rest
    {input_start, 50, "-"},   // choose MELEE
    {input_analog_up, 42, "5css"},   // GO STRAIGHT UP TO RANDOM AREA
    {input_blank, 10, "-"}
};

struct action FromVsModeToSheikBTT[] =
{
    {input_b, 300, "-"}, // go back to vs menu
    {input_b, 60, "6vsmenu"}, // go back to main menu
    {input_blank, 50, "-"}, // wait for load of menu
    {input_up, 20, "-"}, // up to single player
    {input_blank, 60, "-"},    // wait
    {input_a, 60, "-"}, // select single player
    {input_blank, 50, "-"},    // wait
    {input_down, 20, "-"},    // down to event match
    {input_blank, 20, "-"},    // wait
    {input_down, 20, "-"},    // down to stadium stuff
    {input_blank, 20, "-"},    // wait
    {input_a, 20, "-"}, // select stadium stuff
    {input_blank, 20, "-"},    // wait
    {input_a, 20, "-"}, // target test
    {input_blank, 20, "7bttcss"}, // wait for CSS to load before proceeding
    //TODO: GO TO ZELDA USING ANGLE x=226 y=211
};


// Compare the currently displayed frame with the specified image
int GetImageScore(char *filename){
    char command[512];
    char buff[512];
    int output_val = 999999;
    snprintf(command, sizeof(command), "compare -metric ae -fuzz 70%% current.bmp %s.png diff.bmp 2>&1", filename);

    FILE *f = popen(command, "r");

    // make sure the command worked
    if(!f)
    {
        printf("failed to run compare. is it installed?");
        exit(1);
    }

    // get the output
    int index = 0;
    while(1)
    {
        char c;

        c = fgetc(f);
        if(c == EOF)
        {
            buff[index] = '\0';
            break;
        }
        else
        {
            buff[index] = c;
        }

        index++;
    }

    output_val = atoi(buff);

    // close our process handle
    pclose(f);

    return output_val;
}

void process_input(char *data, char *new)
{
    char old_byte1 = data[0];
    char old_byte2 = data[1];
    char new_byte1 = 0;
    char new_byte2 = 0;

    new[2] = data[4];
    new[3] = data[5];
    new[4] = data[6];
    new[5] = data[7];
    new[6] = data[2];
    new[7] = data[3];

    new_byte1 += (old_byte1 & 0x01) << 4;// # start
    new_byte1 += (old_byte1 & 0x10) >> 1;// # Y
    new_byte1 += (old_byte1 & 0x08) >> 1;// # X
    new_byte1 += (old_byte1 & 0x04) >> 1;// # B
    new_byte1 += (old_byte1 & 0x02) >> 1;// # A

    new_byte2 += (old_byte2 & 0x04) << 4;// # L
    new_byte2 += (old_byte2 & 0x08) << 2;// # R
    new_byte2 += (old_byte1 & 0x20) >> 1;// # Z
    new_byte2 += (old_byte1 & 0x40) >> 3;// # DpadU
    new_byte2 += (old_byte1 & 0x80) >> 5;// # DpadD
    new_byte2 += (old_byte2 & 0x02);      //# DpadR
    new_byte2 += (old_byte2 & 0x01);      //# DpadL

    new[0] = new_byte1;
    new[1] = new_byte2;
}

int SerialPort;
int prebuffer = 300;

// wait for device to  request data  then send it
void WriteControl(char *data)
{
    char byte = 0;
    char out[9];

    if (prebuffer == 0)
    {
        while (byte != 'A')
        {
            read(SerialPort, &byte, 1);    
        }
    }
    else
    {
        prebuffer--;
    }

    // prepare data out packet
    out[0] = 'A';
    for(int x = 1;x < 9;x++)
    {
        out[x] = data[x-1];
    }

    // send single packet
    write(SerialPort, out, 9);
}

// wait for device to  request data  then send it
void WriteControlNoWait(char *data)
{
    char out[9];

    out[0] = 'A';
    for(int x = 1;x < 9;x++)
    {
        out[x] = data[x-1];
    }

    // send single packet
    write(SerialPort, out, 9);
}

void press_and_release_button(char *button, int speed)
{
    int i;
    for (i = 0; i < speed; i++)
        WriteControl(button);
    for (i = 0; i < speed; i++)
        WriteControl(input_blank);
}

// Wait for buffer to be empty, press and release button, then wait for buffer to be empty again
void press_and_release_button_sync(char *button, int speed)
{
    int i;
    char byte = 0;
    tcflush(SerialPort, TCIOFLUSH);
    while (byte != '\xB2')
    {
        read(SerialPort, &byte, 1);
    }
    for (i = 0; i < speed; i++)
        WriteControlNoWait(button);
    for (i = 0; i < speed; i++)
        WriteControlNoWait(input_blank);
    
    byte = 0;
    
    tcflush(SerialPort, TCIOFLUSH);
        
    while (byte != '\xB2')
    {
        read(SerialPort, &byte, 1);
    }
}

void press_button(char *button, int polls)
{
    int i;
    for (i = 0; i < polls; i++)
        WriteControlNoWait(button);
}

void RunActionSequence(struct action *seqorig, int lengthorig)
{

    int length, delay;
    struct action *seq;
    RESET:
    seq = seqorig;
    length = lengthorig / sizeof(action);
    
    delay = 0;
    while (length--)
    {
        // wait for image to appear
        
        if (seq->imagename[0] != '-'){
        //printf("waiting for %s\n", seq->imagename);
            while (GetImageScore(seq->imagename) > 1000){
            	// hack to reset if gamecube locks up on boot screen
		    usleep(100000);
		    delay++;
		    if (delay > 200){
		    printf("Uh oh, console locked up. Restarting.\n");
		    char power_msg[] = {'P', '0'};
		    write(SerialPort, power_msg, 2); // turn the console off
		    sleep(5); // wait a second
		    power_msg[1] = '1';
		    write(SerialPort, power_msg, 2); // turn the console on

		    sleep(1);
		    tcflush(SerialPort, TCIOFLUSH);
		    goto RESET;
		    }
            }
            //printf("got %s - %u\n", seq->imagename, GetImageScore(seq->imagename));
        }
        
        press_and_release_button(seq->action, seq->time);
        seq++;
    }
}
//60,250 - 200,460
bool check_sequence(uint32_t seed, int clist[9])
{
    int i;
    
    for(i=0; i<9; i++)
    {
        rng_adv(&seed);
        if (rng_int(&seed, 25) != clist[i])
        {
            return false;
        }
        rng_adv(&seed);
    }
    
    return true;
}


int determine_character()
{
    FILE *f;
    int min_index = -1;
    double min_compare = DBL_MAX;
    
    double output_val = 999999;
    
    static uint8_t buffah[921654];

	f = fopen ("current.bmp", "r");
	fread (buffah, 921654, 1, f);
	fclose(f);
    
//    double ImageDiff(char *buf1, *buf2){

    for(int i = 0;i < CHARACTERS_NUM;i++)
    {
        // run the image analysis
        output_val = ImageDiff(&CHARBUFS[i][0], &buffah[0]);
        //printf("%f\n", output_val);

        //compare and update min index
        if(output_val < min_compare)
        {
            min_compare = output_val;
            min_index = i;
        }
    }

    //return index associated with char name
    printf("%s, ", CHARACTERS[min_index]); 
    fflush(stdout);
    return min_index;
}

int clist[9] = { 11, 10, 12, 5, 16, 19, 21, 12, 22 };

bool SuccessfulThread = 0;
uint32_t FoundSeed = 0;
uint64_t checksPerThread = ((uint64_t)0x100000000 / NUMTHREADS) + 1;

void *RNGThread (void *threadinfo)
{
    uint32_t seed = *(uint32_t *)threadinfo;
    
    uint32_t checks = 0xFFFFFFFF;
    bool success = false;

    //printf("Thread started : %X\n", seed);

    for(checks = 0;checks < checksPerThread && !SuccessfulThread;checks++) // max number of checks would be this many, going through the whole 32 bit spectrum
    {
        /*if(!(checks % 0x10000000))
            printf("Analyzing melee's RNG algorithm for a match....\n");*/

        if(check_sequence(seed, clist))
        {
            success = true;
            break;
        }

        rng_adv(&seed);
    }
    if (success){
        FoundSeed = seed;
        SuccessfulThread = 1;
        //printf("This thread found seed\n");
    }//
    else {
        //printf("Another thread found seed\n");
    }
    return 0;
}

uint32_t startSeed[NUMTHREADS] = {0x1, 0x4029E2C0, 0x4823F683, 0xD8BE873A};

void Connect(){

    char buf;   
    
    char reset = 'R';
    char setupstring[] = {'S', 'a', 'G', 0x80, 0x00, '\n'};
    char retstring[] = {'\0', '\0'};
    
    char port[20] = "/dev/ttyACM0"; //TODO: make dynamic
    speed_t baud = B115200;
    
    PROCESS(input_blank);
    PROCESS(input_up);
    PROCESS(input_down);
    PROCESS(input_left);
    PROCESS(input_right);
    PROCESS(input_start);
    PROCESS(input_a);
    PROCESS(input_b);
    PROCESS(input_analog_up);
    PROCESS(input_analog_down);
    PROCESS(input_analog_left);
    PROCESS(input_analog_right);
    
    
    if ((SerialPort = open(port, O_RDWR)) == -1)
    {
        printf("Failed to connect to the serial device. Uh oh! Is the device plugged in and located at %s?\n", port);
        exit(1);
    }
    
    struct termios settings;
    tcgetattr(SerialPort, &settings);

    cfsetospeed(&settings, baud); 
    settings.c_cflag &= ~PARENB; 
    settings.c_cflag &= ~CSTOPB;
    settings.c_cflag &= ~CSIZE;
    settings.c_cflag |= CS8 | CLOCAL; 
    settings.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    settings.c_oflag &= ~OPOST; 
    settings.c_cc[VMIN]  = 0;
    settings.c_cc[VTIME] = 10;

    tcsetattr(SerialPort, TCSANOW, &settings); 
    tcflush(SerialPort, TCOFLUSH);
    
    do
    {
       tcflush(SerialPort, TCIFLUSH);
       write(SerialPort, &reset, 1);
       sleep(1);
       read (SerialPort, &retstring[0], 2);
       //printf("%X %X\n", retstring[0], retstring[1]);
    }
    while (!(retstring[0] == 0x01 && retstring[1] == 'R'));
   
    write(SerialPort, &setupstring[0], 5);
    sleep(1);
    read (SerialPort, &retstring[0], 2);

    if (!(retstring[0] == 0x01 && retstring[1] == 'S'))
    {
        printf("oh dear.\n");
        exit(1);
    }
    // precalc the start numbers for each thread
    /*uint32_t seedage = 1;

    for (uint32_t c = 0; c < 0xFFFFFFFF; c++){
        if (c % checksPerThread == 0){
            startSeed[c / checksPerThread] = seedage;
            printf("Thread no %lu, starting at seed %X - %X\n", c / checksPerThread, c, seedage);
        }
        rng_adv(&seedage);
    }*/
}

void GetCharacterList(){
    //sleep(1);
    //DETRMINE CURRENT SEED:
    printf("DETECTED : ");
    fflush(stdout);
    for(int peek = 0;peek < 9;peek++)
    {
	//tcflush(SerialPort, TCIFLUSH);
        press_and_release_button_sync(input_a, 7); // place P1 coin
        press_button(input_b, 3);
        press_button(input_blank, 1);
        clist[peek] = determine_character();
        press_and_release_button_sync(input_blank, 1);
        //press_and_release_button_sync(input_b, 3); // recall P1 coin - TODO this could perhaps be made non-sync and moved before determine character to buffer the next character while the algorithm is running
    }
    printf("\n");
}
bool seedFailed = 1;
uint32_t CrackSeed(){
	seedFailed = 1;
    pthread_t threadHandles[NUMTHREADS];
    int rc = 0;
    for (int i = 0; i < NUMTHREADS; i++)
    {
        rc = pthread_create(&threadHandles[i], NULL, RNGThread, &startSeed[i]);
    
        if (rc)
        {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    uint32_t our_seed = 0;

    for (int i = 0; i < NUMTHREADS; i++)
        rc = pthread_join(threadHandles[i], NULL);

    if(SuccessfulThread)
    {
        our_seed = FoundSeed;
        
        SuccessfulThread = 0;
        FoundSeed = 0;
        seedFailed = 0;
        return our_seed;
    }
    else
    {
        printf("Sequence not found. Hmm.... Aborting.\n");
        seedFailed = 1;
    }
}

uint32_t FindSeedDistance(uint32_t our_seed, uint32_t target_seed, uint32_t max_distance){

    uint32_t checks = 0;
    for(checks = 0;checks < max_distance;checks++) // max number of checks would be this many, going through the whole 32 bit spectrum
    {
        if(our_seed == target_seed)
        {
            // target acquired!
            return checks;
            break;
        }
        else
        {
            rng_adv(&our_seed);
        }
    }
    
    // seed is beyond our threshold, return error state
    return 0xFFFFFFFF;
}

int main(int argc, char **argv)
{
    printf("\e[1;1H\e[2J");
    loadpics();
    /*while(1){
    	GetCharacterList();
    }*/
    Connect();
    char power_msg[] = {'P', '0'};
    // At this point, we're all connected to the TAStm32 and ready to rock

    // Boot and get to VS screen

BOOT:
    power_msg[1] = '0';
    write(SerialPort, power_msg, 2); // turn the console off
    sleep(1); // wait a second
    printf("\e[1;1H\e[2J");
    fflush(stdout);
    sleep(1); // wait a second
    printf("TASBot initialized. Starting console...\n");
    sleep(1); // wait a second

    tcflush(SerialPort, TCIOFLUSH);

            power_msg[1] = '1';
            write(SerialPort, power_msg, 2); // turn the console on

            sleep(1);
            tcflush(SerialPort, TCIOFLUSH);
    prebuffer = 292;
    RunActionSequence(GetToVsModeCPU,sizeof(GetToVsModeCPU));
    
    
    int firstTime = 1;
    uint32_t target_seed;
    uint32_t our_seed;
    uint32_t checks;
    while(true)
    {
    	printf("Determining RNG seed from random characters...\n");
        GetCharacterList();

        our_seed = CrackSeed();
        if (seedFailed){
            power_msg[1] = '0';
            write(SerialPort, power_msg, 2); // turn the console off
            sleep(1); // wait a second
            power_msg[1] = '1';
            write(SerialPort, power_msg, 2); // turn the console on

            sleep(1);
            tcflush(SerialPort, TCIOFLUSH);

            goto BOOT;
            
        }
        
        // advance twenty times more - 18 more to iterate through 9-char sequence
        for (int cnt = 0; cnt < 18; cnt++)
            rng_adv(&our_seed);

        

        //TODO: switch this to the actual thing!
        //forces the target RNG seed to be ~10 seconds away
        //const int TARGET_DISTANCE = 49;

        const int TARGET_DISTANCE = 50000;
        if(firstTime)
        {
            target_seed = our_seed;
            for(int blah = 0;blah < TARGET_DISTANCE;blah++)
            {
                rng_adv(&target_seed);
            }

            firstTime = 0;
        }
        
        target_seed = 0x9A338BA8; // 18 seeds before the actual target. so we can check a few characters to be safe without overshooting
        //target_seed = 0xC7E0A072; // THE ACTUAL TARGET

        // CALCULATE HOW TO ADVANCE TO TARGET SEED:
        
        //printf("Now calculating how long it will take to reach target seed 0x%X\n", target_seed);

        checks = FindSeedDistance(our_seed, target_seed, RESET_THRESHOLD);
        
        if (checks == 0xFFFFFFFF)
        {
            printf("Seed is 0x%X, which is too far away.\n", our_seed);
            printf("Restarting to get a better seed!\n");

 

            goto BOOT;
        }
        
        printf("Seed is 0x%X, which is pretty close.\n", our_seed);
        printf("It will take %u RNG advances to reach it.\n", checks);
        printf("This is %.2f hours, %.2f minutes, or %.2f seconds.\n", ((checks/4833.9)/60)/60, (checks/4833.9)/60, checks/4833.9);

        if(checks < MANUAL_THRESHOLD)
        {
            // go to manual advancement
            break;
        }

        printf("\nNow advancing the RNG seed....\n");
        
        // ACTUALLY ADVANCE TO TARGET SEED:

        press_and_release_button(input_a, 10); // place P1 coin
        press_and_release_button(input_analog_right, 24); //move to P2 select
        press_and_release_button(input_analog_down, 10); //move to P2 select
        float sec_sleep = checks/4833.0;
        int sec_converted = (int)sec_sleep;
        float usec_difference = (sec_sleep - sec_converted)*1000*1000;
        int usec_converted = (int)usec_difference;
        sleep(5);
        press_and_release_button(input_a, 10); // turn on CPU
        sleep(sec_converted); // this should bring is to a little bit before the actual seed.
        usleep(usec_converted); // this should bring us even closer to the actual seed!
        press_and_release_button(input_a, 10); // recall the CPU coin
        press_and_release_button(input_analog_up, 10); // move to random space
        press_and_release_button(input_analog_left, 24); //move to random space
        press_and_release_button(input_b, 10); // recall P1 coin
        sleep(1);
    }

    // now we're within 4900
    printf("We're close. Let's do the rest manually.\n");
    sleep(5);

    if(checks % 2 == 1)
    {
        press_and_release_button(input_analog_right, 24); //move to P2 select
        press_and_release_button(input_analog_down, 10); //move to P2 select
        sleep(1);
        press_and_release_button(input_a, 10); // turn on CPU
        press_and_release_button(input_a, 10); // recall the CPU coin
        press_and_release_button(input_analog_up, 10); // move to random space
        press_and_release_button(input_analog_left, 24); //move to random space
        checks--;
    }

    for(int z = 0;z < checks;z+=2)
    {
        press_and_release_button_sync(input_a, 4); // rand select P1
        press_and_release_button_sync(input_b, 4); // recall P1
    }

    printf("Let's confirm we're at the target seed...\n");

    GetCharacterList();

    our_seed = CrackSeed();
    
    if(our_seed == target_seed)
    {
        //printf("We did it! we're at the correct seed! Press ENTER to begin the run....\n"); getchar();
        printf("We did it! we're at the correct seed! (0x%X)\nStarting run...\n", our_seed); 
    }
    else
    {
        printf("Detected seed: 0x%X instead of our target, unfortunately :(\n", our_seed);
    }
    
    tcflush(SerialPort, TCIOFLUSH);
    sleep(3);
    //GO TO SHEIK BTT:
    tcflush(SerialPort, TCIOFLUSH);
    prebuffer = 292;
    RunActionSequence(FromVsModeToSheikBTT,sizeof(FromVsModeToSheikBTT));

    //EXECUTE RUN:
    int ret_val = 1;
    while (ret_val != 0){
    	ret_val = system("./tastm32.py --console gc sheik_v2_no_restart.dtm --transition 198 R");
    }

    return 0;
}
