#ifndef _CUBE_H_
#define _CUBE_H_

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>

enum Direction
{
	dtop,
	dbottom,
	dleft,
	dright,
	dback,
	dfrontRot,
	dnothing,
	derror
};

class Cube
{
public:
	struct Face
	{
		struct Field
		{
			int color;
			struct Borders
			{
				int faceNum;
				int fieldNum;
			} borders [2];
			bool relatedToFrontFace;
		} fields [9];
		int top, bottom, left, right, back;
		int topLeftField;
	};

	struct UndoStep
	{
		Direction action;
		int times;
	};

	Cube ();
	void NewGame (int n = 20);
	void Rotate (Direction dir, bool updateHistory = true);
	void Rotate (bool axis, bool direction, bool updateHistory = true);
	void Draw (int angleX);
	void RotateFrontFace (bool updateHistory = true);
	Direction UndoMove ();
	bool operator== (Cube *cube1);
	bool IsSolved ();
	void Save (FILE *file);
	void Load (FILE *file);
	void PrintPositions (int faceNum);
	void PrintBorders (int faceNum);
	void PrintTopLeftFields ();

private:
	void Initialize ();
	void InitRotate (bool axis, bool direction);
	void StoreFieldBorders (int faceNum);
	Direction GetDirection (int faceNumToFind, int faceNumToSearchIn);
	void StoreFieldBorders_Helper (int faceNum, Direction dirSource);
	void Draw_Face (int faceNum, bool mindRelationToFrontFace = false, bool whatRelationToFrontFace = false);
	void SetTopLeftField (int target, int source, Direction dir);
	void SetTopLeftField (int target, int source, int a0, int a2, int a8, int a6, int b0, int b2, int b8, int b6);
	void CalculateRelationsToFrontFace ();
	inline void Draw_Field (int faceNum, int fieldNum, float xpos, float ypos);
	void UpdateHistory (Direction dir);
	Direction OppositeDirection (Direction dir);
	Face *GetFaces ();
	void WriteUndoToHistory ();

	Face faces [6];
	int front, back, top, bottom, left, right;
	Direction history [20000];
	int historyPtr;
	UndoStep undoStep;
};

#endif