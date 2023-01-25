#include <iostream>
#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h> 
#include <string.h>
#include <sstream>
#include "raylib.h"

#define MAX_DIGITS 10000

static void DrawTextBoxed(Font font, const char* text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint);
static void DrawTextBoxedSelectable(Font font, const char* text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint, int selectStart, int selectLength, Color selectTint, Color selectBackTint);
int callback(void* data, int argc, char** argv, char** azColName);
int callback2(void* data, int argc, char** argv, char** azColName);

using namespace std;

// global variables
string MyString[10][10];    // MyString[id][row] =---> id correspond the row number in the table
string MyString2[10][10];  
int id = 0;
int id2 = 0;

static void AudioProcessEffectLPF(void* buffer, unsigned int frames)
{
    static float low[2] = { 0.0f, 0.0f };
    static const float cutoff = 70.0f / 44100.0f; // 70 Hz lowpass filter
    const float k = cutoff / (cutoff + 0.1591549431f); // RC filter formula

    for (unsigned int i = 0; i < frames * 2; i += 2)
    {
        float l = ((float*)buffer)[i], r = ((float*)buffer)[i + 1];
        low[0] += k * (l - low[0]);
        low[1] += k * (r - low[1]);
        ((float*)buffer)[i] = low[0];
        ((float*)buffer)[i + 1] = low[1];
    }
}

static float* delayBuffer = NULL;
static unsigned int delayBufferSize = 0;
static unsigned int delayReadIndex = 2;
static unsigned int delayWriteIndex = 0;

// Audio effect: delay
static void AudioProcessEffectDelay(void* buffer, unsigned int frames)
{
    for (unsigned int i = 0; i < frames * 2; i += 2)
    {
        float leftDelay = delayBuffer[delayReadIndex++];    // ERROR: Reading buffer -> WHY??? Maybe thread related???
        float rightDelay = delayBuffer[delayReadIndex++];

        if (delayReadIndex == delayBufferSize) delayReadIndex = 0;

        ((float*)buffer)[i] = 0.5f * ((float*)buffer)[i] + 0.5f * leftDelay;
        ((float*)buffer)[i + 1] = 0.5f * ((float*)buffer)[i + 1] + 0.5f * rightDelay;

        delayBuffer[delayWriteIndex++] = ((float*)buffer)[i];
        delayBuffer[delayWriteIndex++] = ((float*)buffer)[i + 1];
        if (delayWriteIndex == delayBufferSize) delayWriteIndex = 0;
    }
}

int main(void)
{
    sqlite3* db; // This is a container for your database
    char* GetError = 0;
    string sql;

    //int sqlite3_open(const char* filename, sqlite3 **ppDB);
    int sq = sqlite3_open("\myStoreItems.db", &db); // If exsited, open; if not exsited, create.

    if (sq != SQLITE_OK) // if sq is not false
    {
        printf("I couldn't created or open the database!\n");
    }
    else
    {
        printf("Database created or open with success!\n");
    }

    sql = "SELECT * from ItemsInStore";

    int recoverData = sqlite3_exec(db, sql.c_str(), callback, 0, &GetError);

    if (recoverData != SQLITE_OK)
    {
        //printf("The SELECT works\n");
        printf("Error: %s\n", GetError);
    }
    else
    {
        printf("The SELECT works\n");
        //printf("Error: %s\n", GetError);
    }

    sqlite3_close(db);

    sqlite3* db2;
    char* GetError2 = 0;
    string sql2;

    int sq2 = sqlite3_open("\databaseForBag.db", &db2); 

    if (sq2 != SQLITE_OK) 
    {
        printf("I couldn't created or open the database!\n");
    }
    else
    {
        printf("Database created or open with success!\n");
    }

    /*sql2 = "INSERT INTO ItmesInBag VALUES (1,'Peter','TJ',29);";
    int insertData = sqlite3_exec(db, sql.c_str(), NULL, 0, &GetError);

    if (insertData != SQLITE_OK)
    {
        printf("Error: %s\n", GetError);
    }
    else
    {
        printf("Data was inserted with success!\n");
    }*/

    sql2 = "SELECT * from ItmesInBag";

    int recoverData2 = sqlite3_exec(db2, sql2.c_str(), callback2, 0, &GetError2);

    if (recoverData2 != SQLITE_OK)
    {
        printf("Error: %s\n", GetError2);
    }
    else
    {
        printf("The SELECT works\n");
    }


    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 628;
    const int screenHeight = 453;

    InitWindow(screenWidth, screenHeight, "Pokemon Store");

    InitAudioDevice();              // Initialize audio device

    Music music = LoadMusicStream("Audios/Shop.mp3");

    Sound getItem = LoadSound("Audios/GetItem.mp3");
    Sound removeItem = LoadSound("Audios/RemoveItem.mp3");
    SetSoundVolume(getItem, 1.0f);
    PlayMusicStream(music);

    bool pause = false;

    Texture2D PokemonStoreTexture = LoadTexture("Images/PokemonMart.png");        // Texture loading

    Texture2D rightActor = LoadTexture("Images/rightActor.png");
    Texture2D leftActor = LoadTexture("Images/leftActor.png");
    Texture2D upActor = LoadTexture("Images/upActor.png");
    Texture2D downActor = LoadTexture("Images/downActor.png");
    Texture2D clert = LoadTexture("Images/clert.png");
    Vector2 position = { 200.0f, 380.0f };

    Rectangle boxCharacter = { position.x, position.y, 50, 65 };
    Rectangle box1 = { 0, 0, 625, 135 };
    Rectangle box2 = { 580, 0, 60, 453 };
    Rectangle box3 = { 0, 450, 628, 2 };
    Rectangle box4 = { 0, 0, 2, 453 };
    Rectangle box5 = { 168, 0, 2, 280 };
    Rectangle box6 = { 0, 278, 170, 2 };
    Rectangle box7 = { 350, 230, 100, 2 };
    Rectangle box8 = { 350, 230, 2, 150 };
    Rectangle box9 = { 350, 390, 100, 2 };
    Rectangle box10 = { 450, 230, 2, 150 };
    //Rectangle box11 = { 170, 160, 70, 70 };

    Rectangle container = { 15.0f, 200.0f, 600, 250 };
    Rectangle containerForMoney = { 15.0f, 15.0f, 150, 50 };
    Color borderColor = MAROON;
    Color borderColorForMoney = RED;
    Font font = GetFontDefault();

    char textForBag[] = "Bag: \n";
    const char textForClert[] = "What do you want to buy? Press Num (ex: 1 is Postion)\n";
    const char textForClert2[] = "Do you want to buy or sell?\nPress KEY B for buy AND Press KEY S for sell.\n";
    bool wordWrap = true;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    
    bool right = false;
    bool left = false;
    bool down = false;
    bool up = true;

    bool showBoxForBag = false;
    bool AskForSellOrBuy = false;
    bool sell = false;
    bool buy = false;
    bool showTextNotEnough = false;

    bool buyFirstItem = false;
    bool buySecondItem = false;
    bool buyThirdItem = false;

    int firstItem = 0;
    int secondItem = 0;
    int thirdItem = 0;

    string money = "1000";
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    { 
        UpdateMusicStream(music);

        if (IsKeyPressed(KEY_P))
        {
            pause = !pause;

            if (pause) PauseMusicStream(music);
            else ResumeMusicStream(music);
        }

        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_RIGHT))
        {
            position.x += 2.0f;
            boxCharacter.x += 2.0f;

            right = true;
            left = false;
            up = false;
            down = false;
        }

        else if (IsKeyDown(KEY_LEFT))
        {
            position.x -= 2.0f;
            boxCharacter.x -= 2.0f;

            right = false;
            left = true;
            up = false;
            down = false;
        }

        else if (IsKeyDown(KEY_UP))
        {
            position.y -= 2.0f;
            boxCharacter.y -= 2.0f;

            right = false;
            left = false;
            up = true;
            down = false;
        }

        else if (IsKeyDown(KEY_DOWN))
        {
            position.y += 2.0f;
            boxCharacter.y += 2.0f;

            right = false;
            left = false;
            up = false;
            down = true;
        }

        BeginDrawing();
        //ClearBackground(RAYWHITE);

        DrawTexture(PokemonStoreTexture, screenWidth / 2 - PokemonStoreTexture.width / 2, screenHeight / 2 - PokemonStoreTexture.height / 2, WHITE);
        //DrawTexture(PokemonStoreTexture, 0, 0, { 255, 255, 255, 255 });
        DrawTextureEx(clert, { 69.0f, 135.0f }, 0.0f, 0.7f, WHITE);
        DrawText("Press ENTER to open bag!", 175, 10, 20, RED);
        DrawText("Press Z key to talk to the clerk!", 150, 35, 20, RED);
        DrawText("Press X to close bag or cancel to talk to clerk!", 100, 60, 20, RED);
        DrawText("Press P to pause/remuse music!", 155, 85, 20, RED);

        //DrawRectangleRec(box1, {0, 0, 0, 0});
        DrawRectangleRec(box1, { 0, 0, 0, 0 });
        DrawRectangleRec(box2, { 0, 0, 0, 0 });
        DrawRectangleRec(box3, { 0, 0, 0, 0 });
        DrawRectangleRec(box4, { 0, 0, 0, 0 });
        DrawRectangleRec(box5, { 0, 0, 0, 0 });
        DrawRectangleRec(box6, { 0, 0, 0, 0 });
        DrawRectangleRec(box7, { 0, 0, 0, 0 });
        DrawRectangleRec(box8, { 0, 0, 0, 0 });
        DrawRectangleRec(box9, { 0, 0, 0, 0 });
        DrawRectangleRec(box10, { 0, 0, 0, 0 });
        //DrawRectangleRec(box11, GOLD);
        DrawRectangleRec(boxCharacter, { 0, 0, 0, 0 });

        if (CheckCollisionRecs(box1, boxCharacter))
        {
            position.y += 2.0f;
            boxCharacter.y += 2.0f;
        }

        if (CheckCollisionRecs(box2, boxCharacter))
        {
            position.x -= 2.0f;
            boxCharacter.x -= 2.0f;
        }

        if (CheckCollisionRecs(box3, boxCharacter))
        {
            position.y -= 2.0f;
            boxCharacter.y -= 2.0f;
        }

        if (CheckCollisionRecs(box4, boxCharacter))
        {
            position.x += 2.0f;
            boxCharacter.x += 2.0f;
        }

        if (CheckCollisionRecs(box5, boxCharacter))
        {
            position.x += 2.0f;
            boxCharacter.x += 2.0f;
        }

        if (CheckCollisionRecs(box6, boxCharacter))
        {
            position.y += 2.0f;
            boxCharacter.y += 2.0f;
        }

        if (CheckCollisionRecs(box7, boxCharacter))
        {
            position.y -= 2.0f;
            boxCharacter.y -= 2.0f;
        }

        if (CheckCollisionRecs(box8, boxCharacter))
        {
            position.x -= 2.0f;
            boxCharacter.x -= 2.0f;
        }

        if (CheckCollisionRecs(box9, boxCharacter))
        {
            position.y += 2.0f;
            boxCharacter.y += 2.0f;
        }

        if (CheckCollisionRecs(box10, boxCharacter))
        {
            position.x += 2.0f;
            boxCharacter.x += 2.0f;
        }

        if (IsKeyDown(KEY_ENTER))
        {
            showBoxForBag = true;
            AskForSellOrBuy = false;
            sell = false;
            buy = false;
            showTextNotEnough = false;
        }

        if (IsKeyDown(KEY_Z))
        {
            AskForSellOrBuy = true;
            showBoxForBag = false;
            sell = false;
            buy = false;
            showTextNotEnough = false;
        }

        if (IsKeyDown(KEY_X))
        {
            showBoxForBag = false;
            AskForSellOrBuy = false;
            sell = false;
            buy = false;
            showTextNotEnough = false;
        }

        if (right == true)
        {
            DrawTextureEx(rightActor, position, 0.0f, 2.5f, WHITE);
        }

        else if (left == true)
        {
            DrawTextureEx(leftActor, position, 0.0f, 2.5f, WHITE);
        }

        else if (up == true)
        {
            DrawTextureEx(upActor, position, 0.0f, 2.5f, WHITE);
        }

        else if (down == true)
        {
            DrawTextureEx(downActor, position, 0.0f, 2.5f, WHITE);
        }

        if (buyFirstItem == true)
        {
            std::string s;
            std::stringstream out;
            out << firstItem;
            s = out.str();

            sql2 = "UPDATE ItmesInBag set Count = " + s + " where Num = 1;";
            int recoverData2 = sqlite3_exec(db2, sql2.c_str(), callback2, 0, &GetError2);
        }

        if (buySecondItem == true)
        {
            std::string s;
            std::stringstream out;
            out << secondItem;
            s = out.str();

            sql2 = "UPDATE ItmesInBag set Count = " + s + " where Num = 2;";
            int recoverData2 = sqlite3_exec(db2, sql2.c_str(), callback2, 0, &GetError2);
        }

        if (buyThirdItem == true)
        {
            std::string s;
            std::stringstream out;
            out << thirdItem;
            s = out.str();

            sql2 = "UPDATE ItmesInBag set Count = " + s + " where Num = 3;";
            int recoverData2 = sqlite3_exec(db2, sql2.c_str(), callback2, 0, &GetError2);
        }

        if (showBoxForBag == true)
        {
            DrawRectangleLinesEx(container, 3, borderColor);    // Draw container border

            // Draw text in container (add some padding)
            Rectangle textInRec = { container.x + 4, container.y + 4, container.width - 4, container.height - 4 };

            Rectangle textInRec2 = { container.x + 4, container.y + 56, container.width - 4, container.height - 56 };
            Rectangle textInRec3 = { container.x + 50, container.y + 56, container.width - 50, container.height - 56 };
            Rectangle textInRec4 = { container.x + 170, container.y + 56, container.width - 170, container.height - 56 };

            Rectangle textInRec7 = { container.x + 4, container.y + 115, container.width - 4, container.height - 115 };
            Rectangle textInRec8 = { container.x + 50, container.y + 115, container.width - 50, container.height - 115 };
            Rectangle textInRec9 = { container.x + 170, container.y + 115, container.width - 170, container.height - 115 };

            Rectangle textInRec12 = { container.x + 4, container.y + 175, container.width - 4, container.height - 175 };
            Rectangle textInRec13 = { container.x + 50, container.y + 175, container.width - 50, container.height - 175 };
            Rectangle textInRec14 = { container.x + 170, container.y + 175, container.width - 170, container.height - 175 };

            DrawTextBoxed(font, textForBag, textInRec, 20.0f, 2.0f, wordWrap, BLACK);

            DrawText("Num", 20, 230, 20, BLACK);
            DrawText("Name", 70, 230, 20, BLACK);
            DrawText("Count", 170, 230, 20, BLACK);

            std::string s1;
            std::stringstream out1;
            out1 << firstItem;
            s1 = out1.str();

            std::string s2;
            std::stringstream out2;
            out2 << secondItem;
            s2 = out2.str();

            std::string s3;
            std::stringstream out3;
            out3 << thirdItem;
            s3 = out3.str();

            DrawTextBoxed(font, MyString2[0][0].c_str(), textInRec2, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString2[0][1].c_str(), textInRec3, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, s1.c_str(), textInRec4, 20.0f, 2.0f, wordWrap, BLACK);
                                        
            DrawTextBoxed(font, MyString2[1][0].c_str(), textInRec7, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString2[1][1].c_str(), textInRec8, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, s2.c_str(), textInRec9, 20.0f, 2.0f, wordWrap, BLACK);
                                        
            DrawTextBoxed(font, MyString2[2][0].c_str(), textInRec12, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString2[2][1].c_str(), textInRec13, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, s3.c_str(), textInRec14, 20.0f, 2.0f, wordWrap, BLACK);
        }

        if (position.x >= 170 && position.x <= 180 && position.y >= 140 && position.y <= 160 && AskForSellOrBuy == true)
        {
            DrawRectangleLinesEx(container, 3, borderColor);    // Draw container border

            Rectangle textInRec = { container.x + 4, container.y + 4, container.width - 4, container.height - 4 };
            DrawTextBoxed(font, textForClert2, textInRec, 20.0f, 2.0f, wordWrap, BLACK);

            // Buy
            if (IsKeyPressed(KEY_B))
            {
                buy = true;
                sell = false;
                AskForSellOrBuy = false;
            }

            //Sell
            if (IsKeyPressed(KEY_S) )
            {
                sell = true;
                buy = false;
                AskForSellOrBuy = false;
            }
        }

        if (buy == true)
        {
            DrawRectangleLinesEx(container, 3, borderColor);    // Draw container border

                // Draw text in container (add some padding)
            Rectangle textInRec = { container.x + 4, container.y + 4, container.width - 4, container.height - 4 };

            Rectangle textInRec2 = { container.x + 4, container.y + 56, container.width - 4, container.height - 56 };
            Rectangle textInRec3 = { container.x + 50, container.y + 56, container.width - 50, container.height - 56 };
            Rectangle textInRec4 = { container.x + 170, container.y + 56, container.width - 170, container.height - 56 };
            Rectangle textInRec5 = { container.x + 250, container.y + 56, container.width - 250, container.height - 56 };
            Rectangle textInRec6 = { container.x + 290, container.y + 56, container.width - 290, container.height - 56 };

            Rectangle textInRec7 = { container.x + 4, container.y + 115, container.width - 4, container.height - 115 };
            Rectangle textInRec8 = { container.x + 50, container.y + 115, container.width - 50, container.height - 115 };
            Rectangle textInRec9 = { container.x + 170, container.y + 115, container.width - 170, container.height - 115 };
            Rectangle textInRec10 = { container.x + 250, container.y + 115, container.width - 250, container.height - 115 };
            Rectangle textInRec11 = { container.x + 290, container.y + 115, container.width - 290, container.height - 115 };

            Rectangle textInRec12 = { container.x + 4, container.y + 175, container.width - 4, container.height - 175 };
            Rectangle textInRec13 = { container.x + 50, container.y + 175, container.width - 50, container.height - 175 };
            Rectangle textInRec14 = { container.x + 170, container.y + 175, container.width - 170, container.height - 175 };
            Rectangle textInRec15 = { container.x + 250, container.y + 175, container.width - 250, container.height - 175 };
            Rectangle textInRec16 = { container.x + 290, container.y + 175, container.width - 290, container.height - 175 };

            DrawTextBoxed(font, textForClert, textInRec, 20.0f, 2.0f, wordWrap, BLACK);

            DrawText("Num", 20, 230, 20, BLACK);
            DrawText("Name", 70, 230, 20, BLACK);
            DrawText("Price", 170, 230, 20, BLACK);
            DrawText("Badge", 240, 230, 20, BLACK);
            DrawText("Effect", 330, 230, 20, BLACK);

            DrawTextBoxed(font, MyString[0][0].c_str(), textInRec2, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[0][1].c_str(), textInRec3, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[0][2].c_str(), textInRec4, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[0][3].c_str(), textInRec5, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[0][4].c_str(), textInRec6, 20.0f, 2.0f, wordWrap, BLACK);

            DrawTextBoxed(font, MyString[1][0].c_str(), textInRec7, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[1][1].c_str(), textInRec8, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[1][2].c_str(), textInRec9, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[1][3].c_str(), textInRec10, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[1][4].c_str(), textInRec11, 20.0f, 2.0f, wordWrap, BLACK);

            DrawTextBoxed(font, MyString[2][0].c_str(), textInRec12, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[2][1].c_str(), textInRec13, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[2][2].c_str(), textInRec14, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[2][3].c_str(), textInRec15, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[2][4].c_str(), textInRec16, 20.0f, 2.0f, wordWrap, BLACK);

            if (IsKeyPressed(KEY_ONE))
            {
                PlaySoundMulti(getItem);
                buyFirstItem = true;
                firstItem += 1;
            }

            if (IsKeyPressed(KEY_TWO))
            {
                PlaySoundMulti(getItem);
                buySecondItem = true;
                secondItem += 1;
            }

            if (IsKeyPressed(KEY_THREE))
            {
                PlaySoundMulti(getItem);
                buyThirdItem = true;
                thirdItem += 1;
            }
        }

        if (sell == true)
        {
            DrawRectangleLinesEx(container, 3, borderColor);    // Draw container border

                // Draw text in container (add some padding)
            Rectangle textInRec = { container.x + 4, container.y + 4, container.width - 4, container.height - 4 };

            Rectangle textInRec2 = { container.x + 4, container.y + 56, container.width - 4, container.height - 56 };
            Rectangle textInRec3 = { container.x + 50, container.y + 56, container.width - 50, container.height - 56 };
            Rectangle textInRec4 = { container.x + 170, container.y + 56, container.width - 170, container.height - 56 };
            Rectangle textInRec5 = { container.x + 250, container.y + 56, container.width - 250, container.height - 56 };
            Rectangle textInRec6 = { container.x + 290, container.y + 56, container.width - 290, container.height - 56 };

            Rectangle textInRec7 = { container.x + 4, container.y + 115, container.width - 4, container.height - 115 };
            Rectangle textInRec8 = { container.x + 50, container.y + 115, container.width - 50, container.height - 115 };
            Rectangle textInRec9 = { container.x + 170, container.y + 115, container.width - 170, container.height - 115 };
            Rectangle textInRec10 = { container.x + 250, container.y + 115, container.width - 250, container.height - 115 };
            Rectangle textInRec11 = { container.x + 290, container.y + 115, container.width - 290, container.height - 115 };

            Rectangle textInRec12 = { container.x + 4, container.y + 175, container.width - 4, container.height - 175 };
            Rectangle textInRec13 = { container.x + 50, container.y + 175, container.width - 50, container.height - 175 };
            Rectangle textInRec14 = { container.x + 170, container.y + 175, container.width - 170, container.height - 175 };
            Rectangle textInRec15 = { container.x + 250, container.y + 175, container.width - 250, container.height - 175 };
            Rectangle textInRec16 = { container.x + 290, container.y + 175, container.width - 290, container.height - 175 };

            DrawTextBoxed(font, textForClert, textInRec, 20.0f, 2.0f, wordWrap, BLACK);

            DrawText("Num", 20, 230, 20, BLACK);
            DrawText("Name", 70, 230, 20, BLACK);
            DrawText("Price", 170, 230, 20, BLACK);
            DrawText("Badge", 240, 230, 20, BLACK);
            DrawText("Effect", 330, 230, 20, BLACK);

            DrawTextBoxed(font, MyString[0][0].c_str(), textInRec2, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[0][1].c_str(), textInRec3, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[0][2].c_str(), textInRec4, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[0][3].c_str(), textInRec5, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[0][4].c_str(), textInRec6, 20.0f, 2.0f, wordWrap, BLACK);

            DrawTextBoxed(font, MyString[1][0].c_str(), textInRec7, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[1][1].c_str(), textInRec8, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[1][2].c_str(), textInRec9, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[1][3].c_str(), textInRec10, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[1][4].c_str(), textInRec11, 20.0f, 2.0f, wordWrap, BLACK);

            DrawTextBoxed(font, MyString[2][0].c_str(), textInRec12, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[2][1].c_str(), textInRec13, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[2][2].c_str(), textInRec14, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[2][3].c_str(), textInRec15, 20.0f, 2.0f, wordWrap, BLACK);
            DrawTextBoxed(font, MyString[2][4].c_str(), textInRec16, 20.0f, 2.0f, wordWrap, BLACK);

            if (IsKeyPressed(KEY_ONE) && firstItem > 0)
            {
                PlaySoundMulti(removeItem);
                buyFirstItem = true;
                firstItem -= 1;
            }
            else if (IsKeyPressed(KEY_ONE) && firstItem <= 0)
            {
                showTextNotEnough = true;
                sell = false;
            }

            if (IsKeyPressed(KEY_TWO) && secondItem > 0)
            {
                PlaySoundMulti(removeItem);
                buySecondItem = true;
                secondItem -= 1;
            }
            else if (IsKeyPressed(KEY_TWO) && secondItem <= 0)
            {
                showTextNotEnough = true;
                sell = false;
            }

            if (IsKeyPressed(KEY_THREE) && thirdItem > 0)
            {
                PlaySoundMulti(removeItem);
                buyThirdItem = true;
                thirdItem -= 1;
            }
            else if (IsKeyPressed(KEY_THREE) && thirdItem <= 0)
            {
                showTextNotEnough = true;
                sell = false;
            }
        }

        if (showTextNotEnough == true)
        {
            DrawRectangleLinesEx(container, 3, borderColor);    // Draw container border

                // Draw text in container (add some padding)
            Rectangle textInRec = { container.x + 4, container.y + 4, container.width - 4, container.height - 4 };

            DrawTextBoxed(font, "You do not have enough item!", textInRec, 20.0f, 2.0f, wordWrap, BLACK);
        }

        EndDrawing();
    }

    // De-Initialization
    UnloadTexture(rightActor);
    UnloadTexture(leftActor);
    UnloadTexture(upActor);
    UnloadTexture(downActor);
    UnloadTexture(PokemonStoreTexture);

    UnloadMusicStream(music);   // Unload music stream buffers from RAM

    CloseAudioDevice();         // Close audio device (music streaming is automatically stopped)

    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    sqlite3_close(db2);

    return 0;
}

// argc = return the number of collumn
// argv = array that get the value of each domain in row
// azColName = return the string with name of attribute

int callback(void* data, int argc, char** argv, char** azColName)
{
    for (int row = 0; row < argc; row++)
    {
        MyString[id][row] = argv[row];
    }
    id++;

    return 0;
}

int callback2(void* data, int argc, char** argv, char** azColName)
{
    for (int row = 0; row < argc; row++)
    {
        MyString2[id2][row] = argv[row];
    }
    id2++;

    return 0;
}

// Draw text using font inside rectangle limits
static void DrawTextBoxed(Font font, const char* text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint)
{
    DrawTextBoxedSelectable(font, text, rec, fontSize, spacing, wordWrap, tint, 0, 0, WHITE, WHITE);
}

// Draw text using font inside rectangle limits with support for text selection
static void DrawTextBoxedSelectable(Font font, const char* text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint, int selectStart, int selectLength, Color selectTint, Color selectBackTint)
{
    int length = TextLength(text);  // Total length in bytes of the text, scanned by codepoints in loop

    float textOffsetY = 0;          // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f;       // Offset X to next character to draw

    float scaleFactor = fontSize / (float)font.baseSize;     // Character rectangle scaling factor

    // Word/character wrapping mechanism variables
    enum { MEASURE_STATE = 0, DRAW_STATE = 1 };
    int state = wordWrap ? MEASURE_STATE : DRAW_STATE;

    int startLine = -1;         // Index where to begin drawing (where a line begins)
    int endLine = -1;           // Index where to stop drawing (where a line ends)
    int lastk = -1;             // Holds last value of the character position

    for (int i = 0, k = 0; i < length; i++, k++)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        if (codepoint == 0x3f) codepointByteCount = 1;
        i += (codepointByteCount - 1);

        float glyphWidth = 0;
        if (codepoint != '\n')
        {
            glyphWidth = (font.glyphs[index].advanceX == 0) ? font.recs[index].width * scaleFactor : font.glyphs[index].advanceX * scaleFactor;

            if (i + 1 < length) glyphWidth = glyphWidth + spacing;
        }

        if (state == MEASURE_STATE)
        {
            if ((codepoint == ' ') || (codepoint == '\t') || (codepoint == '\n')) endLine = i;

            if ((textOffsetX + glyphWidth) > rec.width)
            {
                endLine = (endLine < 1) ? i : endLine;
                if (i == endLine) endLine -= codepointByteCount;
                if ((startLine + codepointByteCount) == endLine) endLine = (i - codepointByteCount);

                state = !state;
            }
            else if ((i + 1) == length)
            {
                endLine = i;
                state = !state;
            }
            else if (codepoint == '\n') state = !state;

            if (state == DRAW_STATE)
            {
                textOffsetX = 0;
                i = startLine;
                glyphWidth = 0;

                // Save character position when we switch states
                int tmp = lastk;
                lastk = k - 1;
                k = tmp;
            }
        }
        else
        {
            if (codepoint == '\n')
            {
                if (!wordWrap)
                {
                    textOffsetY += (font.baseSize + font.baseSize / 2) * scaleFactor;
                    textOffsetX = 0;
                }
            }
            else
            {
                if (!wordWrap && ((textOffsetX + glyphWidth) > rec.width))
                {
                    textOffsetY += (font.baseSize + font.baseSize / 2) * scaleFactor;
                    textOffsetX = 0;
                }

                // When text overflows rectangle height limit, just stop drawing
                if ((textOffsetY + font.baseSize * scaleFactor) > rec.height) break;

                // Draw selection background
                bool isGlyphSelected = false;
                Rectangle r = { (rec.x + textOffsetX - 1), (rec.y + textOffsetY), glyphWidth, ((float)font.baseSize * scaleFactor) };
                if ((selectStart >= 0) && (k >= selectStart) && (k < (selectStart + selectLength)))
                {
                    DrawRectangleRec(r, selectBackTint);
                    isGlyphSelected = true;
                }

                // Draw current character glyph
                Vector2 v = { rec.x + textOffsetX, rec.y + textOffsetY };
                if ((codepoint != ' ') && (codepoint != '\t'))
                {
                    DrawTextCodepoint(font, codepoint, v, fontSize, isGlyphSelected ? selectTint : tint);
                }
            }

            if (wordWrap && (i == endLine))
            {
                textOffsetY += (font.baseSize + font.baseSize / 2) * scaleFactor;
                textOffsetX = 0;
                startLine = endLine;
                endLine = -1;
                glyphWidth = 0;
                selectStart += lastk - k;
                k = lastk;

                state = !state;
            }
        }

        textOffsetX += glyphWidth;
    }
}
