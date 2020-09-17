
#ifndef MENU_H_
#define MENU_H_

// How quickly the cursor moves when up/down buttons are held
#define HOLDDELAY 20

typedef enum {
	MENUTYPE_BROWSER,
	MENUTYPE_TASINPUTS,
	MENUTYPE_TASSTATS
} MenuType;

void Menu_Up(void);
void Menu_HoldUp(void);
void Menu_Down(void);
void Menu_HoldDown(void);
void Menu_Enter(void);
void Menu_Settings(void);

void Menu_Init(void);
void Menu_Display(void);


#endif /* MENU_H_ */
