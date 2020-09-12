
#ifndef MENU_H_
#define MENU_H_

#define MENUTYPE_NORMAL 1 // A menu item that contains other things
#define MENUTYPE_BROWSER 2
#define MENUTYPE_RUNNINGTAS 3

typedef const struct Menu_Item {
		const  char Name[20];
		const  unsigned char Type;
		const  unsigned char Param; // For buttons - top nybble is bank, bottom nybble is button
		const  struct Menu_Item *Next; /**< Pointer to the next menu item of this menu item */
		const  struct Menu_Item *Previous; /**< Pointer to the previous menu item of this menu item */
		const  struct Menu_Item *Parent; /**< Pointer to the parent menu item of this menu item */
		const  struct Menu_Item *Child; /**< Pointer to the child menu item of this menu item */
} Menu_Item_t;


void Menu_Up(void);
void Menu_HoldUp(void);
void Menu_Down(void);
void Menu_HoldDown(void);
void Menu_Enter(void);

void Menu_Init(void);
void Menu_Display(void);


#endif /* MENU_H_ */
