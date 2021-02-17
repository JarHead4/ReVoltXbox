#ifndef _INPUTS_H
#define _INPUTS_H

void	HandleKeyDown(HWND hWnd, int vKeyCode);
void	Handle_New_Track_Input(HWND hWnd, int vKeyCode);
void	Handle_Module_Placement(HWND hWnd, int vKeyCode);
void	Handle_Module_Selection(HWND hWnd, int vKeyCode);
void	Handle_Load_Menu_Input(HWND hWnd, int vKeyCode);
void	Handle_Save_Track_Input(HWND hWnd, int vKeyCode);
void	Handle_Adjust_Track_Input(HWND hWnd, int vKeyCode);
void	MoveCursor(CURSORDESC* cursor, int vKeyCode);
#endif