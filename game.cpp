#include "game.h"

bool Game::Initialize (GL_Window* window, Keys* keys, float tickCount)	
{
	g_window			= window;
	g_keys				= keys;

	selectedDifficulty = 20;

	anim0_dur = 2500.0f;
	anim1_dur = 500.0f;

	glClearColor (0.0f, 0.0f, 0.4f, 0.5f);
	glClearDepth (1.0f);
	glDepthFunc (GL_LEQUAL);
	glEnable (GL_DEPTH_TEST);
	glShadeModel (GL_SMOOTH);
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glAlphaFunc (GL_GREATER, 0.1f);
	glEnable (GL_ALPHA_TEST);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE);
 
	textObject.SetText ("Adok's Magic Cube - Special Edition for tUM 2010");

	for (int i = 0; i < 3; i++)
	{
		if (!bitmap_game [i].New (256, 64))
			return false;
		bitmap_game [i].SetRenderTextFont ("Verdana", 22);
		bitmap_game [i].SetRenderTextColor (0xffff00);
		if (i == 0)
			bitmap_game [0].RenderText (&textObject, Align_Left);
		bitmap_game [i].GenerateTexture (texture_game [i]);
	}

	if (!bitmap_game [3].New (512, 128))
		return false;
	bitmap_game [3].SetRenderTextFont ("Verdana", 55);
	bitmap_game [3].SetRenderTextColor (0x00ffff);
	textObject.SetText ("CONGRATULATIONS!");
	bitmap_game [3].RenderText (&textObject, Align_Center);
	bitmap_game [3].GenerateTexture (texture_game [3]);

	textObject.SetText ("Load Game Number:");

	for (int i = 0; i < 2; i++)
	{
		if (!bitmap_load [i].New (256, 64))
			return false;
		bitmap_load [i].SetRenderTextFont ("Verdana", 22);
		bitmap_load [i].SetRenderTextColor (0xffff00);
		if (i == 0)
			bitmap_load [0].RenderText (&textObject, Align_Center);
		bitmap_load [i].GenerateTexture (texture_load [i]);
	}

	textObject.SetText ("Game saved as number:");

	for (int i = 0; i < 2; i++)
	{
		if (!bitmap_save [i].New (256, 64))
			return false;
		bitmap_save [i].SetRenderTextFont ("Verdana", 22);
		bitmap_save [i].SetRenderTextColor (0xffff00);
		if (i == 0)
			bitmap_save [0].RenderText (&textObject, Align_Center);
		bitmap_save [i].GenerateTexture (texture_save [i]);
	}

	textObject.SetText ("New Game Difficulty:");

	for (int i = 0; i < 2; i++)
	{
		if (!bitmap_difficulty [i].New (256, 64))
			return false;
		bitmap_difficulty [i].SetRenderTextFont ("Verdana", 22);
		bitmap_difficulty [i].SetRenderTextColor (0xffff00);
		if (i == 0)
			bitmap_difficulty [0].RenderText (&textObject, Align_Center);
		bitmap_difficulty [i].GenerateTexture (texture_difficulty [i]);
	}

	for (int i = 0; i < 7; i++)
	{
		if (!bitmap_menu [i].New (256, 64))
			return false;
		bitmap_menu [i].SetRenderTextFont ("Verdana", 22);
		bitmap_menu [i].SetRenderTextColor (0xffff00);
		RenderMenuItem (i);
		bitmap_menu [i].GenerateTexture (texture_menu [i]);
	}

	textObject.SetText ("Use the cursor keys to rotate the cube. Press Space to rotate the front face. Press Escape to enter the menu where you can save and load your game, start a new game, etc. The game is won once all the faces of the cube have been solved. A face is solved when all its fields have the same colour.");

	if (!bitmap_instructions.New (256, 64 * 8))
		return false;
	bitmap_instructions.SetRenderTextFont ("Verdana", 22);
	bitmap_instructions.SetRenderTextColor (0xffff00);
	bitmap_instructions.RenderText (&textObject, Align_Justify);
	bitmap_instructions.GenerateTexture (texture_instructions);

	textObject.SetText ("Idea & Code by Adok. Visit http://hugi.scene.org/adok/ for updates!");

	if (!bitmap_credits.New (256, 64 * 4))
		return false;
	bitmap_credits.SetRenderTextFont ("Verdana", 22);
	bitmap_credits.SetRenderTextColor (0xffff00);
	bitmap_credits.RenderText (&textObject, Align_Justify);
	bitmap_credits.GenerateTexture (texture_credits);
 
	cheatCodePtr = 0;
	cheatMode = false;

	zzoom = -20.0f;

	NewGame ();
	EnterMenu (tickCount);

	music_playing = false;

	return true;
}

void Game::NewGame ()
{
	victory = false;

	angleX = 0;
	angleY = 0;
	targetAngleX = 0;
	targetAngleY = 0;
	angleFrontFace = 0;
	frontFaceRotating = false;

	updateHistory = true;
	steps = 0;
	UpdateStepsDisplay ();

	elapsedTime = 0;

	initTime = startTime = GetTickCount ();
	srand (initTime);

	cube.NewGame (selectedDifficulty);

	InitKeyPressed ();

	zzoom = -20.0f;
	state = ActualGame;
}

void Game::Deinitialize ()
{
}

void Game::Update (float tickCount, float lastTickCount)
{
	switch (state)
	{
	case ActualGame:
		Update_ActualGame (tickCount, lastTickCount);
		break;

	case Load:
		Update_Load (tickCount, lastTickCount);
		break;

	case Save:
		Update_Save (tickCount, lastTickCount);
		break;

	case Difficulty:
		Update_Difficulty (tickCount, lastTickCount);
		break;

	case Menu:
		Update_Menu (tickCount, lastTickCount);
		break;

	case Instructions:
		Update_Instructions (tickCount, lastTickCount);
		break;

	case Credits:
		Update_Credits (tickCount, lastTickCount);
		break;
	}
}

void Game::EnterMenu (float tickCount)
{
	LeaveActualGame (tickCount);
	state = Menu;
	selectedMenuItem = 0;
	cursorPos = 0;
	UpdateMenuDisplay ();
	InitKeyPressed ();
}

inline void Game::Update_ActualGame (float tickCount, float lastTickCount)
{
	const float rotationDelay = 5.0f;
	const char cheatCode [] = "HUGIRULEZ";
	Direction action = dnothing;

	if (victory || elapsedTime + tickCount - initTime > anim0_dur)
		zzoom = -5.0f;
	else
		zzoom = -20.0f + 15.0f * sin (3.141592 / 2 * (elapsedTime + tickCount - initTime) / anim0_dur);

	if (g_keys->keyDown [VK_F1])
	{
		LeaveActualGame (tickCount);
		state = Instructions;
		return;
	}

	if (g_keys->keyDown ['N'])
		nPressed = true;

	if (nPressed && !g_keys->keyDown ['N'])
	{
		LeaveActualGame (tickCount);
		state = Difficulty;
		UpdateDifficultyDisplay ();
		InitKeyPressed ();
		return;
	}

	if (g_keys->keyDown ['S'])
		sPressed = true;

	if (sPressed && !g_keys->keyDown ['S'])
	{
		if (!victory)
		{
			LeaveActualGame (tickCount);
			SaveGame (tickCount);	// don't allow saving game if it's already won
		}
		InitKeyPressed ();
		return;
	}

	if (!lPressedCheat && g_keys->keyDown ['L'] && cheatCode [cheatCodePtr] != 'L')
		lPressed = true;

	if (lPressedCheat && !g_keys->keyDown ['L'])
		lPressedCheat = false;

	if (lPressed && !g_keys->keyDown ['L'])
	{
		LeaveActualGame (tickCount);
		EnterLoad ();
		InitKeyPressed ();
		return;
	}

	if (g_keys->keyDown [cheatCode [cheatCodePtr]])
	{
		if (cheatCode [cheatCodePtr] == 'L')
			lPressedCheat = true;

		cheatCodePtr++;
		if (cheatCode [cheatCodePtr] == '\0')
			cheatMode = true;
	}

	if (g_keys->keyDown [VK_ESCAPE])
		escPressed = true;

	if (escPressed && !g_keys->keyDown [VK_ESCAPE])
		EnterMenu (tickCount);

	if (targetAngleX == 0 && targetAngleY == 0 && !frontFaceRotating)
	{
		if (g_keys->keyDown [VK_SPACE] ||
			g_keys->keyDown [VK_LEFT] ||
			g_keys->keyDown [VK_RIGHT] ||
			g_keys->keyDown [VK_UP] ||
			g_keys->keyDown [VK_DOWN] ||
			g_keys->keyDown [VK_RETURN])
		{
			updateHistory = true;
			if (g_keys->keyDown [VK_SPACE])
				action = dfrontRot;
			else if (g_keys->keyDown [VK_LEFT])
				action = dleft;
			else if (g_keys->keyDown [VK_RIGHT])
				action = dright;
			else if (g_keys->keyDown [VK_UP])
				action = dtop;
			else if (g_keys->keyDown [VK_DOWN])
				action = dbottom;
			else if (cheatMode && g_keys->keyDown [VK_RETURN])
			{
				action = cube.UndoMove ();
				updateHistory = false;
			}

			rotationStartTickCount = tickCount;
		}
	}

	switch (action)
	{
	case dnothing:
		break;

	case dfrontRot:
		if (steps == 0)
			startTime = GetTickCount ();
		if (!victory)
		{
			frontFaceRotating = true;
			steps++;
			UpdateStepsDisplay ();
		}
		break;

	case dleft:
		targetAngleY = 90;
		break;

	case dright:
		targetAngleY = -90;
		break;

	case dtop:
		targetAngleX = 90;
		break;

	case dbottom:
		targetAngleX = -90;
		break;
	}

	if (frontFaceRotating)
	{
		angleFrontFace = (tickCount - rotationStartTickCount) / rotationDelay;
		if (angleFrontFace >= 90)
		{
			angleFrontFace = 0;
			frontFaceRotating = false;
			cube.RotateFrontFace (updateHistory);
			if (cube.IsSolved ())
				EnterVictory (tickCount);
		}
	}
	else if (targetAngleX > 0)
	{
		angleX = (tickCount - rotationStartTickCount) / rotationDelay;
		if (angleX >= targetAngleX)
		{
			targetAngleX = 0;
			angleX = 0;
			cube.Rotate (true, false, updateHistory);
		}
	} else if (targetAngleX < 0)
	{
		angleX = -(tickCount - rotationStartTickCount) / rotationDelay;
		if (angleX <= targetAngleX)
		{
			targetAngleX = 0;
			angleX = 0;
			cube.Rotate (true, true, updateHistory);
		}
	} else if (targetAngleY > 0)
	{
		angleY = (tickCount - rotationStartTickCount) / rotationDelay;
		if (angleY >= targetAngleY)
		{
			targetAngleY = 0;
			angleY = 0;
			cube.Rotate (false, true, updateHistory);
		}
	} else if (targetAngleY < 0)
	{
		angleY = -(tickCount - rotationStartTickCount) / rotationDelay;
		if (angleY <= targetAngleY)
		{
			targetAngleY = 0;
			angleY = 0;
			cube.Rotate (false, false, updateHistory);
		}
	}

	if (steps)
	{
		float m = elapsedTime + tickCount - startTime;

		if (victory)
			m = endTime;

		textObject.SetText (CalculateTimeDisplay (m));
	}
	else
		textObject.SetText ("Elapsed Time: 00:00:00");
	bitmap_game [1].Clear ();
	bitmap_game [1].RenderText (&textObject, Align_Center);
	bitmap_game [1].UpdateTexture (texture_game [1]);
}

inline void Game::Update_Load (float tickCount, float lastTickCount)
{
	if (g_keys->keyDown [VK_ESCAPE])
		escPressed = true;

	if (escPressed && !g_keys->keyDown [VK_ESCAPE])
	{
		ReturnToActualGame (tickCount);
		InitKeyPressed ();
		return;
	}

	if (g_keys->keyDown [VK_SPACE] || g_keys->keyDown [VK_RETURN])
		spacePressed = true;

	if (spacePressed && !g_keys->keyDown [VK_SPACE] && !g_keys->keyDown [VK_RETURN])
	{
		LoadGame (selectedGame, tickCount);
		state = ActualGame;
		InitKeyPressed ();
		return;
	}

	if (!leftPressed && (g_keys->keyDown [VK_LEFT] || g_keys->keyDown [VK_DOWN]))
	{
		leftPressed = true;
		tempTimer = tickCount;
	}

	if (leftPressed && tickCount - tempTimer > 100.0f)
	{
		leftPressed = false;

		if (selectedGame)
		{
			char filename [13];

			int i = selectedGame;

			do
			{
				i--;
				CreateFilename (i, filename);
			} while (i && !FileExists (filename));

			if (FileExists (filename))
			{
				selectedGame = i;
				UpdateGameNumberDisplay ();
			}
		}
	}

	if (!rightPressed && (g_keys->keyDown [VK_RIGHT] || g_keys->keyDown [VK_UP]))
	{
		rightPressed = true;
		tempTimer = tickCount;
	}

	if (rightPressed && tickCount - tempTimer > 100.0f)
	{
		rightPressed = false;

		if (selectedGame < 999)
		{
			char filename [13];

			int i = selectedGame;

			do
			{
				i++;
				CreateFilename (i, filename);
			} while (i < 999 && !FileExists (filename));

			if (FileExists (filename))
			{
				selectedGame = i;
				UpdateGameNumberDisplay ();
			}
		}
	}
}

inline void Game::Update_Save (float tickCount, float lastTickCount)
{
	if (g_keys->keyDown [VK_ESCAPE])
		escPressed = true;

	if (escPressed && !g_keys->keyDown [VK_ESCAPE])
	{
		ReturnToActualGame (tickCount);
		InitKeyPressed ();
		return;
	}

	if (g_keys->keyDown [VK_SPACE] || g_keys->keyDown [VK_RETURN])
		spacePressed = true;

	if (spacePressed && !g_keys->keyDown [VK_SPACE] && !g_keys->keyDown [VK_RETURN])
	{
		ReturnToActualGame (tickCount);
		InitKeyPressed ();
		return;
	}
}

inline void Game::Update_Difficulty (float tickCount, float lastTickCount)
{
	if (g_keys->keyDown [VK_ESCAPE])
		escPressed = true;

	if (escPressed && !g_keys->keyDown [VK_ESCAPE])
	{
		ReturnToActualGame (tickCount);
		InitKeyPressed ();
		return;
	}

	if (g_keys->keyDown [VK_SPACE] || g_keys->keyDown [VK_RETURN])
		spacePressed = true;

	if (spacePressed && !g_keys->keyDown [VK_SPACE] && !g_keys->keyDown [VK_RETURN])
	{
		NewGame ();
		return;
	}

	if (!leftPressed && (g_keys->keyDown [VK_LEFT] || g_keys->keyDown [VK_DOWN]))
	{
		leftPressed = true;
		tempTimer = tickCount;
	}

	if (leftPressed && tickCount - tempTimer > 100.0f)
	{
		leftPressed = false;

		if (selectedDifficulty > 1)
		{
			selectedDifficulty--;
			UpdateDifficultyDisplay ();
		}
	}

	if (!rightPressed && (g_keys->keyDown [VK_RIGHT] || g_keys->keyDown [VK_UP]))
	{
		rightPressed = true;
		tempTimer = tickCount;
	}

	if (rightPressed && tickCount - tempTimer > 100.0f)
	{
		rightPressed = false;

		if (selectedDifficulty < 100)
		{
			selectedDifficulty++;
			UpdateDifficultyDisplay ();
		}
	}
}

inline void Game::Update_Menu (float tickCount, float lastTickCount)
{
	if (cursorPos < selectedMenuItem * 0.25f)
	{
		cursorPos += (selectedMenuItem * 0.25f - cursorPos) * (tickCount - lastTickCount) / 100.0f;
		if (cursorPos > selectedMenuItem * 0.25f)
			cursorPos = selectedMenuItem * 0.25f;
	}
	else if (cursorPos > selectedMenuItem * 0.25f)
	{
		cursorPos += (selectedMenuItem * 0.25f - cursorPos) * (tickCount - lastTickCount) / 100.0f;
		if (cursorPos < selectedMenuItem * 0.25f)
			cursorPos = selectedMenuItem * 0.25f;
	}

	if (g_keys->keyDown [VK_ESCAPE])
		escPressed = true;

	if (escPressed && !g_keys->keyDown [VK_ESCAPE])
	{
		ReturnToActualGame (tickCount);
		InitKeyPressed ();
		return;
	}

	if (g_keys->keyDown [VK_SPACE] || g_keys->keyDown [VK_RETURN])
		spacePressed = true;

	if (g_keys->keyDown [VK_F1])
		state = Instructions;

	if (g_keys->keyDown ['N'])
	{
		state = Difficulty;
		UpdateDifficultyDisplay ();
	}

	if (g_keys->keyDown ['S'] && !victory)
		SaveGame (tickCount);	// don't allow saving game if it's already won

	if (g_keys->keyDown ['L'])
		EnterLoad ();

	if (spacePressed && !g_keys->keyDown [VK_SPACE] && !g_keys->keyDown [VK_RETURN])
	{
		switch (selectedMenuItem)
		{
		case 0:
			ReturnToActualGame (tickCount);
			break;

		case 1:
			if (!victory)
				SaveGame (tickCount);	// don't allow saving game if it's already won
			break;

		case 2:
			EnterLoad ();
			break;

		case 3:
			state = Difficulty;
			UpdateDifficultyDisplay ();
			break;

		case 4:
			state = Instructions;
			break;

		case 5:
			state = Credits;
			break;

		case 6:
			TerminateApplication (g_window);
			break;
		}

		InitKeyPressed ();
		return;
	}

	if (!leftPressed && g_keys->keyDown [VK_DOWN])
	{
		leftPressed = true;
		tempTimer = tickCount;
	}

	if (leftPressed && tickCount - tempTimer > 200.0f)
	{
		leftPressed = false;

		if (selectedMenuItem < 6)
			selectedMenuItem++;
		else
			selectedMenuItem = 0;

		UpdateMenuDisplay ();
	}

	if (!rightPressed && g_keys->keyDown [VK_UP])
	{
		rightPressed = true;
		tempTimer = tickCount;
	}

	if (rightPressed && tickCount - tempTimer > 200.0f)
	{
		rightPressed = false;

		if (selectedMenuItem > 0)
			selectedMenuItem--;
		else
			selectedMenuItem = 6;

		UpdateMenuDisplay ();
	}
}

inline void Game::Update_Instructions (float tickCount, float lastTickCount)
{
	if (g_keys->keyDown [VK_ESCAPE] || g_keys->keyDown [VK_RETURN] || g_keys->keyDown [VK_SPACE])
		escPressed = true;

	if (escPressed && !g_keys->keyDown [VK_ESCAPE] && !g_keys->keyDown [VK_RETURN] && !g_keys->keyDown [VK_SPACE])
	{
		ReturnToActualGame (tickCount);
		InitKeyPressed ();
		return;
	}
}

inline void Game::Update_Credits (float tickCount, float lastTickCount)
{
	Update_Instructions (tickCount, lastTickCount);
}

void Game::Draw (float tickCount)
{
	switch (state)
	{
	case ActualGame:
		Draw_ActualGame (tickCount);
		break;

	case Load:
		Draw_Load (tickCount);
		break;

	case Save:
		Draw_Save (tickCount);
		break;

	case Difficulty:
		Draw_Difficulty (tickCount);
		break;

	case Menu:
		Draw_Menu (tickCount);
		break;

	case Instructions:
		Draw_Instructions (tickCount);
		break;

	case Credits:
		Draw_Credits (tickCount);
		break;
	}
}

inline void Game::Draw_ActualGame (float tickCount)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity ();

	glTranslatef (0.0f, 0.0f, zzoom);

	glEnable (GL_TEXTURE_2D);

	if (zzoom == -5.0f)
	{
		glColor3f (1.0f, 1.0f, 0.0f);

		glBindTexture (GL_TEXTURE_2D, texture_game [0]);

		float m = elapsedTime + tickCount - initTime;

		if (m < anim0_dur + anim1_dur)
			m = -1.0f + sin (3.141592 / 2 * (m - anim0_dur) / anim1_dur);
		else
			m = 0;

		glBegin (GL_QUADS);
			glTexCoord2f (0.0f, 0.0f); glVertex3f (-2.6f, -2.3f + .3f + m,  0.0f);
			glTexCoord2f (1.0f, 0.0f); glVertex3f (-0.5f, -2.3f + .3f + m,  0.0f);
			glTexCoord2f (1.0f, 1.0f); glVertex3f (-0.5f, -1.8f + .3f + m,  0.0f);
			glTexCoord2f (0.0f, 1.0f); glVertex3f (-2.6f, -1.8f + .3f + m,  0.0f);
		glEnd ();

		glBindTexture (GL_TEXTURE_2D, texture_game [1]);

		glBegin (GL_QUADS);
			glTexCoord2f (0.0f, 0.0f); glVertex3f ( 0.5f,  1.5f - m,  0.0f);
			glTexCoord2f (1.0f, 0.0f); glVertex3f ( 2.6f,  1.5f - m,  0.0f);
			glTexCoord2f (1.0f, 1.0f); glVertex3f ( 2.6f,  2.0f - m,  0.0f);
			glTexCoord2f (0.0f, 1.0f); glVertex3f ( 0.5f,  2.0f - m,  0.0f);
		glEnd ();

		glBindTexture (GL_TEXTURE_2D, texture_game [2]);

		glBegin (GL_QUADS);
			glTexCoord2f (0.0f, 0.0f); glVertex3f ( 0.5f,  1.2f + .15f - m,  0.0f);
			glTexCoord2f (1.0f, 0.0f); glVertex3f ( 2.6f,  1.2f + .15f - m,  0.0f);
			glTexCoord2f (1.0f, 1.0f); glVertex3f ( 2.6f,  1.7f + .15f - m,  0.0f);
			glTexCoord2f (0.0f, 1.0f); glVertex3f ( 0.5f,  1.7f + .15f - m,  0.0f);
		glEnd ();
	}

	if (victory)
	{
		glBindTexture (GL_TEXTURE_2D, texture_game [3]);

		glColor3f (0.0f, 1.0f, 1.0f);

		float zzoom1 = 5.0f - (elapsedTime + tickCount - startTime - endTime) / 320.0f;

		if (zzoom1 < 2.0f)
			zzoom1 = 2.0f;

		glBegin (GL_QUADS);
			glTexCoord2f (0.0f, 0.0f); glVertex3f (-1.6f,  -0.7f,  zzoom1);
			glTexCoord2f (1.0f, 0.0f); glVertex3f ( 1.6f,  -0.7f,  zzoom1);
			glTexCoord2f (1.0f, 1.0f); glVertex3f ( 1.6f,   0.3f,  zzoom1);
			glTexCoord2f (0.0f, 1.0f); glVertex3f (-1.6f,   0.3f,  zzoom1);
		glEnd ();
	}

	glDisable (GL_TEXTURE_2D);

	glColor3f (1.0f, 1.0f, 1.0f);

	glRotatef (45, 1.0f, 0.0f, 0.0f);
	glRotatef (-45, 0.0f, 1.0f, 0.0f);

	glRotatef (angleX, 1.0f, 0.0f, 0.0f);
	glRotatef (angleY, 0.0f, 1.0f, 0.0f);

	cube.Draw (angleFrontFace);
}

inline void Game::Draw_Load (float tickCount)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity ();

	glTranslatef (0.0f, 0.0f, -5.0f);

	glEnable (GL_TEXTURE_2D);

	glColor3f (1.0f, 1.0f, 0.0f);

	glBindTexture (GL_TEXTURE_2D, texture_load [0]);

	glBegin (GL_QUADS);
		glTexCoord2f (0.0f, 0.0f); glVertex3f (-1.05f,  0.3f,  0.0f);
		glTexCoord2f (1.0f, 0.0f); glVertex3f ( 1.05f,  0.3f,  0.0f);
		glTexCoord2f (1.0f, 1.0f); glVertex3f ( 1.05f,  0.8f,  0.0f);
		glTexCoord2f (0.0f, 1.0f); glVertex3f (-1.05f,  0.8f,  0.0f);
	glEnd ();

	glBindTexture (GL_TEXTURE_2D, texture_load [1]);

	glBegin (GL_QUADS);
		glTexCoord2f (0.0f, 0.0f); glVertex3f (-1.05f, -0.1f,  0.0f);
		glTexCoord2f (1.0f, 0.0f); glVertex3f ( 1.05f, -0.1f,  0.0f);
		glTexCoord2f (1.0f, 1.0f); glVertex3f ( 1.05f,  0.4f,  0.0f);
		glTexCoord2f (0.0f, 1.0f); glVertex3f (-1.05f,  0.4f,  0.0f);
	glEnd ();
}

inline void Game::Draw_Save (float tickCount)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity ();

	glTranslatef (0.0f, 0.0f, -5.0f);

	glEnable (GL_TEXTURE_2D);

	glColor3f (1.0f, 1.0f, 0.0f);

	glBindTexture (GL_TEXTURE_2D, texture_save [0]);

	glBegin (GL_QUADS);
		glTexCoord2f (0.0f, 0.0f); glVertex3f (-1.05f,  0.3f,  0.0f);
		glTexCoord2f (1.0f, 0.0f); glVertex3f ( 1.05f,  0.3f,  0.0f);
		glTexCoord2f (1.0f, 1.0f); glVertex3f ( 1.05f,  0.8f,  0.0f);
		glTexCoord2f (0.0f, 1.0f); glVertex3f (-1.05f,  0.8f,  0.0f);
	glEnd ();

	glBindTexture (GL_TEXTURE_2D, texture_save [1]);

	glBegin (GL_QUADS);
		glTexCoord2f (0.0f, 0.0f); glVertex3f (-1.05f, -0.1f,  0.0f);
		glTexCoord2f (1.0f, 0.0f); glVertex3f ( 1.05f, -0.1f,  0.0f);
		glTexCoord2f (1.0f, 1.0f); glVertex3f ( 1.05f,  0.4f,  0.0f);
		glTexCoord2f (0.0f, 1.0f); glVertex3f (-1.05f,  0.4f,  0.0f);
	glEnd ();
}

inline void Game::Draw_Difficulty (float tickCount)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity ();

	glTranslatef (0.0f, 0.0f, -5.0f);

	glEnable (GL_TEXTURE_2D);

	glColor3f (1.0f, 1.0f, 0.0f);

	glBindTexture (GL_TEXTURE_2D, texture_difficulty [0]);

	glBegin (GL_QUADS);
		glTexCoord2f (0.0f, 0.0f); glVertex3f (-1.05f,  0.3f,  0.0f);
		glTexCoord2f (1.0f, 0.0f); glVertex3f ( 1.05f,  0.3f,  0.0f);
		glTexCoord2f (1.0f, 1.0f); glVertex3f ( 1.05f,  0.8f,  0.0f);
		glTexCoord2f (0.0f, 1.0f); glVertex3f (-1.05f,  0.8f,  0.0f);
	glEnd ();

	glBindTexture (GL_TEXTURE_2D, texture_difficulty [1]);

	glBegin (GL_QUADS);
		glTexCoord2f (0.0f, 0.0f); glVertex3f (-1.05f, -0.1f,  0.0f);
		glTexCoord2f (1.0f, 0.0f); glVertex3f ( 1.05f, -0.1f,  0.0f);
		glTexCoord2f (1.0f, 1.0f); glVertex3f ( 1.05f,  0.4f,  0.0f);
		glTexCoord2f (0.0f, 1.0f); glVertex3f (-1.05f,  0.4f,  0.0f);
	glEnd ();
}

inline void Game::Draw_Menu (float tickCount)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity ();

	glTranslatef (0.0f, 0.0f, -5.0f);

	glEnable (GL_TEXTURE_2D);

	for (int i = 0; i < 7; i++)
	{
		if (i == selectedMenuItem)
			glColor3f (1.0f, 0.0f, 0.0f);
		else
			glColor3f (1.0f, 1.0f, 0.0f);

		glBindTexture (GL_TEXTURE_2D, texture_menu [i]);

		glBegin (GL_QUADS);
			glTexCoord2f (0.0f, 0.0f); glVertex3f (-1.05f,  0.3f - i * 0.25f,  0.0f);
			glTexCoord2f (1.0f, 0.0f); glVertex3f ( 1.05f,  0.3f - i * 0.25f,  0.0f);
			glTexCoord2f (1.0f, 1.0f); glVertex3f ( 1.05f,  0.8f - i * 0.25f,  0.0f);
			glTexCoord2f (0.0f, 1.0f); glVertex3f (-1.05f,  0.8f - i * 0.25f,  0.0f);
		glEnd ();
	}

	glDisable (GL_TEXTURE_2D);

	glColor3f (0.0f, 0.0f, 0.8f);

	glBegin (GL_QUADS);
		glTexCoord2f (0.0f, 0.0f); glVertex3f (-1.05f,  0.64f - cursorPos,  -0.01f);
		glTexCoord2f (1.0f, 0.0f); glVertex3f ( 1.05f,  0.64f - cursorPos,  -0.01f);
		glTexCoord2f (1.0f, 1.0f); glVertex3f ( 1.05f,  0.80f - cursorPos,  -0.01f);
		glTexCoord2f (0.0f, 1.0f); glVertex3f (-1.05f,  0.80f - cursorPos,  -0.01f);
	glEnd ();
}

inline void Game::Draw_Instructions (float tickCount)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity ();

	glTranslatef (0.0f, 0.0f, -5.0f);

	glEnable (GL_TEXTURE_2D);

	glColor3f (1.0f, 1.0f, 0.0f);

	glBindTexture (GL_TEXTURE_2D, texture_instructions);

	glBegin (GL_QUADS);
		glTexCoord2f (0.0f, 0.0f); glVertex3f (-1.05f,  0.9f - 8 * 0.5f,  0.0f);
		glTexCoord2f (1.0f, 0.0f); glVertex3f ( 1.05f,  0.9f - 8 * 0.5f,  0.0f);
		glTexCoord2f (1.0f, 1.0f); glVertex3f ( 1.05f,  0.9f,			  0.0f);
		glTexCoord2f (0.0f, 1.0f); glVertex3f (-1.05f,  0.9f,			  0.0f);
	glEnd ();
}

inline void Game::Draw_Credits (float tickCount)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity ();

	glTranslatef (0.0f, 0.0f, -5.0f);

	glEnable (GL_TEXTURE_2D);

	glColor3f (1.0f, 1.0f, 0.0f);

	glBindTexture (GL_TEXTURE_2D, texture_credits);

	glBegin (GL_QUADS);
		glTexCoord2f (0.0f, 0.0f); glVertex3f (-1.05f,  0.7f - 4 * 0.5f,  0.0f);
		glTexCoord2f (1.0f, 0.0f); glVertex3f ( 1.05f,  0.7f - 4 * 0.5f,  0.0f);
		glTexCoord2f (1.0f, 1.0f); glVertex3f ( 1.05f,  0.7f,			  0.0f);
		glTexCoord2f (0.0f, 1.0f); glVertex3f (-1.05f,  0.7f,			  0.0f);
	glEnd ();
}

char *Game::CalculateTimeDisplay (float m)
{
	long totalTime = (long) m / 1000;
	int seconds = totalTime % 60;
	totalTime /= 60;
	int minutes = totalTime % 60;
	int hours = totalTime / 60;

	char *timeDisplay = new char [20];
	char *temp = new char [5];

	strcpy (timeDisplay, "Elapsed Time: ");
	temp = itoa (hours, temp, 10);
	if (strlen (temp) == 1)
		strcat (timeDisplay, "0");
	strcat (timeDisplay, temp);
	strcat (timeDisplay, ":");
	temp = itoa (minutes, temp, 10);
	if (strlen (temp) == 1)
		strcat (timeDisplay, "0");
	strcat (timeDisplay, temp);
	strcat (timeDisplay, ":");
	temp = itoa (seconds, temp, 10);
	if (strlen (temp) == 1)
		strcat (timeDisplay, "0");
	strcat (timeDisplay, temp);

	return timeDisplay;
}

inline void Game::EnterVictory (float tickCount)
{
	endTime = elapsedTime + tickCount - startTime;
	endSteps = steps;
	victory = true;
	UpdateStepsDisplay ();
}

bool Game::SaveGame (float tickCount)
{
	FILE *file;
	char filename [13];

	int n = 1000;
	do
	{
		n--;
		CreateFilename (n, filename);
	} while (n && !FileExists (filename));

	if (n == 999)
		return false;		// game cannot be saved

	if (FileExists (filename))
		n++;
	CreateFilename (n, filename);

	file = fopen (filename, "wb");

	fwrite (&elapsedTime, sizeof (tickCount), 1, file);
	fwrite (&steps, sizeof (steps), 1, file);

	cube.Save (file);

	fclose (file);

	char *tempString = new char [20];
	textObject.SetText (itoa (n, tempString, 10));
	bitmap_save [1].Clear ();
	bitmap_save [1].RenderText (&textObject, Align_Center);
	bitmap_save [1].UpdateTexture (texture_save [1]);

	state = Save;

	return true;
}

void Game::LoadGame (int number, float tickCount)
{
	char filename [13];
	
	CreateFilename (number, filename);

	FILE *file = fopen (filename, "rb");

	fread (&elapsedTime, sizeof (elapsedTime), 1, file);
	fread (&steps, sizeof (steps), 1, file);

	cube.Load (file);

	fclose (file);

	initTime = startTime = GetTickCount ();
	state = ActualGame;
	angleX = 0;
	angleY = 0;
	targetAngleX = 0;
	targetAngleY = 0;
	angleFrontFace = 0;
	frontFaceRotating = false;
	victory = false;
	UpdateStepsDisplay ();
	InitKeyPressed ();
}

void Game::CreateFilename (int number, char *filename)
{
	strcpy (filename, "savegame.");

	if (number < 100)
		strcat (filename, "0");

	if (number < 10)
		strcat (filename, "0");

	char *temp = new char [4];
	temp = itoa (number, temp, 10);

	strcat (filename, temp);
}

bool Game::FileExists (char *filename)
{
	FILE *file = fopen (filename, "rb");

	if (file == NULL)
		return false;

	fclose (file);
	return true;
}

inline void Game::EnterLoad ()
{	
	char filename [13];

	selectedGame = 1000;
	do
	{
		selectedGame--;
		CreateFilename (selectedGame, filename);
	} while (selectedGame && !FileExists (filename));

	if (!FileExists (filename))
		return;		// no game has been saved, therefore none can be loaded

	UpdateGameNumberDisplay ();
	state = Load;
	InitKeyPressed ();
}

void Game::UpdateStepsDisplay ()
{
	char *tempString = new char [20];
	char *tempString2 = new char [20];
	strcpy (tempString, "Steps: ");
	if (victory)
		strcat (tempString, itoa (endSteps, tempString2, 10));
	else
		strcat (tempString, itoa (steps, tempString2, 10));
	textObject.SetText (tempString);
	bitmap_game [2].Clear ();
	bitmap_game [2].RenderText (&textObject, Align_Center);
	bitmap_game [2].UpdateTexture (texture_game [2]);
}

void Game::UpdateGameNumberDisplay ()
{
	char *tempString = new char [20];
	textObject.SetText (itoa (selectedGame, tempString, 10));
	bitmap_load [1].Clear ();
	bitmap_load [1].RenderText (&textObject, Align_Center);
	bitmap_load [1].UpdateTexture (texture_load [1]);
}

void Game::UpdateDifficultyDisplay ()
{
	char *tempString = new char [20];
	textObject.SetText (itoa (selectedDifficulty, tempString, 10));
	bitmap_difficulty [1].Clear ();
	bitmap_difficulty [1].RenderText (&textObject, Align_Center);
	bitmap_difficulty [1].UpdateTexture (texture_difficulty [1]);
}

void Game::InitKeyPressed ()
{
	nPressed = false;
	sPressed = false;
	lPressed = false;
	escPressed = false;
	spacePressed = false;
	leftPressed = false;
	rightPressed = false;
}

void Game::RenderMenuItem (int i)
{
	switch (i)
	{
	case 0:
		textObject.SetText ("Resume Game");
		break;

	case 1:
		textObject.SetText ("Save Game");
		break;

	case 2:
		textObject.SetText ("Load Game");
		break;

	case 3:
		textObject.SetText ("New Game");
		break;

	case 4:
		textObject.SetText ("Instructions");
		break;

	case 5:
		textObject.SetText ("Credits");
		break;

	case 6:
		textObject.SetText ("Quit");
		break;
	}

	bitmap_menu [i].RenderText (&textObject, Align_Center);
}

void Game::UpdateMenuDisplay ()
{
	for (int i = 0; i < 7; i++)
	{
		bitmap_menu [i].Clear ();
		if (i == selectedMenuItem)
			bitmap_menu [i].SetRenderTextColor (0xff0000);
		else
			bitmap_menu [i].SetRenderTextColor (0xffff00);
		RenderMenuItem (i);
		bitmap_menu [i].UpdateTexture (texture_menu [i]);
	}
}

void Game::LeaveActualGame (float tickCount)
{
	if (steps)
		elapsedTime += tickCount - startTime;
}

void Game::ReturnToActualGame (float tickCount)
{
	startTime = tickCount;
	if (!music_playing)
	{
		music_playing = true;
		initTime = startTime = GetTickCount ();
	}
	state = ActualGame;
}
