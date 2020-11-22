#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

/*const char map[6][11] = {
    {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', '*'},
    {'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', '*'},
    {'U', 'V', 'W', 'X', 'Y', 'Z', ' ', ' ', ' ', ' ', '*'},
    {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*'},
    {'-', '+', '=', '!', '?', '@', '%', '&', '$', '.', '*'},
    {'^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^'}};*/

const char map[6][11] = { //PAL
    {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', '*'},
    {'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', '*'},
    {'U', 'V', 'W', 'X', 'Y', 'Z', '*', '*', '*', '*', '*'},
    {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*'},
    {'-', '+', '=', '!', '?', '@', '%', '&', '$', ' ', '*'},
    {'^', '^', '^', '^', '^', '^', '^', '^', '^', '^', '^'}};

const char input_blank[] = {0x00, 0x40, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};
const char input_up[] = {0x40, 0x40, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};
const char input_down[] = {0x80, 0x40, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};
const char input_left[] = {0x00, 0x41, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};
const char input_right[] = {0x00, 0x42, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};
const char input_start[] = {0x01, 0x40, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};
const char input_a[] = {0x02, 0x40, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};
const char input_b[] = {0x04, 0x40, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80};

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

char PreviousEntries[24][5];
int slot = 0;

struct action
{
    const char *action;
    int time;
} action;

struct action GetToNameEntry[] = {
    {input_blank, 300}, // loading
    {input_start, 8},   // skip intro vid
    {input_blank, 50},  // wait for start screen
    {input_start, 8},   // press start on start screen
    {input_blank, 250}, // wait for menu
    {input_down, 8},    // select VS
    {input_start, 8},   // go into VS menu
    {input_blank, 25},  // wait for VS menu to load
    {input_up, 8},      // select name entry
    {input_start, 8},   // go into name entry
    {input_blank, 50}   // wait for name entry to load
};

// assumes we're in the name entry menu with "new" higlighted
struct action EraseNames[] =
    {
        {input_b, 10},
        {input_b, 10},
        {input_down, 4},
        {input_down, 4},
        {input_a, 10},
        {input_up, 4},
        {input_a, 10},
        {input_up, 4},
        {input_a, 20},
        {input_left, 4},
        {input_a, 20},
        {input_left, 4},
        {input_a, 10},
        {input_b, 10},
        {input_b, 10},
        {input_up, 4},
        {input_up, 4},
        {input_a, 10},
        {input_up, 4},
        {input_a, 10}

};

FILE *outfile;

int x = 0;
int y = 0;

void press_button(const char *button, int speed)
{
    int i;
    for (i = 0; i < speed; i++)
        fwrite(button, 8, 1, outfile);
    if (button != input_blank)
        ;
    for (i = 0; i < speed; i++)
        fwrite(&input_blank, 8, 1, outfile);
}

void press_nothing(int frames)
{
    for (int i = 0; i < frames; i++)
        fwrite(&input_blank, 8, 1, outfile);
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
        press_button(xdistance < 0 ? &input_left[0] : &input_right[0], 2);
    }
    for (int ypos = 0; ypos < abs(ydistance); ypos++)
    {
        press_button(ydistance < 0 ? &input_up[0] : &input_down[0], 2);
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

// assumes we start with "new" selected on name entry screen
// also that the screen is full
void erase_names()
{

    // 2 down to erase button
    press_button(&input_down[0], 8);
    press_button(&input_down[0], 8);

    // press erase
    press_button(&input_start[0], 16);

    for (int i = 0; i < 24; i++)
    {
        // select name at top left
        press_button(&input_start[0], 4);

        press_nothing(10); // wait for screen to appear

        // 1 left to yes button
        press_button(&input_left[0], 4);

        // press yes
        press_button(&input_start[0], 4);
    }
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
                
                TypeStrings(PreviousEntries);
                press_nothing(300); // wait a bit
                RunActionSequence(EraseNames, sizeof(EraseNames)); // erase all the names and start again
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

    header = fopen("header.dtm", "r");
    outfile = fopen("outfile.dtm", "w");

    //             /   |   |   |   /   |   |   |   /   |   |   |   /   |   |   |   /   |   |   |   /   |   |   |   X
    //char *text1 = "WEL COME TO THE SECRET TAS BLOCKWITH OWNASAURUS PRACTICAL   TAS KINGKIRB 64 -   AND RASTERI    -";
    while (fread(&buf, 1, 1, header) == 1)
        fwrite(&buf, 1, 1, outfile);
    fclose(header);

    int i, j;

    //RunActionSequence(GetToNameEntry, sizeof(GetToNameEntry));
    RunActionSequence(EraseNames, sizeof(EraseNames));

    /*read(STDIN_FILENO, &buf, 1);

    for (i = 0; i < strlen(text2); i+=4){
        ProcessCharacter(text2+i);
    }*/

    while (1)
    {
        read(STDIN_FILENO, &buf, 1);
        buf = toupper(buf);
        if (ValidChar(buf))
        {
            printf("Valid : %c\n", buf);
            ProcessCharacter(buf);
        }
    }

    //RunActionSequence(EraseNames, sizeof(EraseNames));
}