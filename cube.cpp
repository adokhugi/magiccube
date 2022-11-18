#include "cube.h"

Cube::Cube ()
{
	Initialize ();
}

void Cube::Initialize ()
{
	front = 0;
	faces [front].topLeftField = 0;
	top		= faces [front].top		= 1;
	bottom	= faces [front].bottom	= 4;
	left	= faces [front].left	= 3;
	right	= faces [front].right	= 2;
	back	= faces [front].back	= 5;				// 0
	faces [faces [front].top].topLeftField		= 0;
	faces [faces [front].bottom].topLeftField	= 0;
	faces [faces [front].left].topLeftField		= 0;
	faces [faces [front].right].topLeftField	= 0;
	faces [faces [front].back].topLeftField		= 0;
	InitRotate (true, true);							// 4
	InitRotate (true, true);							// 5
	InitRotate (true, true);							// 1
	front = 0;
	InitRotate (false, true);							// 3
	front = 0;
	InitRotate (false, false);							// 2
	front = 0;
	for (int faceNum = 0; faceNum < 6; faceNum++)
	{
		StoreFieldBorders (faceNum);
		for (int fieldNum = 0; fieldNum < 9; fieldNum++)
			faces [faceNum].fields [fieldNum].color = faceNum;
	}
	CalculateRelationsToFrontFace ();
	historyPtr = 0;
	undoStep.times = 0;
}

void Cube::NewGame (int n)
{
	Cube cube1;
	int attempts = 24 * n;	// prevents endless loop
	bool lastStepFrontFaceRotation = false;
	int lastRotatedFrontFace = front;

	Initialize ();
	
	while (n && attempts)
	{
		attempts--;

		switch (rand () % 3)
		{
		case 0:
			Rotate (true, true);
			lastStepFrontFaceRotation = false;
			break;

		case 1:
			Rotate (false, false);
			lastStepFrontFaceRotation = false;
			break;

		case 2:
			RotateFrontFace ();
			if (!lastStepFrontFaceRotation && lastRotatedFrontFace != front)
				n--;
			for (int i = 0; i < historyPtr; i++)
			{
				cube1.Initialize ();
				cube1.Rotate (history [i]);
				if (history [i] == dfrontRot && cube1 == this)
				{
					for (int j = 0; j < 3; j++)
						RotateFrontFace ();
					historyPtr -= 4;
					if (!lastStepFrontFaceRotation && lastRotatedFrontFace != front)
						n++;
					i = historyPtr;
				}
			}
			lastStepFrontFaceRotation = true;
			lastRotatedFrontFace = front;
			break;
		}
	}

	do
	{
		switch (rand () % 2)
		{
		case 0:
			Rotate (true, true);
			break;

		case 1:
			Rotate (false, false);
			break;
		}
	} while (rand () % 3);
}

void Cube::InitRotate (bool axis, bool direction)
{
	int front_new;

	switch (axis)
	{
	case true: // x
		if (direction)
		{
			front_new = faces [front].bottom;
			faces [front_new].bottom = faces [front].back;
			faces [front_new].back = faces [front].top;
			faces [front_new].top = front;
			faces [front_new].left = faces [front].left;
			faces [front_new].right = faces [front].right;
		}
		else
		{
			front_new = faces [front].top;
			faces [front_new].top = faces [front].back;
			faces [front_new].back = faces [front].bottom;
			faces [front_new].bottom = front;
			faces [front_new].left = faces [front].left;
			faces [front_new].right = faces [front].right;
		}
		break;

	case false: // y
		if (direction)
		{
			front_new = faces [front].left;
			faces [front_new].left = faces [front].back;
			faces [front_new].back = faces [front].right;
			faces [front_new].right = front;
			faces [front_new].top = faces [front].top;
			faces [front_new].bottom = faces [front].bottom;
		}
		else
		{
			front_new = faces [front].right;
			faces [front_new].right = faces [front].back;
			faces [front_new].back = faces [front].left;
			faces [front_new].left = front;
			faces [front_new].top = faces [front].top;
			faces [front_new].bottom = faces [front].bottom;
		}
		break;
	}

	front = front_new;
}

void Cube::Rotate (Direction dir, bool updateHistory)
{
	switch (dir)
	{
	case dbottom:
		Rotate (true, true, updateHistory);
		break;

	case dtop:
		Rotate (true, false, updateHistory);
		break;

	case dleft:
		Rotate (false, true, updateHistory);
		break;

	case dright:
		Rotate (false, false, updateHistory);
		break;

	case dfrontRot:
		RotateFrontFace (updateHistory);
		break;
	}
}

void Cube::Rotate (bool axis, bool direction, bool updateHistory)
{
	int front_new, back_new, top_new, bottom_new, left_new, right_new;
	Direction dir;

	switch (axis)
	{
	case true: // x
		if (direction)
		{
			if (updateHistory)
				UpdateHistory (dbottom);
			front_new = bottom;
			bottom_new = back;
			back_new = top;
			top_new = front;
			left_new = left;
			right_new = right;
			SetTopLeftField (front_new, front, dbottom);
		}
		else
		{
			if (updateHistory)
				UpdateHistory (dtop);
			front_new = top;
			top_new = back;
			back_new = bottom;
			bottom_new = front;
			left_new = left;
			right_new = right;
			SetTopLeftField (front_new, front, dtop);
		}
		break;

	case false: // y
		if (direction)
		{
			if (updateHistory)
				UpdateHistory (dleft);
			front_new = left;
			left_new = back;
			back_new = right;
			right_new = front;
			top_new = top;
			bottom_new = bottom;
			SetTopLeftField (front_new, front, dleft);
		}
		else
		{
			if (updateHistory)
				UpdateHistory (dright);
			front_new = right;
			right_new = back;
			back_new = left;
			left_new = front;
			top_new = top;
			bottom_new = bottom;
			SetTopLeftField (front_new, front, dright);
		}
		break;
	}

	SetTopLeftField (left_new, front_new, dleft);
	SetTopLeftField (right_new, front_new, dright);
	SetTopLeftField (top_new, front_new, dtop);
	SetTopLeftField (bottom_new, front_new, dbottom);
	SetTopLeftField (back_new, top_new, dtop);

	front = front_new;
	back = back_new;
	top = top_new;
	bottom = bottom_new;
	left = left_new;
	right = right_new;

	CalculateRelationsToFrontFace ();
}

void Cube::SetTopLeftField (int target, int source, Direction dir)
{
	switch (dir)
	{
	case dtop:
		SetTopLeftField (target, source, 0, 2, 8, 6, 2, 8, 6, 0);
		break;

	case dbottom:
		SetTopLeftField (target, source, 8, 6, 0, 2, 6, 0, 2, 8);
		break;

	case dright:
		SetTopLeftField (target, source, 2, 8, 6, 0, 0, 2, 8, 6);
		break;

	case dleft:
		SetTopLeftField (target, source, 6, 0, 2, 8, 8, 6, 0, 2);
		break;
	}
}

void Cube::SetTopLeftField (int target, int source, int a0, int a2, int a8, int a6, int b0, int b2, int b8, int b6)
{
	switch (faces [source].topLeftField)
	{
	case 0:
		faces [target].topLeftField = faces [source].fields [a0].borders [1].fieldNum;
		break;

	case 2:
		faces [target].topLeftField = faces [source].fields [a2].borders [1].fieldNum;
		break;

	case 8:
		faces [target].topLeftField = faces [source].fields [a8].borders [1].fieldNum;
		break;

	case 6:
		faces [target].topLeftField = faces [source].fields [a6].borders [1].fieldNum;
		break;
	}

	switch (faces [target].topLeftField)
	{
	case 0:
		faces [target].topLeftField = b0;
		break;

	case 2:
		faces [target].topLeftField = b2;
		break;

	case 8:
		faces [target].topLeftField = b8;
		break;

	case 6:
		faces [target].topLeftField = b6;
		break;
	}
}

void Cube::PrintPositions (int faceNum)
{
	printf ("Current face: %u\ntop: %u\nbottom: %u\nleft: %u\nright: %u\nback: %u\n\n", 
		faceNum, faces [faceNum].top, faces [faceNum].bottom, faces [faceNum].left, 
		faces [faceNum].right, faces [faceNum].back); 
}

void Cube::StoreFieldBorders (int faceNum)
{
	faces [faceNum].fields [0].borders [1].faceNum = 
	faces [faceNum].fields [1].borders [0].faceNum = 
	faces [faceNum].fields [2].borders [0].faceNum = 
		faces [faceNum].top;

	StoreFieldBorders_Helper (faceNum, dtop);

	faces [faceNum].fields [2].borders [1].faceNum = 
	faces [faceNum].fields [5].borders [0].faceNum = 
	faces [faceNum].fields [8].borders [0].faceNum = 
		faces [faceNum].right;

	StoreFieldBorders_Helper (faceNum, dright);

	faces [faceNum].fields [8].borders [1].faceNum = 
	faces [faceNum].fields [7].borders [0].faceNum = 
	faces [faceNum].fields [6].borders [0].faceNum = 
		faces [faceNum].bottom;

	StoreFieldBorders_Helper (faceNum, dbottom);

	faces [faceNum].fields [6].borders [1].faceNum = 
	faces [faceNum].fields [3].borders [0].faceNum = 
	faces [faceNum].fields [0].borders [0].faceNum = 
		faces [faceNum].left;

	StoreFieldBorders_Helper (faceNum, dleft);

	faces [faceNum].fields [1].borders [1].faceNum = 
	faces [faceNum].fields [3].borders [1].faceNum = 
	faces [faceNum].fields [4].borders [0].faceNum = 
	faces [faceNum].fields [4].borders [1].faceNum = 
	faces [faceNum].fields [5].borders [1].faceNum = 
	faces [faceNum].fields [7].borders [1].faceNum = 
		-1;
}

void Cube::StoreFieldBorders_Helper (int faceNum, Direction dirSource)
{
	int num [3];
	Direction dirTarget;

	switch (dirSource)
	{
	case dtop:
		num [0] = 0;
		num [1] = 1;
		num [2] = 2;
		dirTarget = GetDirection (faceNum, faces [faceNum].top);
		break;

	case dright:
		num [0] = 2;
		num [1] = 5;
		num [2] = 8;
		dirTarget = GetDirection (faceNum, faces [faceNum].right);
		break;

	case dbottom:
		num [0] = 8;
		num [1] = 7;
		num [2] = 6;
		dirTarget = GetDirection (faceNum, faces [faceNum].bottom);
		break;

	case dleft:
		num [0] = 6;
		num [1] = 3;
		num [2] = 0;
		dirTarget = GetDirection (faceNum, faces [faceNum].left);
		break;
	}

	switch (dirTarget)
	{
	case dtop:
		faces [faceNum].fields [num [0]].borders [1].fieldNum = 2;
		faces [faceNum].fields [num [1]].borders [0].fieldNum = 1;
		faces [faceNum].fields [num [2]].borders [0].fieldNum = 0;
		break;

	case dbottom:
		faces [faceNum].fields [num [0]].borders [1].fieldNum = 6;
		faces [faceNum].fields [num [1]].borders [0].fieldNum = 7;
		faces [faceNum].fields [num [2]].borders [0].fieldNum = 8;
		break;

	case dleft:
		faces [faceNum].fields [num [0]].borders [1].fieldNum = 0;
		faces [faceNum].fields [num [1]].borders [0].fieldNum = 3;
		faces [faceNum].fields [num [2]].borders [0].fieldNum = 6;
		break;

	case dright:
		faces [faceNum].fields [num [0]].borders [1].fieldNum = 8;
		faces [faceNum].fields [num [1]].borders [0].fieldNum = 5;
		faces [faceNum].fields [num [2]].borders [0].fieldNum = 2;
		break;
	}
}

Direction Cube::GetDirection (int faceNumToFind, int faceNumToSearchIn)
{
	if (faces [faceNumToSearchIn].top == faceNumToFind)
		return dtop;
	else if (faces [faceNumToSearchIn].bottom == faceNumToFind)
		return dbottom;
	else if (faces [faceNumToSearchIn].left == faceNumToFind)
		return dleft;
	else if (faces [faceNumToSearchIn].right == faceNumToFind)
		return dright;
	else if (faces [faceNumToSearchIn].back == faceNumToFind)
		return dback;

	return derror;
}

void Cube::PrintBorders (int faceNum)
{
	printf ("Current face: %u\n", faceNum);
	for (int fieldNum = 0; fieldNum < 9; fieldNum++)
	{
		printf ("Field %u: borders ", fieldNum);
		if (faces [faceNum].fields [fieldNum].borders [0].faceNum >= 0)
		{
			printf ("face %u, field %u", faces [faceNum].fields [fieldNum].borders [0].faceNum,
			                             faces [faceNum].fields [fieldNum].borders [0].fieldNum);
			if (faces [faceNum].fields [fieldNum].borders [1].faceNum >= 0)
				printf (", and face %u, field %u", faces [faceNum].fields [fieldNum].borders [1].faceNum,
				                                   faces [faceNum].fields [fieldNum].borders [1].fieldNum);
			printf ("\n");				
		}
		else
			printf ("no other face\n");
	}
	printf ("\n");
}

void Cube::Draw (int angleFrontFace)
{
	if (angleFrontFace == 0)
	{
		Draw_Face (front);

		glRotatef (-90, 1.0f, 0.0f, 0.0f);
		Draw_Face (top);

		glRotatef (-90, 1.0f, 0.0f, 0.0f);

		Draw_Face (back);

		glRotatef (270, 1.0f, 0.0f, 0.0f);
		Draw_Face (bottom);

		glRotatef (-90, 1.0f, 0.0f, 0.0f);
		glRotatef (-90, 0.0f, 1.0f, 0.0f);
		Draw_Face (left);

		glRotatef (180, 0.0f, 1.0f, 0.0f);
		Draw_Face (right);
	}
	else
	{
		bool step = false;

		for (int i = 0; i < 2; i++)
		{
			Draw_Face (front, true, step);

			glRotatef (-90, 1.0f, 0.0f, 0.0f);
			Draw_Face (top, true, step);

			glRotatef (-90, 1.0f, 0.0f, 0.0f);

			Draw_Face (back, true, step);

			glRotatef (270, 1.0f, 0.0f, 0.0f);
			Draw_Face (bottom, true, step);

			glRotatef (-90, 1.0f, 0.0f, 0.0f);
			glRotatef (-90, 0.0f, 1.0f, 0.0f);
			Draw_Face (left, true, step);

			glRotatef (180, 0.0f, 1.0f, 0.0f);
			Draw_Face (right, true, step);

			glRotatef (-90, 0.0f, 1.0f, 0.0f);
			glRotatef (angleFrontFace, 0.0f, 0.0f, 1.0f);
			step = true;
		}
	}
}

void Cube::Draw_Face (int faceNum, bool mindRelationToFrontFace, bool whatRelationToFrontFace)
{
	glPushMatrix ();

	switch (faces [faceNum].topLeftField)
	{
	case 0:
		break;

	case 2:
		glRotatef (90, 0.0f, 0.0f, 1.0f);
		break;

	case 8:
		glRotatef (180, 0.0f, 0.0f, 1.0f);
		break;

	case 6:
		glRotatef (-90, 0.0f, 0.0f, 1.0f);
		break;
	}

	float xpos;
	float ypos = 1.0f;
	int fieldNum = 0;
	if (!mindRelationToFrontFace)
		for (int y = 0; y < 3; y++)
		{
			xpos = -1.0f;
			for (int x = 0; x < 3; x++)
			{
				Draw_Field (faceNum, fieldNum, xpos, ypos);
				xpos += 2.0f / 3;
				fieldNum++;
			}
			ypos -= 2.0f / 3;
		}
	else
		for (int y = 0; y < 3; y++)
		{
			xpos = -1.0f;
			for (int x = 0; x < 3; x++)
			{
				if (faces [faceNum].fields [fieldNum].relatedToFrontFace == whatRelationToFrontFace)
					Draw_Field (faceNum, fieldNum, xpos, ypos);
				xpos += 2.0f / 3;
				fieldNum++;
			}
			ypos -= 2.0f / 3;
		}

	glPopMatrix ();
}

inline void Cube::Draw_Field (int faceNum, int fieldNum, float xpos, float ypos)
{
	const float space = 2.0f / 3 / 9;

	glColor3f (0.0f, 0.0f, 0.0f);

	glBegin (GL_QUADS);
		glVertex3f (xpos,             ypos,             1.0f);
		glVertex3f (xpos + 2.0f / 3,  ypos,             1.0f);
		glVertex3f (xpos + 2.0f / 3,  ypos - 2.0f / 3,  1.0f);
		glVertex3f (xpos,             ypos - 2.0f / 3,  1.0f);
	glEnd ();

	switch (faces [faceNum].fields [fieldNum].color)
	{
	case 0:
		glColor3f (1.0f, 1.0f, 1.0f);	// white
		break;

	case 1:
		glColor3f (1.0f, 0.0f, 0.0f);	// red
		break;

	case 2:
		glColor3f (0.0f, 1.0f, 0.0f);	// green
		break;

	case 3:
		glColor3f (0.0f, 0.0f, 1.0f);	// blue
		break;

	case 4:
		glColor3f (1.0f, 1.0f, 0.0f);	// yellow
		break;

	case 5:
		glColor3f (1.0f, 0.0f, 1.0f);	// magenta
		break;
	}

	glBegin (GL_QUADS);
		glVertex3f (xpos + space,             ypos - space,             1.002f);
		glVertex3f (xpos + 2.0f / 3 - space,  ypos - space,             1.002f);
		glVertex3f (xpos + 2.0f / 3 - space,  ypos - 2.0f / 3 + space,  1.002f);
		glVertex3f (xpos + space,             ypos - 2.0f / 3 + space,  1.002f);
	glEnd ();
}

void Cube::RotateFrontFace (bool updateHistory)
{
	Face rotatedFace;

	for (int i = 0; i < 9; i++)
		rotatedFace.fields [i] = faces [front].fields [i];

	int faceNum = 0;
	for (int y = 0; y < 3; y++)
		for (int x = 0; x < 3; x++)
		{
			rotatedFace.fields [3 * (2 - x) + y].color = faces [front].fields [faceNum].color;
			faceNum++;
		}

	for (int i = 0; i < 9; i++)
		faces [front].fields [i] = rotatedFace.fields [i];

	int tempColor [3];
	tempColor [0] = faces [faces [front].fields [0].borders [1].faceNum].fields [faces [front].fields [0].borders [1].fieldNum].color;
	tempColor [1] = faces [faces [front].fields [1].borders [0].faceNum].fields [faces [front].fields [1].borders [0].fieldNum].color;
	tempColor [2] = faces [faces [front].fields [2].borders [0].faceNum].fields [faces [front].fields [2].borders [0].fieldNum].color;

	faces [faces [front].fields [0].borders [1].faceNum].fields [faces [front].fields [0].borders [1].fieldNum].color =
		faces [faces [front].fields [2].borders [1].faceNum].fields [faces [front].fields [2].borders [1].fieldNum].color;
	faces [faces [front].fields [1].borders [0].faceNum].fields [faces [front].fields [1].borders [0].fieldNum].color =
		faces [faces [front].fields [5].borders [0].faceNum].fields [faces [front].fields [5].borders [0].fieldNum].color;
	faces [faces [front].fields [2].borders [0].faceNum].fields [faces [front].fields [2].borders [0].fieldNum].color =
		faces [faces [front].fields [8].borders [0].faceNum].fields [faces [front].fields [8].borders [0].fieldNum].color;

	faces [faces [front].fields [2].borders [1].faceNum].fields [faces [front].fields [2].borders [1].fieldNum].color =
		faces [faces [front].fields [8].borders [1].faceNum].fields [faces [front].fields [8].borders [1].fieldNum].color;
	faces [faces [front].fields [5].borders [0].faceNum].fields [faces [front].fields [5].borders [0].fieldNum].color =
		faces [faces [front].fields [7].borders [0].faceNum].fields [faces [front].fields [7].borders [0].fieldNum].color;
	faces [faces [front].fields [8].borders [0].faceNum].fields [faces [front].fields [8].borders [0].fieldNum].color =
		faces [faces [front].fields [6].borders [0].faceNum].fields [faces [front].fields [6].borders [0].fieldNum].color;

	faces [faces [front].fields [8].borders [1].faceNum].fields [faces [front].fields [8].borders [1].fieldNum].color =
		faces [faces [front].fields [6].borders [1].faceNum].fields [faces [front].fields [6].borders [1].fieldNum].color;
	faces [faces [front].fields [7].borders [0].faceNum].fields [faces [front].fields [7].borders [0].fieldNum].color =
		faces [faces [front].fields [3].borders [0].faceNum].fields [faces [front].fields [3].borders [0].fieldNum].color;
	faces [faces [front].fields [6].borders [0].faceNum].fields [faces [front].fields [6].borders [0].fieldNum].color =
		faces [faces [front].fields [0].borders [0].faceNum].fields [faces [front].fields [0].borders [0].fieldNum].color;

	faces [faces [front].fields [6].borders [1].faceNum].fields [faces [front].fields [6].borders [1].fieldNum].color =
		tempColor [0];
	faces [faces [front].fields [3].borders [0].faceNum].fields [faces [front].fields [3].borders [0].fieldNum].color =
		tempColor [1];
	faces [faces [front].fields [0].borders [0].faceNum].fields [faces [front].fields [0].borders [0].fieldNum].color =
		tempColor [2];

	if (updateHistory)
		UpdateHistory (dfrontRot);
}

void Cube::PrintTopLeftFields ()
{
	printf ("Top Left Fields:\n");
	for (int i = 0; i < 6; i++)
		printf ("Face %u: Field %u\n", i, faces [i].topLeftField);
	printf ("\n");
}

void Cube::CalculateRelationsToFrontFace ()
{
	for (int faceNum = 0; faceNum < 6; faceNum++)
		if (faceNum == front)
			for (int fieldNum = 0; fieldNum < 9; fieldNum++)
				faces [faceNum].fields [fieldNum].relatedToFrontFace = true;
		else
			for (int fieldNum = 0; fieldNum < 9; fieldNum++)
				if (faces [faceNum].fields [fieldNum].borders [0].faceNum == front
					|| faces [faceNum].fields [fieldNum].borders [1].faceNum == front)
					faces [faceNum].fields [fieldNum].relatedToFrontFace = true;
				else
					faces [faceNum].fields [fieldNum].relatedToFrontFace = false;
}

void Cube::UpdateHistory (Direction dir)
{
	WriteUndoToHistory ();

	undoStep.times = 0;

	history [historyPtr] = dir;
	historyPtr++;

	if (historyPtr >= 20000)
	{
		for (historyPtr = 0; historyPtr < 10000; historyPtr++)
			history [historyPtr] = history [historyPtr + 10000];
	}
}

void Cube::WriteUndoToHistory ()
{
	switch (undoStep.times)
	{
	case 0:
		break;

	case 1:
		if (undoStep.action == dfrontRot)
			for (int i = 0; i < 3; i++)
			{
				history [historyPtr] = dfrontRot;
				historyPtr++;
			}
		else
		{
			history [historyPtr] = OppositeDirection (undoStep.action);
			historyPtr++;
		}
		break;

	case 2:
		history [historyPtr] = undoStep.action;
		historyPtr++;
		history [historyPtr] = undoStep.action;
		historyPtr++;
		break;
	}
}

Direction Cube::UndoMove ()
{
	while (undoStep.times == 0)
	{
		if (historyPtr == 0)
			return dnothing;

		historyPtr--;
		undoStep.action = history [historyPtr];
		undoStep.times = 0;

		while (undoStep.action == history [historyPtr] && historyPtr >= 0)
		{
			historyPtr--;
			undoStep.times++;
			if (undoStep.times == 4)
				undoStep.times = 0;
		}
		historyPtr++;

		if (undoStep.action == dfrontRot)
		{
			switch (undoStep.times)
			{
			case 0:
				break;

			case 1:
				undoStep.times = 3;
				break;

			case 2:
				break;

			case 3:
				undoStep.times = 1;
				break;
			}
		}
		else
		{
			switch (undoStep.times)
			{
			case 0:
				break;

			case 1:
				undoStep.action = OppositeDirection (undoStep.action);
				break;

			case 2:
				break;

			case 3:
				undoStep.times = 1;
				break;
			}
		}
	}

	undoStep.times--;
	return undoStep.action;
}

Direction Cube::OppositeDirection (Direction dir)
{
	switch (dir)
	{
	case dtop:
		return dbottom;
		break;

	case dbottom:
		return dtop;
		break;

	case dleft:
		return dright;
		break;

	case dright:
		return dleft;
		break;
	}
}

Cube::Face *Cube::GetFaces ()
{
	return faces;
}

bool Cube::operator== (Cube *cube1)
{
	Face *faces1 = cube1->GetFaces ();

	for (int faceNum = 0; faceNum < 6; faceNum++)
		for (int fieldNum = 0; fieldNum < 9; fieldNum++)
			if (faces [faceNum].fields [fieldNum].color != faces1 [faceNum].fields [fieldNum].color)
				return false;

	return true;
}

bool Cube::IsSolved ()
{
	for (int faceNum = 0; faceNum < 6; faceNum++)
		for (int fieldNum = 0; fieldNum < 9; fieldNum++)
			if (faces [faceNum].fields [fieldNum].color != faceNum)
				return false;

	return true;
}

void Cube::Save (FILE *file)
{
	fputc ((char) front, file);
	fputc ((char) top, file);
	fputc ((char) bottom, file);
	fputc ((char) left, file);
	fputc ((char) right, file);
	fputc ((char) back, file);

	fputc ((char) faces [front].topLeftField, file);
	fputc ((char) faces [top].topLeftField, file);
	fputc ((char) faces [bottom].topLeftField, file);
	fputc ((char) faces [left].topLeftField, file);
	fputc ((char) faces [right].topLeftField, file);
	fputc ((char) faces [back].topLeftField, file);

	for (int faceNum = 0; faceNum < 6; faceNum++)
		for (int fieldNum = 0; fieldNum < 9; fieldNum++)
			fputc ((char) faces [faceNum].fields [fieldNum].color, file);

	WriteUndoToHistory ();

	for (int i = 0; i < historyPtr; i++)
		fputc ((char) history [i], file);
}

void Cube::Load (FILE *file)
{
	Initialize ();

	front = fgetc (file);
	top = fgetc (file);
	bottom = fgetc (file);
	left = fgetc (file);
	right = fgetc (file);
	back = fgetc (file);

	faces [front].topLeftField = fgetc (file);
	faces [top].topLeftField = fgetc (file);
	faces [bottom].topLeftField = fgetc (file);
	faces [left].topLeftField = fgetc (file);
	faces [right].topLeftField = fgetc (file);
	faces [back].topLeftField = fgetc (file);

	CalculateRelationsToFrontFace ();

	for (int faceNum = 0; faceNum < 6; faceNum++)
		for (int fieldNum = 0; fieldNum < 9; fieldNum++)
			faces [faceNum].fields [fieldNum].color = fgetc (file);

	int temp;
	historyPtr = 0;
	while ((temp = fgetc (file)) != EOF)
	{
		history [historyPtr] = (Direction) temp;
		historyPtr++;
	}
}
