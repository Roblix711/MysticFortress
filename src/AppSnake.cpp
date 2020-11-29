
// main header
#include <yse/yse.hpp>
#include <RazorDraw.hpp>
#include <MathExt.hpp>
#include <EngineXG4.hpp>
#include "AppSnake.h"
#include "Color.h"
#include "DX9MAT.h"

// prettyness
#define MICROSOFT
#define NOCODE

// linker
#pragma comment(lib, "libyse64.lib")


RAZOR::ENGINE* APP_SNAKE::RazorEngine = nullptr;
const UINT32 MAX_GRID_X = 75;
const UINT32 MAX_GRID_Y = 50;

// pair of points
struct INTPAIR
{
    INT32 x;
    INT32 y;
};

// GLOBALS for snake
class SNAKE
{
    public:
    VECTOR<INTPAIR> Body;
    INT32 Direction;
    INT32 NextDirection;
    INT32 FoodsEaten;
    INT32 Hue;

    void MoveSnake()
    {
        VECTOR<INTPAIR> NewBody = Body;
        if (FoodsEaten == 0)
        {
            NewBody.erase( NewBody.begin() );
        }
        else
        {
            FoodsEaten--;
        }

        // now, we need to add a "NEW" element
        INTPAIR LastElement = NewBody.back();
        switch(NextDirection)
        {
            case 0: NewBody.push_back( {LastElement.x+1, LastElement.y} ); break;
            case 1: NewBody.push_back( {LastElement.x, LastElement.y-1} ); break;
            case 2: NewBody.push_back( {LastElement.x-1, LastElement.y} ); break;
            case 3: NewBody.push_back( {LastElement.x, LastElement.y+1} ); break;
            default: break;
        }
        Direction = NextDirection;
        Body = NewBody;
    }
};

struct FOOD
{
    INT32 x, y;
    INT32 Quality;
    INT32 Score;

    void NewFood()
    {
        x = rand() % MAX_GRID_X;
        y = rand() % MAX_GRID_Y;
        Quality = (rand() % 50) < 10 ? 1 : 0;
        Score = 10 + rand() % 10;

        if (Quality == 1) // bad
            Score *= -1;    
    }
};

class FOOD_MANAGER
{
    public:
    VECTOR<FOOD> ListFood;
    INT32 MaxFood;

    void InitFood(INT32 NumFood)
    {
        // init foods
        MaxFood = NumFood;

        // make random locations for food
        ListFood.resize(MaxFood);
        for(TSIZE i=0; i<ListFood.size(); ++i)
        {
            ListFood[i].NewFood();
        }
    }
};


struct SPIDER
{
    INT32 X;
    INT32 Y;
    INT32 MaxY;
    INT32 State;
    BOOLEEN Grapple;
    FOOD* TargetFood;
};

class ENEMIES_MANAGER
{
    public:
    VECTOR<SPIDER> Spiders;

    void AddSpider()
    {
        // recycle spiders
        for(TSIZE i=0; i<Spiders.size(); ++i)
        {
            if (Spiders[i].State == 2)
            {
                Spiders[i] = {rand() % MAX_GRID_X, 0, rand() % (MAX_GRID_Y - 5), 0, false, nullptr };
                return;
            }
        }
        Spiders.push_back( {rand() % MAX_GRID_X, 0, rand() % (MAX_GRID_Y - 5), 0, false, nullptr } );
    }

};

ENEMIES_MANAGER Enemies;
FOOD_MANAGER MasterFood;
SNAKE Snake;

// game state
class GAME_STATE
{
    public:
    INT32 Score;
    BOOLEEN GameOver;
    DX9MAT *FoodSprite;
    DX9MAT *BadFoodSprite;
    DX9MAT *Background;
    DX9MAT *Spider;
    YSE::sound BackgroundSound;
    YSE::sound ChompSound;
    YSE::sound Scuttle;
    YSE::sound Bite;
    YSE::sound Squish;


    void NewGame()
    {
        // init food manager
        MasterFood.InitFood(50);
        Snake.Body.clear();
        Snake.Body.push_back( {15,15} );
        Snake.Body.push_back( {16,15} );
        Snake.Body.push_back( {17,15} );
        Snake.Body.push_back( {18,15} );
        Snake.Body.push_back( {19,15} );
        Snake.Body.push_back( {20,15} );
        Snake.Direction = 0;
        Snake.NextDirection = 0;
        Snake.FoodsEaten = 0;    
        Snake.Hue = rand()%360;
        GameOver = false;
        Score = 0;
        
    }

    void LoadSprite()
    {

        FoodSprite    = new DX9MAT(APP_SNAKE::RazorEngine, APP_SNAKE::RazorEngine->PathMakeAbsolute("assets/images/apple.png"));
        BadFoodSprite = new DX9MAT(APP_SNAKE::RazorEngine, APP_SNAKE::RazorEngine->PathMakeAbsolute("assets/images/apple_bad.png"));
        Background    = new DX9MAT(APP_SNAKE::RazorEngine, APP_SNAKE::RazorEngine->PathMakeAbsolute("assets/images/background.png"));
        Spider        = new DX9MAT(APP_SNAKE::RazorEngine, APP_SNAKE::RazorEngine->PathMakeAbsolute("assets/images/spider.png"));

        // audio samples
        Squish          .create(APP_SNAKE::RazorEngine->PathMakeAbsolute("assets/audio/squish.wav")    .c_str(), &YSE::ChannelFX(), false);
        Bite            .create(APP_SNAKE::RazorEngine->PathMakeAbsolute("assets/audio/bite.wav")      .c_str(), &YSE::ChannelFX(), false);
        Scuttle         .create(APP_SNAKE::RazorEngine->PathMakeAbsolute("assets/audio/scuttle.wav")   .c_str(), &YSE::ChannelFX(), false);
        ChompSound      .create(APP_SNAKE::RazorEngine->PathMakeAbsolute("assets/audio/chomp.wav")     .c_str(), &YSE::ChannelFX(), false);
        BackgroundSound .create(APP_SNAKE::RazorEngine->PathMakeAbsolute("assets/audio/background.wav").c_str(), NULL, true);
        BackgroundSound.play();

        //Squish.setVolume(200);

    }
};

GAME_STATE Game;

void APP_SNAKE::Tick()    
{ 
    YSE::System().update();

    // increment counter
    static UINT32 Counter = 0; 
    Counter++;

    RAZOR::XBOX_CONTROLLER_STATE XboxState;
    BOOLEEN bXbox = RazorEngine->GetXboxState(0, XboxState);

    if (bXbox == true)
    {
        if (XboxState.ButtonLeft  == true) if (Snake.Direction != 0) Snake.NextDirection = 2;
        if (XboxState.ButtonRight == true) if (Snake.Direction != 2) Snake.NextDirection = 0;
        if (XboxState.ButtonUp    == true) if (Snake.Direction != 1) Snake.NextDirection = 1;
        if (XboxState.ButtonDown  == true) if (Snake.Direction != 3) Snake.NextDirection = 3;

        if (Game.GameOver == true)
        {
            if (XboxState.ButtonA == true) // enter
            {
                Game.NewGame();
            }
        }

    }


    // check every second
    if (Game.GameOver == false)
    {
        if (Counter % 10 == 0)
        {
            for(TSIZE i=0; i<Enemies.Spiders.size(); ++i)
            {
                if (Enemies.Spiders[i].State == 0)
                {
                    if (Enemies.Spiders[i].Y < Enemies.Spiders[i].MaxY)
                    {
                        Enemies.Spiders[i].Y++;
                    }
                    else
                    {
                        Enemies.Spiders[i].State = 1;
                    }
                }
                else if (Enemies.Spiders[i].State == 1)
                {
                    if (Enemies.Spiders[i].Y > 0)
                    {
                        if (Enemies.Spiders[i].Grapple == true)
                        {
                            Enemies.Spiders[i].TargetFood->y--;
                        }
                        Enemies.Spiders[i].Y--;
                    }
                    else
                    {
                        if (Enemies.Spiders[i].Grapple == true)
                        {
                            Enemies.Spiders[i].TargetFood->NewFood();
                            Enemies.Spiders[i].Grapple = false;
                        }
                        Enemies.Spiders[i].State = 2;
                    }

                    

                }
            }

            // check collision with apple
            for(TSIZE i=0; i<Enemies.Spiders.size(); ++i)
            for(TSIZE j=0; j<MasterFood.ListFood.size(); ++j)
            {
                if (Enemies.Spiders[i].Grapple == false)
                if (Enemies.Spiders[i].X == MasterFood.ListFood[j].x)
                if (Enemies.Spiders[i].Y == MasterFood.ListFood[j].y)
                {
                    Enemies.Spiders[i].State = 1;
                    Enemies.Spiders[i].Grapple = true;
                    Enemies.Spiders[i].TargetFood = &MasterFood.ListFood[j];

                    // play bite
                    UINT32 Length = Game.Bite.getLength();
                    Game.Bite.setTime(Length*0.10);
                    Game.Bite.play();
                }
            }
        }

        UINT32 SpeedModifier = 15;
        if (RazorEngine->GetKeyboardStateCSA().Shift == true || ((bXbox == true) && XboxState.ButtonB == true))
        {
            SpeedModifier = 8;
            Game.Score --;
        }

        if (Counter % SpeedModifier == 0)
        {
            // move the snake
            Snake.MoveSnake();

            // check if snake is in same square as food
            for(TSIZE i=0; i<Snake.Body.size(); ++i)
            {
                for(TSIZE j=0; j<MasterFood.ListFood.size(); ++j)
                {
                    if (Snake.Body[i].x == MasterFood.ListFood[j].x)
                    if (Snake.Body[i].y == MasterFood.ListFood[j].y)
                    {
                        // increase length by 1
                        Snake.FoodsEaten++;
                        Game.Score += MasterFood.ListFood[j].Score;

                        // eat food
                        MasterFood.ListFood[j].NewFood();

                        // play sound
                        Game.ChompSound.setTime(0);
                        Game.ChompSound.play();

                        if (rand() % 50 < 25)
                        {
                            Enemies.AddSpider();
                            UINT32 Length = Game.Scuttle.getLength();
                            Game.Scuttle.setTime(Length*0.33);
                            Game.Scuttle.play();
                        }

                    }
                }
            }

            // check if snake is colliding with itself
            for(TSIZE i=0;   i<Snake.Body.size(); ++i)
            for(TSIZE j=i+1; j<Snake.Body.size(); ++j)
            {
                // check if body collides
                if (Snake.Body[i].x == Snake.Body[j].x)
                if (Snake.Body[i].y == Snake.Body[j].y)        
                {
                    Game.GameOver = true;
                    Game.Squish.play();
                }
            }

            // check if snake is outside the bounds
            for(TSIZE i=0; i<Snake.Body.size(); ++i)
            {
                if (Snake.Body[i].x < 0)         {Game.Squish.play(); Game.GameOver = true;}
                if (Snake.Body[i].y < 0)         {Game.Squish.play(); Game.GameOver = true;}
                if (Snake.Body[i].x >= MAX_GRID_X) {Game.Squish.play(); Game.GameOver = true;}
                if (Snake.Body[i].y >= MAX_GRID_Y) {Game.Squish.play(); Game.GameOver = true;}
            }

            // check spider and snake collisions
            for(TSIZE i=0; i<Snake.Body.size(); ++i)
            for(TSIZE j=0; j<Enemies.Spiders.size(); ++j)
            {
                if ( Snake.Body[i].x == Enemies.Spiders[j].X)
                if ( Snake.Body[i].y <= Enemies.Spiders[j].Y)
                {
                    Game.GameOver = true;
                    Game.Squish.play();
                }
            }

        }
    }
}


// main drawing procedure
void APP_SNAKE::Draw()
{
    // clear screen for refresh -> can be omitted if smart
    RazorEngine->DrawClearScreen( 0 );

    // draw all
    RazorEngine->DrawCircle( RazorEngine->GetMouseX(), RazorEngine->GetMouseY(), 15, ColorMakeRGBA(255,0,0,255), false) ;

    // now draw the memory footprint
    if (RazorEngine->GetKeyboardState().FKeys.F[0] == true)
    {
        auto Footprints = RazorEngine->GetSystemMetricsAverage();
        VECTOR<DOUBLE> MemoryFootprint = Footprints["Memory"];
        VECTOR<DOUBLE> CPUFootprint    = Footprints["CPU"];
        VECTOR<DOUBLE> LoopTimers      = Footprints["Loop"];
        RazorEngine->DrawTextsF(2, 2, cGreen, DT_LEFT | DT_TOP, false, "Draw[%3.2lf]", 1000.0/LoopTimers[0]);
    }

    // draw a grid
    Game.Background->DrawMat(0, 0, 1, 1);
    DOUBLE GridY = RazorEngine->GetScreenHeight()*1.0/MAX_GRID_Y;
    DOUBLE GridX = RazorEngine->GetScreenWidth()*1.0/MAX_GRID_X;

    // draw the horizontal lines
    for(DOUBLE i=0; i<RazorEngine->GetScreenHeight(); i += GridY)
    {
        //RazorEngine->DrawLine(0, round(i), RazorEngine->GetScreenWidth(), round(i), cVDGray);
    }

    // draw the Vertical lines
    for(DOUBLE i=0; i<RazorEngine->GetScreenWidth(); i += GridX)
    {
        //RazorEngine->DrawLine(round(i), 0, round(i), RazorEngine->GetScreenHeight(), cVDGray);
    }

    // draw the foods
    for(INT32 i=0; i<MasterFood.ListFood.size(); ++i)
    {
        //INT32 CenterX = round((MasterFood.ListFood[i].x + 0.5)*GridX);
        //INT32 CenterY = round((MasterFood.ListFood[i].y + 0.5)*GridY);
        //RazorEngine->DrawCircle(CenterX, CenterY, std::min(GridX, GridY)/2.0, cGreen, true); 

        INT32 CenterX = round((MasterFood.ListFood[i].x)*GridX);
        INT32 CenterY = round((MasterFood.ListFood[i].y)*GridY);

        if (MasterFood.ListFood[i].Quality == 0) Game.FoodSprite->DrawMatAlpha(CenterX-4, CenterY-4, 0.035, 0.035, cWhite);
        else                                     Game.BadFoodSprite->DrawMatAlpha(CenterX-4, CenterY-4, 0.035, 0.035, cWhite);
    }

    // draw the snake
    for(INT32 i=0; i<Snake.Body.size(); ++i)
    {
        BOOLEEN bLast = (i == Snake.Body.size()-1);
        //RazorEngine->DrawRect( round(Snake.Body[i].x*GridX)    , round(Snake.Body[i].y*GridY), 
        ///                        round((Snake.Body[i].x+1)*GridX) , round((Snake.Body[i].y+1)*GridY), 
        //                        bLast ? cDRed : cRed, true); 

        INT32 CenterX = round((Snake.Body[i].x+0.5)*GridX);
        INT32 CenterY = round((Snake.Body[i].y+0.5)*GridY);

        float R,G,B;
        float H,S,V;

        HSVtoRGB(R, G, B, Snake.Hue, 1, 1);

        RGBtoHSV(R, G, B, H, S, V);
        H += 10*sin(2*M_PI*i/6.0);
        if (H < 0) H += 360;
        V = 0.6 + 0.4*sin(2*M_PI*i/13.32425);
        HSVtoRGB(R, G, B, H, S, V);

        // variation 
        //R /= 1.5 + 0.5*sin(2*M_PI*i/12.0);
        //G /= 1.5 + 0.5*sin(2*M_PI*i/12.0);
        //B /= 1.5 + 0.5*sin(2*M_PI*i/12.0);
        
        RazorEngine->DrawCircle(CenterX, CenterY, 2 + std::min(GridX, GridY)/2.0 + (i==(Snake.Body.size()-1) ? 3 : 0) + (i==0 ? -2 : 0), ColorMakeRGBA(R*255, B*255, G*255, 255), true); 
    }

    // DRAW SOME antenna and eyes
    INTPAIR Head = Snake.Body.back();
    INT32 HeadX = round((Head.x+0.5)*GridX);
    INT32 HeadY = round((Head.y+0.5)*GridY);
    RazorEngine->DrawCircle(HeadX + GridX/3*cos(Snake.Direction*M_PI/2) + GridX/3*cos(Snake.Direction*M_PI/2 + M_PI/2), HeadY - GridY/3*sin(Snake.Direction*M_PI/2) - GridY/3*sin(Snake.Direction*M_PI/2 + M_PI/2), 3, cWhite, true);
    RazorEngine->DrawCircle(HeadX + GridX/3*cos(Snake.Direction*M_PI/2) + GridX/3*cos(Snake.Direction*M_PI/2 - M_PI/2), HeadY - GridY/3*sin(Snake.Direction*M_PI/2) - GridY/3*sin(Snake.Direction*M_PI/2 - M_PI/2), 3, cWhite, true);

    RazorEngine->DrawCircle(HeadX + GridX/3*cos(Snake.Direction*M_PI/2) + GridX/3*cos(Snake.Direction*M_PI/2 + M_PI/2), HeadY - GridY/3*sin(Snake.Direction*M_PI/2) - GridY/3*sin(Snake.Direction*M_PI/2 + M_PI/2), 2, cBlack, true);
    RazorEngine->DrawCircle(HeadX + GridX/3*cos(Snake.Direction*M_PI/2) + GridX/3*cos(Snake.Direction*M_PI/2 - M_PI/2), HeadY - GridY/3*sin(Snake.Direction*M_PI/2) - GridY/3*sin(Snake.Direction*M_PI/2 - M_PI/2), 2, cBlack, true);

    // draw enemies
    for(INT32 i=0; i<Enemies.Spiders.size(); ++i)
    {
        if (Enemies.Spiders[i].State == 0 || Enemies.Spiders[i].State == 1)
        {
            INT32 CenterX = round((Enemies.Spiders[i].X+0.5)*GridX);
            INT32 CenterY = round((Enemies.Spiders[i].Y+0.5)*GridY);
            INT32 LeftX = round((Enemies.Spiders[i].X+0.5)*GridX) - Game.Spider->Width*0.1/2.0;
            INT32 LeftY = round((Enemies.Spiders[i].Y+0.5)*GridY) - Game.Spider->Height*0.1/2.0;
            RazorEngine->DrawLine( CenterX, CenterY, CenterX, 0, cGray, 5);
            Game.Spider->DrawMatAlpha( LeftX, LeftY, 0.1, 0.1, cWhite);
        }
    }
    
    
    if (Game.GameOver == true)
    {
        RazorEngine->DrawTextsF("Arial", 56, RazorEngine->GetScreenWidth()/2, RazorEngine->GetScreenHeight()/2, cYellow, DT_CENTER | DT_VCENTER, false, "GAME OVER!!");
        RazorEngine->DrawTextsF("Arial", 32, RazorEngine->GetScreenWidth()/2, RazorEngine->GetScreenHeight()/2+25, cTeal, DT_CENTER | DT_TOP, false, "Score: %d", Game.Score);

    }

    RazorEngine->DrawTextsF("Consolas", 32, RazorEngine->GetScreenWidth()-15, 5, cRed, DT_RIGHT | DT_TOP, false, "Score: %d", Game.Score);

}

// destroy resources
void APP_SNAKE::ResDest()
{

}

// initialize resources
void APP_SNAKE::ResInit() 
{
    
    Game.LoadSprite();
    Game.NewGame();

}


// callbacks for some other less used events
void APP_SNAKE::OnLost()  {  }
void APP_SNAKE::OnReset() {  }
void APP_SNAKE::Frame()   {  }


// proxy to engine
void APP_SNAKE::dprintf(const CHAR *message, ...)
{
    // start variable arguments
    RAZOR::VAR_LIST args;
    va_start(args, message);
    RazorEngine->ConsoleLogV(message, args);   
    va_end(args);
}

void APP_SNAKE::MouseLeftUp()
{

}

void APP_SNAKE::MouseLeftDown()
{
    INT32 Mx = RazorEngine->GetMouseX();
    INT32 My = RazorEngine->GetMouseY();
}

// process some events
void APP_SNAKE::MouseRightUp()   
{       
                 
}
void APP_SNAKE::MouseRightDown() 
{    
}


void APP_SNAKE::MouseMiddle()
{

}

void APP_SNAKE::GlobalScrollDown()
{

}

void APP_SNAKE::GlobalScrollUp()
{

}

// called on keyboard
void APP_SNAKE::GlobalKeyboardAny(UINT32 KeyCode, BOOLEEN State)
{
    //printf("%d %c %d\n", KeyCode, KeyCode, State);

    // on down
    if (State == 1) 
    {
        switch(KeyCode)
        {
            case ENGINEXG4::ENUMS::KEY_W:
            case ENGINEXG4::ENUMS::KEY_UP:      
                if (Snake.Direction != 3) Snake.NextDirection = 1; break;

            case ENGINEXG4::ENUMS::KEY_S:    
            case ENGINEXG4::ENUMS::KEY_DOWN:    
                if (Snake.Direction != 1) Snake.NextDirection = 3; break;

            case ENGINEXG4::ENUMS::KEY_A:   
            case ENGINEXG4::ENUMS::KEY_LEFT:   
                if (Snake.Direction != 0) Snake.NextDirection = 2; break;

            case ENGINEXG4::ENUMS::KEY_D:   
            case ENGINEXG4::ENUMS::KEY_RIGHT:   
                if (Snake.Direction != 2) Snake.NextDirection = 0; break;

            case ENGINEXG4::ENUMS::KEY_ESCAPE:
                RazorEngine->EngineStop();
            default: break;
        }
    }
    if (Game.GameOver == true)
    {
        if (KeyCode == ENGINEXG4::ENUMS::KEY_ENTER) // enter
        {
            Game.NewGame();
        }
    }
}

// called when drag/dropped with a list of paths (files / folders)
void APP_SNAKE::OnDragDrop(const VECTOR<STRING> &Paths)
{

}

// start
void APP_SNAKE::Start(MICROSOFT::HINSTANCE hInstance, RAZOR::WINDOWS_CALLBACK WndProc)
{
    YSE::System().init();

    // initialize the razor engine // use default events -> add functions here for mouse / keyboard callbacks
    RAZOR::INPUT_CALLBACKS Events; 
    Events.KeyboardAny      = &APP_SNAKE::GlobalKeyboardAny;
    Events.MouseLeftUp      = &APP_SNAKE::MouseLeftUp;
    Events.MouseLeftDown    = &APP_SNAKE::MouseLeftDown;
    Events.MouseRightUp     = &APP_SNAKE::MouseRightUp;
    Events.MouseRightDown   = &APP_SNAKE::MouseRightDown;
    Events.MouseMiddleAny   = &APP_SNAKE::MouseMiddle;  
    Events.MouseScrollDown  = &APP_SNAKE::GlobalScrollDown; 
    Events.MouseScrollUp    = &APP_SNAKE::GlobalScrollUp;
    RAZOR::FUNCTION_LINKS ExLinks = { &APP_SNAKE::Draw, &APP_SNAKE::Tick, &APP_SNAKE::Frame, &APP_SNAKE::ResInit, &APP_SNAKE::ResDest, &APP_SNAKE::OnLost, &APP_SNAKE::OnReset, WndProc };
    RAZOR::WINDOW_PARAMS  WinParams = { "Snake & Spider", 1200, 800, 60, 100, true, true, 0, true, &APP_SNAKE::OnDragDrop };

    // initialize
    RazorEngine = new RAZOR::ENGINE(hInstance);
    RazorEngine->EngineInitialize( {&ExLinks, &WinParams, &Events} );
    
    // okay, write something to console for fun
    dprintf("EngineXG4 Version %s\n", "1.00");
    RazorEngine->StartAll();

    // start main loop here -> this is a blocking call
    RazorEngine->EngineMainLoop();

    YSE::System().close();

    // delete our engine
    delete RazorEngine;
}

