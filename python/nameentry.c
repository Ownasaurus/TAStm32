#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>

const char map[6][11] = {
    {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', '*'},
    {'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', '*'},
    {'U', 'V', 'W', 'X', 'Y', 'Z', ' ', ' ', ' ', ' ', '*'},
    {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*'},
    {'-', '+', '=', '!', '?', '@', '%', '&', '$', '.', '*'},
    {'^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^'}};

/*const char map[6][11] = { //PAL
    {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', '*'},
    {'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', '*'},
    {'U', 'V', 'W', 'X', 'Y', 'Z', '*', '*', '*', '*', '*'},
    {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*'},
    {'-', '+', '=', '!', '?', '@', '%', '&', '$', ' ', '*'},
    {'^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^'}};*/

char input_blank_orig[] = {0x00, 0x40, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};
char input_up_orig[] = {0x40, 0x40, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};
char input_down_orig[] = {0x80, 0x40, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};
char input_left_orig[] = {0x00, 0x41, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};
char input_right_orig[] = {0x00, 0x42, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};
char input_start_orig[] = {0x01, 0x40, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};
char input_a_orig[] = {0x02, 0x40, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};
char input_b_orig[] = {0x04, 0x40, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};

const char substitutions[][2] = {
    " .",
    "B8",
    "T7",
    "L1",
    "A4",
    "S$",
    "A@",
    "E3",
    "O0",
    "S5",
    "T+",
    "I1",
    " -",
    " ="};
    
char input_blank[8];
char input_up[8];
char input_down[8];
char input_left[8];
char input_right[8];
char input_start[8];
char input_a[8];
char input_b[8];

char PreviousEntries[24][5];
int slot = 0;

struct action
{
    char *action;
    int time;
} action;

struct action GetToNameEntry[] = {
    {input_blank, 600}, // loading, hold B to get to progressive scan menu
    {input_a, 30}, // enable progressive scan
    {input_blank, 700}, // wait for intro vid to start
    {input_start, 8},   // skip intro vid
    {input_blank, 300},  // wait for start screen
    {input_start, 20},   // press start on start screen
    {input_blank, 100}, // wait for menu
    {input_down, 10},    // select VS
    {input_start, 50},   // go into VS menu
    {input_blank, 50},  // wait for VS menu to load
    {input_up, 20},      // select name entry
    {input_start, 8},   // go into name entry
    {input_blank, 50}   // wait for name entry to load
};

// assumes we're in the name entry menu with "new" higlighted
struct action EraseNames[] =
    {
        {input_b, 15},
        {input_b, 15},
        {input_down, 8},
        {input_down, 8},
        {input_a, 15},
        {input_up, 8},
        {input_a, 15},
        {input_up, 8},
        {input_a, 15},
        {input_left, 8},
        {input_a, 15},
        {input_left, 8},
        {input_a, 15},
        {input_b, 15},
        {input_b, 15},
        {input_up, 8},
        {input_up, 8},
        {input_a, 15},
        {input_up, 8},
        {input_a, 15}

};

FILE *outfile;

int x = 0;
int y = 0;

int SerialPort;

int prebuffer = 300;

// wait for device to  request data  then send it
void WriteControl(char *data) {

	char byte = 0;
	char ay = 'A';
	if (prebuffer == 0) {
		while (byte != 'A') {
			read(SerialPort, &byte, 1);
			
		}
	}
	else prebuffer--;
	write(SerialPort, &ay, 1);
	write(SerialPort, data, 8);
}

void process_input(char *data, char *new) {

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

void press_button(char *button, int speed)
{
    int i;
    for (i = 0; i < speed; i++)
        WriteControl(button);
    for (i = 0; i < speed; i++)
        WriteControl(input_blank);
}

void press_nothing(int frames)
{
    for (int i = 0; i < frames; i++)
        WriteControl(input_blank);
}

// navigate to and enter a character
void type_char(char c)
{
    int xdistance = 0, ydistance = 0;

    // find the char in the table
    for (int ypos = 0; ypos < 6; ypos++)
    {
        for (int xpos = 0; xpos < 11; xpos++)
        {
            if (map[ypos][xpos] == c)
            {
                xdistance = xpos - x;
                ydistance = ypos - y;
                xpos = ypos = 1000;
            }
        }
    }

    //printf ("%d %d\n", xdistance, ydistance);

    if (xdistance >= 6)
        xdistance = -(11 - xdistance);
    else if (xdistance <= -6)
        xdistance = -(-11 - xdistance);

    if (ydistance >= 3)
        ydistance = -(6 - ydistance);
    else if (ydistance <= -3)
        ydistance = -(-6 - ydistance);

    for (int xpos = 0; xpos < abs(xdistance); xpos++)
    {
        press_button(xdistance < 0 ? &input_left[0] : &input_right[0], 3);
    }
    for (int ypos = 0; ypos < abs(ydistance); ypos++)
    {
        press_button(ydistance < 0 ? &input_up[0] : &input_down[0], 3);
    }
    x = x + xdistance;
    y = y + ydistance;

    press_button(&input_a[0], 3);
}

// assumes we start with "new" selected on name entry screen
void add_name(char *name)
{

    press_button(&input_start[0], 10);
    printf("%c%c%c%c\n", name[0], name[1], name[2], name[3]);
    x = 0;
    y = 0;
    for (int i = 0; i < 4; i++)
    {
        type_char(name[i]);
    }

    press_button(&input_start[0], 10);
}

void RunActionSequence(struct action *seq, int length)
{
    length /= sizeof(action);
    while (length--)
    {
        //printf("%d\n", length);
        press_button(seq->action, seq->time);
        seq++;
    }
}

char PreviouslyUsed()
{
    for (int k = 0; k < slot; k++)
    {
        if (strcmp(PreviousEntries[k], PreviousEntries[slot]) == 0)
        {
            printf("previously done : %s\n", PreviousEntries[slot]);
            return 1;
        }
    }
    return 0;
}

int countchar(char *string, char c)
{
    int len = strlen(string);
    int cnt = 0;
    while (len--)
    {
        if (string[len] == c)
            cnt++;
    }
    return cnt;
}

void TypeString(char *string)
{
    slot = 0;
    // enter the names
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 6; j++)
        {
            memcpy(&PreviousEntries[slot][0], &string[(i * 4) + (j * 16)], 4);
            PreviousEntries[slot][4] = 0;
            if (PreviouslyUsed())
            {
                // don't add it if we already did
            }

            add_name(PreviousEntries[slot]);

            slot++;
        }
    }
}

void TypeStrings()
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 6; j++)
        {
            add_name(&PreviousEntries[i + (j * 4)][0]);
        }
    }
}

int row = 0, column = 0;

// character within slot
int character = 0;

// process 1 char from input stream, needs to be re-entrant
void ProcessCharacter(char c)
{
    PreviousEntries[slot][character++] = c;

    // if we filled a slot, add it
    if (character == 4)
    {
        
        if (PreviouslyUsed())
        {
            // don't add it if we already did
        }
        else
        {
            slot++;
            // if we filled all slots, type a screen of text
            if (slot == 24)
            {
                RunActionSequence(EraseNames, sizeof(EraseNames)); // erase all the names and start again
                press_nothing(20);
                TypeStrings(PreviousEntries);
                press_nothing(1000); // wait a bit
                
                slot = 0;
            }
        }
        
        character = 0;
    }
}

char ValidChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 -+=!?@%&$.";

char ValidChar(char c)
{
    int l = strlen(ValidChars);
    while (l--)
    {
        if (ValidChars[l - 1] == c)
            return 1;
    }
    return 0;
}

/*
algorithm is - 
1. keep adding characters until we get a sequence with no repeats across 4-character groupings

*/


int main()
{

    FILE *header;
    char buf;
    
    char reset = 'R';
    char setupstring[] = {'S', 'A', 'G', 0x80, 0x00, '\n'};
    char retstring[2];

    //header = fopen("header.dtm", "r");
    // = fopen("outfile.dtm", "w");
    
    char port[20] = "/dev/ttyACM0";
    speed_t baud = B115200; /* baud rate */
    
    process_input(input_blank_orig, input_blank);
    process_input(input_up_orig, input_up);
    process_input(input_down_orig, input_down);
    process_input(input_left_orig, input_left);
    process_input(input_right_orig, input_right);
    process_input(input_start_orig, input_start);
    process_input(input_a_orig, input_a);
    process_input(input_b_orig, input_b);
    
    ; /* connect to port */
    
    if ((SerialPort = open(port, O_RDWR)) == -1)
    {
    	printf("uh oh\n");
    	exit(1);
    }
    
    
    


    struct termios settings;
    tcgetattr(SerialPort, &settings);

    cfsetospeed(&settings, baud); /* baud rate */
    settings.c_cflag &= ~PARENB; /* no parity */
    settings.c_cflag &= ~CSTOPB; /* 1 stop bit */
    settings.c_cflag &= ~CSIZE;
    settings.c_cflag |= CS8 | CLOCAL; /* 8 bits */
    settings.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    settings.c_oflag &= ~OPOST; /* raw output */
    settings.c_cc[VMIN]  = 0;
	settings.c_cc[VTIME] = 10;

    tcsetattr(SerialPort, TCSANOW, &settings); /* apply the settings */
    tcflush(SerialPort, TCOFLUSH);
    
    retstring[0] = 0; retstring[1] = 0;
    
    write(SerialPort, &reset, 1);
    sleep(1);
    read (SerialPort, &retstring[0], 2);
    
    while (!(retstring[0] == 0x01 && retstring[1] == 'R')){
	write(SerialPort, &reset, 1);
    	read (SerialPort, &retstring[0], 2);
    }
    printf("reset ok\n");
    sleep(1);
    tcflush(SerialPort, TCIOFLUSH);
    sleep(1);
    tcflush(SerialPort, TCIOFLUSH);
    
    write(SerialPort, &setupstring[0], 5);
    sleep(1);
    read (SerialPort, &retstring[0], 2);
    
    if (!(retstring[0] == 0x01 && retstring[1] == 'S')){
        printf("oh dear\n", retstring[0], retstring[1]);
        exit(1);
    }
    
    //printf("ok :%x %x\n",   retstring[0],   retstring[1]);
    
    

    //             /   |   |   |   /   |   |   |   /   |   |   |   /   |   |   |   /   |   |   |   /   |   |   |   X
    //char *text1 = "WEL COME TO THE SECRET TAS BLOCKWITH OWNASAURUS PRACTICAL   TAS KINGKIRB 64 -   AND RASTERI    -";
    /*while (fread(&buf, 1, 1, header) == 1)
    fwrite(&buf, 1, 1, outfile);
    fclose(header);*/
    

    int i, j;

    RunActionSequence(GetToNameEntry, sizeof(GetToNameEntry));
    //RunActionSequence(EraseNames, sizeof(EraseNames));

    /*read(STDIN_FILENO, &buf, 1);

    for (i = 0; i < strlen(text2); i+=4){
    ProcessCharacter(text2+i);
    }*/

fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
int numread = 0;

    while (1)
    {
        numread = read(0, &buf, 1);
        if (numread == 1){
		buf = toupper(buf);
		if (ValidChar(buf))
		{
		    printf("Valid : %c\n", buf);
		    ProcessCharacter(buf);
		}
        }
        else {
        	press_nothing(1);
        	printf("nothing\n");
        }
    }

    //RunActionSequence(EraseNames, sizeof(EraseNames));
}
