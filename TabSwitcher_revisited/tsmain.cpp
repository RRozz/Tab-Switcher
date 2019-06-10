	#include "irrlicht.h"
#include "windows.h"
#include <iostream>
#include <string>

#include "Main2.cpp"


using namespace std;
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

int main(int argc, char* args[]){
// initialize irrlicht - using Direct3D 9
	MyEventReceiver receiver;
	IrrlichtDevice *device =
	createDevice(EDT_DIRECT3D9, core::dimension2d<u32>(800, 600), 16, false, false, true, &receiver);
	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();
	IGUIEnvironment* env = device->getGUIEnvironment();

// get the window handle -- by simply assigning a window name via irrlicht then using FindWindowA()
	device->setWindowCaption(L"Window Switcher - Reivisited");
	HWND hwnd;
	hwnd = FindWindowA(NULL, "Window Switcher - Reivisited");
	SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hwnd, RGB(128,128,128), 0, LWA_COLORKEY);
	RECT screensize;
	GetWindowRect(GetDesktopWindow(), &screensize);

// load the images used as buttons and a backboard
	ITexture *btnbgtexture = driver->getTexture("ressystem/btnBack.png");
	ITexture *btnsBottomtexture = driver->getTexture("ressystem/btnsBottom.png");
	ITexture *btn1texture = driver->getTexture("ressystem/btn1Top.png");
	ITexture *btn2texture = driver->getTexture("ressystem/btn2Top.png");
	xfpslimiter fps;
	fps.timer = device->getTimer();
	bool moved = true;
	bool refocus = false;
	position2di miniPos;
	miniPos.X = 0;
	miniPos.Y = 0;
	HWND win1;
	HWND win2;

// modify the window's flags so that there is no border, no taskbar icon, and set the transparent color to be the same
// color as the transparent pixel loaded from transparentPixel.png later; effectively drawing only the backboard and buttons
	LONG lStyle = GetWindowLong(hwnd, GWL_STYLE);
	lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU | WS_POPUP);
	SetWindowLong(hwnd, GWL_STYLE, lStyle);
	LONG lExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
	ShowWindow(hwnd, SW_HIDE);
	lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE | WS_EX_TOPMOST | WS_EX_TOOLWINDOW);
	SetWindowLong(hwnd, GWL_EXSTYLE, lExStyle);
	SetWindowLong(hwnd, GWL_EXSTYLE, (GetWindowLong(hwnd,GWL_EXSTYLE) | WS_EX_TOOLWINDOW) & ~WS_EX_APPWINDOW);
	SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hwnd, RGB(128,128,128), 0, LWA_COLORKEY);
	ShowWindow(hwnd, SW_SHOW);
	RECT xrect;
	GetWindowRect(hwnd, &xrect);
	SetWindowPos(hwnd, HWND_TOPMOST, xrect.left, xrect.top, xrect.right, xrect.bottom, SWP_NOMOVE | SWP_NOSIZE);

// now we just want to make sure that the window does not start up off-screen, so we check if it is is any way off-screen
// and set it to an appropriate position if so
	{int x = miniPos.X;
	int y = miniPos.Y;
	int right = x + 182;
	int bottom = y + 78;
	if(x < 0){
		x = 0;
	}else if(x > screensize.right){
		x = 0;
	}
	if(right < 0){
		x = 0;
	}else if(right > screensize.right){
		x = screensize.right - 182;
	}
	if(y < 0){
		y = 0;
	}else if(y > screensize.bottom){
		y = 0;
	}
	if(bottom < 0){
		y = 0;
	}else if(bottom > screensize.bottom){
		y = screensize.bottom - 78;
	}
	MoveWindow(hwnd, x, y, 182, 78, true);
	}

// hide the console so that it doesn't appear as a window or as an icon in the taskbar
	ShowWindow(GetConsoleWindow(), SW_HIDE);

// now we load in transparentPixel.png to act as the background which will be drawn but ultimately not shown
// and we're also loading the buttons in and setting them up to be drawn
	IGUIImage *transparentbg = env->addImage(driver->getTexture("ressystem/transparentPixel.png"), vector2d<s32>(0, 0));
	transparentbg->setScaleImage(true);
	transparentbg->setMinSize(dimension2du(182, 78));
	transparentbg->setMaxSize(dimension2du(182, 78));
	IGUIImage *btnbg = env->addImage(btnbgtexture, vector2d<s32>(0, 0));
	IGUIImage *btnsBottom = env->addImage(btnsBottomtexture, vector2d<s32>(0, 0));
	IGUIImage *btn1 = env->addImage(btn1texture, vector2d<s32>(0, 0));
	IGUIImage *btn2 = env->addImage(btn2texture, vector2d<s32>(0, 0));

// only draw when moved == true, which happens only when a button is pressed or released; this minimizes the cpu usage
	moved = true;
	bool mouse = false;
	bool btnpress = false;
	bool moving = false;
	bool getgrab = false;
	char lastpress = 0;
	moved = true;
	vector2d<s32> clickpos;
	POINT winmousepos;
	RECT windowstartpos;

// now we set up 3 rectangles to check for clicks: 1 rectangle for each of the 2 buttons and 1 for the backboard
// when the button is clicked, the corresponding window will be set to foreground
// and if it's the background, the window will move the same distance and direction as the mouse (click-and-drag)
	rect<s32> btn1Zone;
	rect<s32> btn2Zone;
	rect<s32> moveZone;
	moveZone.UpperLeftCorner.X = btnbg->getRelativePosition().UpperLeftCorner.X + 108;
	moveZone.UpperLeftCorner.Y = btnbg->getRelativePosition().UpperLeftCorner.Y + 20;
	moveZone.LowerRightCorner.X = moveZone.UpperLeftCorner.X + 70;
	moveZone.LowerRightCorner.Y = moveZone.UpperLeftCorner.Y + 54;
	btn1Zone.UpperLeftCorner.X = 16;
	btn1Zone.UpperLeftCorner.Y = 24;
	btn1Zone.LowerRightCorner.X = 58;
	btn1Zone.LowerRightCorner.Y = 67;
	btn2Zone.UpperLeftCorner.X = 66;
	btn2Zone.UpperLeftCorner.Y = 24;
	btn2Zone.LowerRightCorner.X = 108;
	btn2Zone.LowerRightCorner.Y = 67;
	while(device->run()){
		fps.nowTime = fps.timer->getTime();
		if(device->isWindowFocused()){
			if(refocus)moved = true; // checks if window lost focus; if it regained focus, update screen
			refocus = false;
		}else{
			refocus = true;
			if(getgrab){
// the on-screen buttons work as follows: (the desired window must not be minimized when clicking into it to give it focus)
// 1. Click button
// 2. Press SPACE while this window (TabSwitcher - Revisited) has focus
// 3. Either ALT+TAB or click into the window you want to set to the button you just pressed
// if the SPACE button was pressed, then set the foreground window to correspond to the last pressed button
				if(lastpress == 0){
					win1 = GetForegroundWindow();
				}else{
					win2 = GetForegroundWindow();
				}
				getgrab = false;
			}
		}
		if(receiver.MouseState.LeftButtonDown){
			if(!mouse){
				mouse = true;
				clickpos = receiver.MouseState.Position;
				if(btn1Zone.isPointInside(clickpos)){ // if button 1 is clicked, make the "UP" position image invisible
					btn1->setVisible(false);
					moved = true;
				}
				if(btn2Zone.isPointInside(clickpos)){ // same as button 1
					btn2->setVisible(false);
					moved = true;
				}
				if(moveZone.isPointInside(clickpos)){ // check if the backboard is being clicked; if yes, prepare for drag
					moving = true;
					GetCursorPos(&winmousepos);
					GetWindowRect(hwnd, &windowstartpos);
				}
			}
		}else if(mouse){
			if(btn1Zone.isPointInside(receiver.MouseState.Position) && btn1Zone.isPointInside(clickpos)){
				if(win1 != NULL){
// bring window 1 set to the foreground if button 1 was released
					if(IsIconic(win1))ShowWindow(win1, SW_RESTORE);
					ShowWindow(win1, SW_SHOW);
					SetForegroundWindow(win1);
				}
				lastpress = 0;
			}
			if(btn2Zone.isPointInside(receiver.MouseState.Position) && btn2Zone.isPointInside(clickpos)){
// bring window 2 set to the foreground if button 2 was released
				if(win2 != NULL){
					if(IsIconic(win2))ShowWindow(win2, SW_RESTORE);
					ShowWindow(win2, SW_SHOW);
					SetForegroundWindow(win2);
				}
				lastpress = 1;
			}
			mouse = false;
			moving = false;
			btn1->setVisible(true);
			btn2->setVisible(true);
			moved = true;
		}
		if(moving){
// if being dragged, move the window by the amount of pixels the mouse has moved
			POINT curpos;
			GetCursorPos(&curpos);
			if(curpos.x != winmousepos.x || curpos.y != winmousepos.y){
				int newx = windowstartpos.left;
				int newy = windowstartpos.top;
				newx += (curpos.x - winmousepos.x);
				newy += (curpos.y - winmousepos.y);
				MoveWindow(hwnd, newx, newy, 182, 78, true);
			}
		}
		if(GetKeyState(VK_UP) & 0x8000 && GetKeyState(VK_MENU) & 0x8000){
// ALT + UP = set button 1 to correspond to the foreground window
			win1 = GetForegroundWindow();
		}
		if(GetKeyState(VK_DOWN) & 0x8000 && GetKeyState(VK_MENU) & 0x8000){
// ALT + DOWN = set button 2 to correspond to the foreground window
			win2 = GetForegroundWindow();
		}
		if(GetKeyState(VK_DELETE) & 0x8000){
			btnpress = true;
		}else if(btnpress){
// When the DELETE key is released, switch between the windows if one of the windows is currently selected.
// Will only work so fast to switch windows; after a certain frequency, Windows decides to not switch the foreground window,
// but to put the would-have-been-set-to-foreground window into an "I have a message for you", blinking state.
// Simply pressing it a second time within the window of time will open the window normally. Strange Windows quirk.
			if(win1 == GetForegroundWindow()){
				SetForegroundWindow(win2);
			}else if(win2 == GetForegroundWindow()){
				SetForegroundWindow(win1);
			}
			btnpress = false;
		}
// terminate the program via ESCAPE button while this window (TabSwitcher - Revisited) has focus
		if(receiver.IsKeyDown(KEY_ESCAPE))break;
// prepare to grab the window via the on-screen buttons
		if(receiver.IsKeyDown(KEY_SPACE))getgrab = true;
		if(moved){
			driver->beginScene(false, false, SColor(255, 228, 128, 28));
			env->drawAll();
			driver->endScene();
			moved = false;
		}else{
			device->yield();
		}
		fps.sdlwait();
	}
	device->drop();
}