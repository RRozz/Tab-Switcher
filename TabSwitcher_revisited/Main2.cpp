using namespace std;
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
class MyEventReceiver : public IEventReceiver // the IEventReceiver is responsible for catching input
{
public:
    struct SMouseState
    {
        core::position2di Position;
        bool LeftButtonDown;
        bool RightButtonDown;
		float wheel;
        SMouseState() : LeftButtonDown(false), wheel(7) { }
    } MouseState;
    // This is the one method that we have to implement
    virtual bool OnEvent(const SEvent& event)
    {
        // Remember whether each key is down or up
        if (event.EventType == irr::EET_KEY_INPUT_EVENT)
            KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
        if (event.EventType == irr::EET_MOUSE_INPUT_EVENT)
        {
            switch(event.MouseInput.Event)
            {
            case EMIE_LMOUSE_PRESSED_DOWN:
                MouseState.LeftButtonDown = true;
                break;

            case EMIE_LMOUSE_LEFT_UP:
                MouseState.LeftButtonDown = false;
                break;

            case EMIE_RMOUSE_PRESSED_DOWN:
                MouseState.RightButtonDown = true;
                break;

            case EMIE_RMOUSE_LEFT_UP:
                MouseState.RightButtonDown = false;
                break;

            case EMIE_MOUSE_MOVED:
                MouseState.Position.X = event.MouseInput.X;
                MouseState.Position.Y = event.MouseInput.Y;
                break;

			case EMIE_MOUSE_WHEEL:
				MouseState.wheel += event.MouseInput.Wheel;
				break;

            default:
                // We won't use the wheel
                break;
            }
        }
		//if(event.EventType == irr::EET_MOUSE_INPUT_EVENT)
			//KeyIsDown[event.MouseInput.ButtonStates] = event.MouseInput.isLeftPressed;
		//if(event.EventType == irr::EET_MOUSE_INPUT_EVENT)
			//KeyIsDown[event.MouseInput.ButtonStates] = event.MouseInput.Event;

        return false;
    }

    // This is used to check whether a key is being held down
    virtual bool IsKeyDown(EKEY_CODE keyCode) const
    {
        return KeyIsDown[keyCode];
    }

	virtual bool ControlKeyCheck(int keyint) const{
		return KeyIsDown[keyint];
	}
    
    MyEventReceiver()
    {
        for (u32 i=0; i<KEY_KEY_CODES_COUNT; ++i)
            KeyIsDown[i] = false;
    }

private:
    // We use this array to store the current state of each key
    bool KeyIsDown[KEY_KEY_CODES_COUNT];
};
class xfpslimiter{ // limits frame rate to maintain low cpu usage
	//
	int MaxFPS;
	int MaxSDLFPS;
	int FPS;
public:
	int nowTime;
	xfpslimiter();
	void sdlwait();
	irr::ITimer *timer;
};
xfpslimiter::xfpslimiter(){
	//
	MaxFPS = 30;
	MaxSDLFPS = 30;
	FPS = 0;
	nowTime = 0;
}
void xfpslimiter::sdlwait(){
	if(timer->getTime() - nowTime < 1000/MaxFPS){
		FPS = timer->getTime() - nowTime;
		if(FPS == 0){
			Sleep(1000/MaxSDLFPS);
		}else{
			Sleep(int((int((1000/FPS)))/MaxSDLFPS));
		}
	}
}