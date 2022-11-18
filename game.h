#ifndef _GAME_H_
#define _GAME_H_

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>

#include "NeHeGL.h"
#include "NeHe_Window.h"
#include "cube.h"
#include "Bitmap.h"

class Game
{
public:
	bool Initialize (GL_Window *window, Keys *keys, float tickCount);
	void Deinitialize ();
	void Update (float tickCount, float lastTickCount);
	void Draw (float tickCount);
	void EnterMenu (float tickCount);

private:
	enum GameState
	{
		ActualGame,
		Menu,
		Instructions,
		Credits,
		Difficulty,
		Load,
		Save
	};

	void NewGame ();
	inline void EnterVictory (float tickCount);
	bool SaveGame (float tickCount);
	void LoadGame (int number, float tickCount);
	void CreateFilename (int number, char *filename);
	char *CalculateTimeDisplay (float m);
	bool FileExists (char *filename);
	inline void Update_ActualGame (float tickCount, float lastTickCount);
	inline void Update_Load (float tickCount, float lastTickCount);
	inline void Update_Save (float tickCount, float lastTickCount);
	inline void Update_Difficulty (float tickCount, float lastTickCount);
	inline void Update_Menu (float tickCount, float lastTickCount);
	inline void Update_Instructions (float tickCount, float lastTickCount);
	inline void Update_Credits (float tickCount, float lastTickCount);
	inline void Draw_ActualGame (float tickCount);
	inline void Draw_Load (float tickCount);
	inline void Draw_Save (float tickCount);
	inline void Draw_Difficulty (float tickCount);
	inline void Draw_Menu (float tickCount);
	inline void Draw_Instructions (float tickCount);
	inline void Draw_Credits (float tickCount);
	inline void EnterLoad ();
	void UpdateStepsDisplay ();
	void UpdateGameNumberDisplay ();
	void UpdateDifficultyDisplay ();
	void InitKeyPressed ();
	void RenderMenuItem (int i);
	void UpdateMenuDisplay ();
	void LeaveActualGame (float tickCount);
	void ReturnToActualGame (float tickCount);

	float anim0_dur;
	float anim1_dur;
	GL_Window *g_window;
	Keys *g_keys;
	Cube cube;
	int angleX;
	int angleY;
	int targetAngleX;
	int targetAngleY;
	int angleFrontFace;
	float rotationStartTickCount;
	bool frontFaceRotating;
	Bitmap bitmap_game [4];
	GLuint texture_game [4];
	bool cheatMode;
	bool updateHistory;
	int cheatCodePtr;
	float startTime;
	float initTime;
	float zzoom;
	TextObject textObject;
	int steps;
	GameState state;
	float endTime;
	int endSteps;
	float elapsedTime;
	bool nPressed;
	bool sPressed;
	bool lPressed;
	bool lPressedCheat;
	bool escPressed;
	bool spacePressed;
	bool leftPressed;
	bool rightPressed;
	bool victory;
	int selectedGame;
	Bitmap bitmap_load [2];
	GLuint texture_load [2];
	Bitmap bitmap_save [2];
	GLuint texture_save [2];
	Bitmap bitmap_difficulty [2];
	GLuint texture_difficulty [2];
	int selectedDifficulty;
	float tempTimer;
	Bitmap bitmap_menu [7];
	GLuint texture_menu [7];
	int selectedMenuItem;
	Bitmap bitmap_instructions;
	GLuint texture_instructions;
	Bitmap bitmap_credits;
	GLuint texture_credits;
	float cursorPos;
	bool music_playing;
};

#endif