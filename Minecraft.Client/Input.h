#pragma once
#ifdef _WINDOWS64
extern bool g_Sprint;
#endif

class Player;

class Input
{
public:
	float xa;
    float ya;

    bool wasJumping;
    bool jumping;
    bool sneaking;
	bool sprinting;

	Input();

    virtual void tick(LocalPlayer *player);

private:
    bool lReset;
    bool rReset;
};
