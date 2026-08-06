
#include <iostream>
#include <cmath>
#include <ctime>
#include <vector>	
#include <random>
#include <cstring>
#include <string>
#include "glut.h"

float MapLength = 5000;
float MapHeight = 5000;

/* radius view user of x axis from center */
float UserViewRadiusX = 400;
/* radius view user of x axis from center */
float UserViewRadiusY = 233.333;

float aircraftX = -50; // initial x , centre of aircraft
float aircraftY = -950; // initial y , centre of aircraft
float aircraftSize = 15; // aircraft scale, aircraft have fixed aspect ratio 1x1
float aircraftRotation = 0;
float aircraftSpeed = 6/* 6 */;

int numOfObstaclePixel = 0;
float ObstaclesPixel[100000000][2]; // obstacle pixel coordinate

bool isObstaclePixelLoaded = false; // static obstacle, true only after all obstacle loaded, store all obstacle only once at start then stop, but still running every frame
bool aircraftEndGame = false; // our aircraft still sruvive or no

int GeneraterRandomInt(int from, int to)
{
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> dist6(from, to); // distribution in range [1, 6]

	return dist6(rng);
}

struct Bullet {
	float x;
	float y;
	float speed;
	float radius;
	float direction; // 90 left, 270 right, 180 down, 0 up

	bool operator==(const Bullet& other) const {
		// Compare relevant members to determine equality
		return x == other.x && y == other.y && speed == other.speed && radius == other.radius && direction == other.direction;
	}
};
std::vector<Bullet> allBullets; // vector to store bullets

struct EnemyBullet {
	float x;
	float y;
	float speed;
	float radius;
	int direction; // 90 left, 270 right, 180 down, 0 up

	bool operator==(const EnemyBullet& other) const {
		// Compare relevant members to determine equality
		return x == other.x && y == other.y && speed == other.speed && radius == other.radius && direction == other.direction;
	}
};
std::vector<EnemyBullet> allEnemyBullets; // vector to store bullets

struct Explosion
{
	float x;
	float y;
	int radius;
	bool operator==(const Explosion& other) const {
		// Compare relevant members to determine equality
		return x == other.x && y == other.y && radius == other.radius;
	}

	float tempX = 0;
	float tempY = 0;
};
std::vector<Explosion> allExplosion;

struct Star
{
	float x;
	float y;
	float size;
};
std::vector<Star> allStar;

struct EnemyAircraft {
	float x;
	float y;
	float speed;
	float size;

	// 90 left, 270 right, 180 down, 0 up
	int firstdirection;
	float firstdistance;
	int seconddirection;
	float seconddistance;

	/* skip when default initialize */
	int currentdirection = firstdirection;
	float turningpointx = x;
	float turningpointy = y;

	bool operator==(const EnemyAircraft& other) const {
		return x == other.x && y == other.y && speed == other.speed && size == other.size && currentdirection == other.currentdirection;
	}
};
std::vector<EnemyAircraft> allEnemyAircraft; // vector to store aircraft

int roundFloatToInt(float i)
{
	i = i + 0.5 - (i < 0);
	return (int)i;
}

/* 90 -> 270, 270 -> 90, 0 -> 180, 180 -> 0 */
int flipDirection(int direction)
{
	switch (direction)
	{
	case 0:
		return 180;
	case 90:
		return 270;
	case 180:
		return 0;
	case 270:
		return 90;
	default:
		return 0;
		break;
	}
}

/* +ve distance - > -ve distance, -ve distance to +ve distance*/
int flipDistance(float distance)
{
	return -distance;
}

void addObstaclePixel(float x, float y)
{
	bool obstaclePixelExist = false;
	for (int i = 0; i < numOfObstaclePixel; i++)
	{
		if (roundFloatToInt(x) == ObstaclesPixel[i][0] && roundFloatToInt(y) == ObstaclesPixel[i][1])
			obstaclePixelExist = true;
	}
	if (!obstaclePixelExist)
	{
		numOfObstaclePixel += 1;
		ObstaclesPixel[numOfObstaclePixel - 1][0] = roundFloatToInt(x);
		ObstaclesPixel[numOfObstaclePixel - 1][1] = roundFloatToInt(y);
	}
}

void CheckCollisionConstantly()
{
	/* collision of our aircraft with  */
	/* 1. obstacle */ for (int i = 0; i < numOfObstaclePixel; i++)
	{
		// our aircraft
		if (aircraftRotation == 90 || aircraftRotation == 270) /* collision our aircraft with obstacle for x-axis */
		{
			for (int j = 0; j <= aircraftSize; j++)
			{
				if (aircraftX - j == ObstaclesPixel[i][0] && (aircraftY - j <= ObstaclesPixel[i][1] && aircraftY + j >= ObstaclesPixel[i][1]))
					aircraftX += aircraftSpeed;
				if (aircraftX + j == ObstaclesPixel[i][0] && (aircraftY - j <= ObstaclesPixel[i][1] && aircraftY + j >= ObstaclesPixel[i][1]))
					aircraftX -= aircraftSpeed;
			}
		}
		else if (aircraftRotation == 0 || aircraftRotation == 180) /* collision our aircraft with obstacle for y-axis */
		{
			for (int j = 0; j <= aircraftSize; j++)
			{
				if (aircraftY - j == ObstaclesPixel[i][1] && (aircraftX - j <= ObstaclesPixel[i][0] && aircraftX + j >= ObstaclesPixel[i][0]))
					aircraftY += aircraftSpeed;
				if (aircraftY + j == ObstaclesPixel[i][1] && (aircraftX - j <= ObstaclesPixel[i][0] && aircraftX + j >= ObstaclesPixel[i][0]))
					aircraftY -= aircraftSpeed;
			}
		}

		//enemy aircraft prevent lag

	}
	/* 2. map boundary */
	if (aircraftX - aircraftSize < -(MapLength / 2))
		aircraftX += aircraftSpeed;
	if (aircraftX + aircraftSize > (MapLength / 2))
		aircraftX -= aircraftSpeed;
	if (aircraftY - aircraftSize < -(MapHeight / 2))
		aircraftY += aircraftSpeed;
	if (aircraftY + aircraftSize > (MapHeight / 2))
		aircraftY -= aircraftSpeed;

	/* collision of our bullet */
	for (auto& bullet : allBullets)
	{
		if (bullet.x < aircraftX - UserViewRadiusX || bullet.x > aircraftX + UserViewRadiusX || bullet.y < aircraftY - UserViewRadiusY || bullet.y > aircraftY + UserViewRadiusY)
		{
			allBullets.erase(std::remove(allBullets.begin(), allBullets.end(), bullet), allBullets.end());
		}
		else
		{
			/* collision our bullet with obstacle */
			for (int i = 0; i < numOfObstaclePixel; i++)
			{
				if (roundFloatToInt(bullet.x - bullet.radius) == (ObstaclesPixel[i][0]) && (roundFloatToInt(bullet.y - bullet.radius) <= (ObstaclesPixel[i][1]) && roundFloatToInt(bullet.y + bullet.radius) >= (ObstaclesPixel[i][1])))
				{
					allBullets.erase(std::remove(allBullets.begin(), allBullets.end(), bullet), allBullets.end());
				}
				if (roundFloatToInt(bullet.x + bullet.radius) == (ObstaclesPixel[i][0]) && (roundFloatToInt(bullet.y - bullet.radius) <= (ObstaclesPixel[i][1]) && roundFloatToInt(bullet.y + bullet.radius) >= (ObstaclesPixel[i][1])))
				{
					allBullets.erase(std::remove(allBullets.begin(), allBullets.end(), bullet), allBullets.end());
				}
				if (roundFloatToInt(bullet.y - bullet.radius) == (ObstaclesPixel[i][1]) && (roundFloatToInt(bullet.x - bullet.radius) <= (ObstaclesPixel[i][0]) && roundFloatToInt(bullet.x + bullet.radius) >= (ObstaclesPixel[i][0])))
				{
					allBullets.erase(std::remove(allBullets.begin(), allBullets.end(), bullet), allBullets.end());
				}
				if (roundFloatToInt(bullet.y + bullet.radius) == (ObstaclesPixel[i][1]) && (roundFloatToInt(bullet.x - bullet.radius) <= (ObstaclesPixel[i][0]) && roundFloatToInt(bullet.x + bullet.radius) >= ObstaclesPixel[i][0]))
				{
					allBullets.erase(std::remove(allBullets.begin(), allBullets.end(), bullet), allBullets.end());
				}
			}
		}

		/* collision our bullet with map boundary */
		if (bullet.x < -(MapLength / 2))
			allBullets.erase(std::remove(allBullets.begin(), allBullets.end(), bullet), allBullets.end());
		if (bullet.x > (MapLength / 2))
			allBullets.erase(std::remove(allBullets.begin(), allBullets.end(), bullet), allBullets.end());
		if (bullet.y < -(MapHeight / 2))
			allBullets.erase(std::remove(allBullets.begin(), allBullets.end(), bullet), allBullets.end());
		if (bullet.y > (MapHeight / 2))
			allBullets.erase(std::remove(allBullets.begin(), allBullets.end(), bullet), allBullets.end());

		/* collision our bullet with enemy aircraft */
		for (int i = 0; i < allEnemyAircraft.size(); i++)
		{
			if ((bullet.x < allEnemyAircraft[i].x + allEnemyAircraft[i].size && bullet.x > allEnemyAircraft[i].x - allEnemyAircraft[i].size) && (bullet.y < allEnemyAircraft[i].y + allEnemyAircraft[i].size && bullet.y > allEnemyAircraft[i].y - allEnemyAircraft[i].size))
			{
				allExplosion.push_back({ allEnemyAircraft[i].x ,allEnemyAircraft[i].y, 240 });
				allEnemyAircraft.erase(std::remove(allEnemyAircraft.begin(), allEnemyAircraft.end(), allEnemyAircraft[i]), allEnemyAircraft.end());
				allBullets.erase(std::remove(allBullets.begin(), allBullets.end(), bullet), allBullets.end());
			}
		}
	}

	/* collision of enemy aircraft */
	for (auto& enemyAircraft : allEnemyAircraft)//int i = 0; i < allEnemyAircraft.size(); i++
	{
		if (enemyAircraft.x > aircraftX - UserViewRadiusX && enemyAircraft.x < aircraftX + UserViewRadiusX && enemyAircraft.y > aircraftY - UserViewRadiusY && enemyAircraft.y < aircraftY + UserViewRadiusY)
		{
			/* collision enemy aircraft with our aircraft  */
			for (int i = 0; i <= aircraftSize; i++)
			{
				for (int j = 0; j < enemyAircraft.size + 1; j++)
				{
					/* left collision */
					if (roundFloatToInt(aircraftX - i) == roundFloatToInt(enemyAircraft.x + j) && roundFloatToInt(aircraftY) == roundFloatToInt(enemyAircraft.y))
					{
						if (!aircraftEndGame)
							allExplosion.push_back({ aircraftX ,aircraftY, 240 });
						aircraftEndGame = true;
					}
					/* right collision */
					if (roundFloatToInt(aircraftX + i) == roundFloatToInt(enemyAircraft.x - j) && roundFloatToInt(aircraftY) == roundFloatToInt(enemyAircraft.y))
					{
						if (!aircraftEndGame)
							allExplosion.push_back({ aircraftX ,aircraftY, 240 });
						aircraftEndGame = true;
					}
					/* bot collision */
					if (roundFloatToInt(aircraftY - i) == roundFloatToInt(enemyAircraft.y + j) && roundFloatToInt(aircraftX) == roundFloatToInt(enemyAircraft.x))
					{
						if (!aircraftEndGame)
							allExplosion.push_back({ aircraftX ,aircraftY, 240 });
						aircraftEndGame = true;
					}
					/* top collision */
					if (roundFloatToInt(aircraftY + i) == roundFloatToInt(enemyAircraft.y - j) && roundFloatToInt(aircraftX) == roundFloatToInt(enemyAircraft.x))
					{
						if (!aircraftEndGame)
							allExplosion.push_back({ aircraftX ,aircraftY, 240 });
						aircraftEndGame = true;
					}
				}
			}

			/* collision enemy aircraft with map boundary  */
			if (enemyAircraft.x - aircraftSize < -(MapLength / 2))
				enemyAircraft.currentdirection = flipDirection(enemyAircraft.currentdirection);
			if (enemyAircraft.x + aircraftSize > (MapLength / 2))
				enemyAircraft.currentdirection = flipDirection(enemyAircraft.currentdirection);
			if (enemyAircraft.y - aircraftSize < -(MapHeight / 2))
				enemyAircraft.currentdirection = flipDirection(enemyAircraft.currentdirection);
			if (enemyAircraft.y + aircraftSize > (MapHeight / 2))
				enemyAircraft.currentdirection = flipDirection(enemyAircraft.currentdirection);

			/* collision enemy aircraft with obstacle  */
			for (int i = 0; i < numOfObstaclePixel; i++)
			{
				for (auto& enemyAircraft : allEnemyAircraft)
				{

					if (roundFloatToInt(enemyAircraft.x - enemyAircraft.size) == roundFloatToInt(ObstaclesPixel[i][0]) && (roundFloatToInt(enemyAircraft.y - enemyAircraft.size) <= roundFloatToInt(ObstaclesPixel[i][1]) && roundFloatToInt(enemyAircraft.y + enemyAircraft.size) >= roundFloatToInt(ObstaclesPixel[i][1])))
						enemyAircraft.currentdirection = flipDirection(enemyAircraft.currentdirection);
					if (roundFloatToInt(enemyAircraft.x + enemyAircraft.size) == roundFloatToInt(ObstaclesPixel[i][0]) && (roundFloatToInt(enemyAircraft.y - enemyAircraft.size) <= roundFloatToInt(ObstaclesPixel[i][1]) && roundFloatToInt(enemyAircraft.y + enemyAircraft.size) >= roundFloatToInt(ObstaclesPixel[i][1])))
						enemyAircraft.currentdirection = flipDirection(enemyAircraft.currentdirection);
					if (roundFloatToInt(enemyAircraft.y - enemyAircraft.size) == roundFloatToInt(ObstaclesPixel[i][1]) && (roundFloatToInt(enemyAircraft.x - enemyAircraft.size) <= roundFloatToInt(ObstaclesPixel[i][0]) && roundFloatToInt(enemyAircraft.x + enemyAircraft.size) >= roundFloatToInt(ObstaclesPixel[i][0])))
						enemyAircraft.currentdirection = flipDirection(enemyAircraft.currentdirection);
					if (roundFloatToInt(enemyAircraft.y + enemyAircraft.size) == roundFloatToInt(ObstaclesPixel[i][1]) && (roundFloatToInt(enemyAircraft.x - enemyAircraft.size) <= roundFloatToInt(ObstaclesPixel[i][0]) && roundFloatToInt(enemyAircraft.x + enemyAircraft.size) >= roundFloatToInt(ObstaclesPixel[i][0])))
						enemyAircraft.currentdirection = flipDirection(enemyAircraft.currentdirection);
				}
			}
		}
	}

	/* collision of enemy bullet */
	for (auto& enemybullet : allEnemyBullets)/* collision enemy bullet with obstacle and our aircraft */
	{
		//prevent lag chekc obstacle every 5 distancce traveled
		if ((enemybullet.direction == 90 || enemybullet.direction == 270) && (fmod(roundFloatToInt(enemybullet.x), 5) == 0) || (enemybullet.direction == 0 || enemybullet.direction == 180) && (fmod(roundFloatToInt(enemybullet.y), 5) == 0))
		{/* collision enemy bullet with obstacle */
			if (enemybullet.x < aircraftX - UserViewRadiusX || enemybullet.x > aircraftX + UserViewRadiusX || enemybullet.y < aircraftY - UserViewRadiusY || enemybullet.y > aircraftY + UserViewRadiusY)
			{
				allEnemyBullets.erase(std::remove(allEnemyBullets.begin(), allEnemyBullets.end(), enemybullet), allEnemyBullets.end());
			}
			else
			{
				for (int i = 0; i < numOfObstaclePixel; i++)
				{
					if (roundFloatToInt(enemybullet.x - enemybullet.radius) == roundFloatToInt(ObstaclesPixel[i][0]) && (roundFloatToInt(enemybullet.y - enemybullet.radius) <= roundFloatToInt(ObstaclesPixel[i][1]) && roundFloatToInt(enemybullet.y + enemybullet.radius) >= roundFloatToInt(ObstaclesPixel[i][1])))
					{
						allEnemyBullets.erase(std::remove(allEnemyBullets.begin(), allEnemyBullets.end(), enemybullet), allEnemyBullets.end());
						break;
					}
					if (roundFloatToInt(enemybullet.x + enemybullet.radius) == roundFloatToInt(ObstaclesPixel[i][0]) && (roundFloatToInt(enemybullet.y - enemybullet.radius) <= roundFloatToInt(ObstaclesPixel[i][1]) && roundFloatToInt(enemybullet.y + enemybullet.radius) >= roundFloatToInt(ObstaclesPixel[i][1])))
					{
						allEnemyBullets.erase(std::remove(allEnemyBullets.begin(), allEnemyBullets.end(), enemybullet), allEnemyBullets.end());
						break;
					}
					if (roundFloatToInt(enemybullet.y - enemybullet.radius) == roundFloatToInt(ObstaclesPixel[i][1]) && (roundFloatToInt(enemybullet.x - enemybullet.radius) <= roundFloatToInt(ObstaclesPixel[i][0]) && roundFloatToInt(enemybullet.x + enemybullet.radius) >= roundFloatToInt(ObstaclesPixel[i][0])))
					{
						allEnemyBullets.erase(std::remove(allEnemyBullets.begin(), allEnemyBullets.end(), enemybullet), allEnemyBullets.end());
						break;
					}
					if (roundFloatToInt(enemybullet.y + enemybullet.radius) == roundFloatToInt(ObstaclesPixel[i][1]) && (roundFloatToInt(enemybullet.x - enemybullet.radius) <= roundFloatToInt(ObstaclesPixel[i][0]) && roundFloatToInt(enemybullet.x + enemybullet.radius) >= roundFloatToInt(ObstaclesPixel[i][0])))
					{
						allEnemyBullets.erase(std::remove(allEnemyBullets.begin(), allEnemyBullets.end(), enemybullet), allEnemyBullets.end());
						break;
					}
				}
			}
		}

		/* collision enemy bullet with map boundary */
		if (enemybullet.x < -(MapLength / 2))
			allEnemyBullets.erase(std::remove(allEnemyBullets.begin(), allEnemyBullets.end(), enemybullet), allEnemyBullets.end());
		if (enemybullet.x > (MapLength / 2))
			allEnemyBullets.erase(std::remove(allEnemyBullets.begin(), allEnemyBullets.end(), enemybullet), allEnemyBullets.end());
		if (enemybullet.y < -(MapHeight / 2))
			allEnemyBullets.erase(std::remove(allEnemyBullets.begin(), allEnemyBullets.end(), enemybullet), allEnemyBullets.end());
		if (enemybullet.y > (MapHeight / 2))
			allEnemyBullets.erase(std::remove(allEnemyBullets.begin(), allEnemyBullets.end(), enemybullet), allEnemyBullets.end());

		/* collision enemy bullet with our aircraft */
		if ((enemybullet.x < aircraftX + aircraftSize && enemybullet.x > aircraftX - aircraftSize) && (enemybullet.y < aircraftY + aircraftSize && enemybullet.y > aircraftY - aircraftSize))
		{
			if (!aircraftEndGame)
				allExplosion.push_back({ aircraftX ,aircraftY, 240 });

			aircraftEndGame = true;
		}
	}

}

#pragma region drawing func

void drawQuad(float x, float y, float width, float height, float red, float green, float blue)
{
	glBegin(GL_QUADS);
	glColor3f(red, green, blue);
	glVertex2f(x, y);
	glVertex2f(x, y + height);
	glVertex2f(x + width, y + height);
	glVertex2f(x + width, y);

	if (!isObstaclePixelLoaded)
	{
		for (float i = x; i < x + width; i++)
		{
			for (float j = y; j < y + height; j++)
			{
				addObstaclePixel(i, j);
			}
		}
	}

	glEnd();
}

void drawQuadOutline(float x, float y, float width, float height, float red, float green, float blue)
{
	glBegin(GL_LINE_LOOP);
	glColor3f(red, green, blue);
	glVertex2f(x, y);
	glVertex2f(x, y + height);
	glVertex2f(x + width, y + height);
	glVertex2f(x + width, y);

	if (!isObstaclePixelLoaded)
	{
		for (float i = x; i < x + width; i++)
		{
			for (float j = y; j < y + height; j++)
			{
				addObstaclePixel(i, j);
			}
		}
	}

	glEnd();
}

void drawQuadFree(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float red, float green, float blue)
{
	float minX = std::min({ x0, x1, x2, x3 });
	float maxX = std::max({ x0, x1, x2, x3 });
	float minY = std::min({ y0, y1, y2, y3 });
	float maxY = std::max({ y0, y1, y2, y3 });

	if (!isObstaclePixelLoaded)
	{
		for (float i = minX; i <= maxX; i += 1.0f)
		{
			for (float j = minY; j <= maxY; j += 1.0f)
			{
				float w0 = ((y1 - y2) * (i - x2) + (x2 - x1) * (j - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
				float w1 = ((y2 - y0) * (i - x2) + (x0 - x2) * (j - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
				float w2 = 1.0f - w0 - w1;

				float w3 = ((y0 - y3) * (i - x3) + (x3 - x0) * (j - y3)) / ((y0 - y3) * (x1 - x3) + (x3 - x0) * (y1 - y3));
				float w4 = ((y3 - y1) * (i - x3) + (x1 - x3) * (j - y3)) / ((y0 - y3) * (x1 - x3) + (x3 - x0) * (y1 - y3));

				if ((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w3 >= 0 && w4 >= 0 && w2 >= 0))
				{
					addObstaclePixel(i, j);
				}
			}
		}
	}

	glBegin(GL_QUADS);
	glColor3f(red, green, blue);
	glVertex2f(x0, y0);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glVertex2f(x3, y3);
	glEnd();
}

void drawSquare(float x, float y, float length, float red, float green, float blue)
{
	glBegin(GL_QUADS);
	glColor3f(red, green, blue);
	glVertex2f(x, y);
	glVertex2f(x, y + length);
	glVertex2f(x + length, y + length);
	glVertex2f(x + length, y);

	if (!isObstaclePixelLoaded)
	{
		for (float i = x; i < x + length; i++)
		{
			for (float j = y; j < y + length; j++)
			{
				addObstaclePixel(i, j);
			}
		}
	}

	glEnd();
}

void drawSquareOutline(float x, float y, float length, float red, float green, float blue)
{
	glBegin(GL_LINE_LOOP);
	glColor3f(red, green, blue);
	glVertex2f(x, y);
	glVertex2f(x, y + length);
	glVertex2f(x + length, y + length);
	glVertex2f(x + length, y);

	if (!isObstaclePixelLoaded)
	{
		for (float i = x; i < x + length; i++)
		{
			for (float j = y; j < y + length; j++)
			{
				addObstaclePixel(i, j);
			}
		}
	}

	glEnd();
}

void drawSquareFree(float x, float y, float length, float red, float green, float blue)
{
	float halfLength = length / 2.0f;
	float x0 = x - halfLength;
	float y0 = y - halfLength;
	float x1 = x + halfLength;
	float y1 = y - halfLength;
	float x2 = x + halfLength;
	float y2 = y + halfLength;
	float x3 = x - halfLength;
	float y3 = y + halfLength;

	float minX = std::min({ x0, x1, x2, x3 });
	float maxX = std::max({ x0, x1, x2, x3 });
	float minY = std::min({ y0, y1, y2, y3 });
	float maxY = std::max({ y0, y1, y2, y3 });

	if (!isObstaclePixelLoaded)
	{
		for (float i = minX; i <= maxX; i += 1.0f)
		{
			for (float j = minY; j <= maxY; j += 1.0f)
			{
				float w0 = ((y1 - y2) * (i - x2) + (x2 - x1) * (j - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
				float w1 = ((y2 - y0) * (i - x2) + (x0 - x2) * (j - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
				float w2 = 1.0f - w0 - w1;

				float w3 = ((y0 - y3) * (i - x3) + (x3 - x0) * (j - y3)) / ((y0 - y3) * (x1 - x3) + (x3 - x0) * (y1 - y3));
				float w4 = ((y3 - y1) * (i - x3) + (x1 - x3) * (j - y3)) / ((y0 - y3) * (x1 - x3) + (x3 - x0) * (y1 - y3));

				if ((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w3 >= 0 && w4 >= 0 && w2 >= 0))
				{
					addObstaclePixel(i, j);
				}
			}
		}
	}

	glBegin(GL_QUADS);
	glColor3f(red, green, blue);
	glVertex2f(x0, y0);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glVertex2f(x3, y3);
	glEnd();
}

void drawTriangle(float x, float y, float length, float red, float green, float blue)
{
	float halfLength = length / 2.0f;
	float x0 = x;
	float y0 = y;
	float x1 = x + length;
	float y1 = y;
	float x2 = x + halfLength;
	float y2 = y + length;

	float minX = std::min({ x0, x1, x2 });
	float maxX = std::max({ x0, x1, x2 });
	float minY = std::min({ y0, y1, y2 });
	float maxY = std::max({ y0, y1, y2 });

	if (!isObstaclePixelLoaded)
	{
		for (float i = minX; i <= maxX; i += 1.0f)
		{
			for (float j = minY; j <= maxY; j += 1.0f)
			{
				float w0 = ((y1 - y2) * (i - x2) + (x2 - x1) * (j - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
				float w1 = ((y2 - y0) * (i - x2) + (x0 - x2) * (j - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
				float w2 = 1.0f - w0 - w1;

				if (w0 >= 0 && w1 >= 0 && w2 >= 0)
				{
					addObstaclePixel((i), (j));
				}
			}
		}
	}

	glBegin(GL_TRIANGLES);
	glColor3f(red, green, blue);
	glVertex2f(x0, y0);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();
}

void drawTriangleOutline(float x, float y, float length, float red, float green, float blue)
{
	float halfLength = length / 2.0f;
	float x0 = x;
	float y0 = y;
	float x1 = x + length;
	float y1 = y;
	float x2 = x + halfLength;
	float y2 = y + length;

	float minX = std::min({ x0, x1, x2 });
	float maxX = std::max({ x0, x1, x2 });
	float minY = std::min({ y0, y1, y2 });
	float maxY = std::max({ y0, y1, y2 });

	if (!isObstaclePixelLoaded)
	{
		for (float i = minX; i <= maxX; i += 1.0f)
		{
			for (float j = minY; j <= maxY; j += 1.0f)
			{
				float w0 = ((y1 - y2) * (i - x2) + (x2 - x1) * (j - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
				float w1 = ((y2 - y0) * (i - x2) + (x0 - x2) * (j - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
				float w2 = 1.0f - w0 - w1;

				if (w0 >= 0 && w1 >= 0 && w2 >= 0)
				{
					addObstaclePixel(i, j);
				}
			}
		}
	}

	glBegin(GL_LINE_LOOP);
	glColor3f(red, green, blue);
	glVertex2f(x0, y0);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();
}

void drawIsoscelesTriangle(float x, float y, float length, float height, float red, float green, float blue)
{
	float halfLength = length / 2.0f;
	float x0 = x;
	float y0 = y;
	float x1 = x + length;
	float y1 = y;
	float x2 = x + halfLength;
	float y2 = y + height;

	float minX = std::min({ x0, x1, x2 });
	float maxX = std::max({ x0, x1, x2 });
	float minY = std::min({ y0, y1, y2 });
	float maxY = std::max({ y0, y1, y2 });

	if (!isObstaclePixelLoaded)
	{
		for (float i = minX; i <= maxX; i += 1.0f)
		{
			for (float j = minY; j <= maxY; j += 1.0f)
			{
				float w0 = ((y1 - y2) * (i - x2) + (x2 - x1) * (j - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
				float w1 = ((y2 - y0) * (i - x2) + (x0 - x2) * (j - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
				float w2 = 1.0f - w0 - w1;

				if (w0 >= 0 && w1 >= 0 && w2 >= 0)
				{
					addObstaclePixel((i), (j));
				}
			}
		}
	}

	glBegin(GL_TRIANGLES);
	glColor3f(red, green, blue);
	glVertex2f(x0, y0);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();
}

void drawIsoscelesOutlineTriangle(float x, float y, float length, float height, float red, float green, float blue)
{
	float halfLength = length / 2.0f;
	float x0 = x;
	float y0 = y;
	float x1 = x + length;
	float y1 = y;
	float x2 = x + halfLength;
	float y2 = y + height;

	float minX = std::min({ x0, x1, x2 });
	float maxX = std::max({ x0, x1, x2 });
	float minY = std::min({ y0, y1, y2 });
	float maxY = std::max({ y0, y1, y2 });

	if (!isObstaclePixelLoaded)
	{
		for (float i = minX; i <= maxX; i += 1.0f)
		{
			for (float j = minY; j <= maxY; j += 1.0f)
			{
				float w0 = ((y1 - y2) * (i - x2) + (x2 - x1) * (j - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
				float w1 = ((y2 - y0) * (i - x2) + (x0 - x2) * (j - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
				float w2 = 1.0f - w0 - w1;

				if (w0 >= 0 && w1 >= 0 && w2 >= 0)
				{
					addObstaclePixel((i), (j));
				}
			}
		}
	}

	glBegin(GL_LINE_LOOP);
	glColor3f(red, green, blue);
	glVertex2f(x0, y0);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();
}

void drawCircle(float x, float y, float radius, float red, float green, float blue)
{
	float theta;
	glBegin(GL_POLYGON);
	glColor3f(red, green, blue);
	for (float i = 0; i < 2 * 3.142; i += 3.142 / 4000)
	{
		glVertex2f(x - cos(i) * radius, y - sin(i) * radius);
		if (!isObstaclePixelLoaded)
		{
			addObstaclePixel(roundFloatToInt(x - cos(i) * radius), roundFloatToInt(y - sin(i) * radius));
		}
	}

	glEnd();
}

void drawCircleOutline(float x, float y, float radius, float red, float green, float blue)
{
	float theta;
	glBegin(GL_LINE_LOOP);
	glColor3f(red, green, blue);
	for (float i = 0; i < 2 * 3.142; i += 3.142 / 200)
	{
		glVertex2f(x - cos(i) * radius, y - sin(i) * radius);
		if (!isObstaclePixelLoaded)
		{
			addObstaclePixel(roundFloatToInt(x - cos(i) * radius), roundFloatToInt(y - sin(i) * radius));
		}
	}

	glEnd();
}

void drawText(int x, int y, float r, float g, float b, void* font, const char* string)
{
	glColor3f(r, g, b);
	glRasterPos2f(x, y);
	int len, i;
	len = (int)strlen(string);
	for (i = 0; i < len; i++) {
		glutBitmapCharacter(font, string[i]);
	}
}

void drawText(int x, int y, float r, float g, float b, void* font, std::string string)
{
	glColor3f(r, g, b);
	glRasterPos2f(x, y);
	int len, i;
	len = string.length();
	for (i = 0; i < len; i++) {
		glutBitmapCharacter(font, string[i]);
	}
}

void drawCirclePlanet(float x, float y, float radius, float red, float green, float blue)
{
	glBegin(GL_POLYGON);
	glColor3f(red, green, blue);
	for (float i = 0; i < 2 * 3.142; i += 3.142 / 400)
	{
		float percent = float(i) / float(2 * 3.142);
		float darkColor = 0.7;
		glColor3f(red * (percent + darkColor), green * (percent + darkColor), blue * (percent + darkColor));

		glVertex2f(x - cos(i) * radius, y - sin(i) * radius);
		if (!isObstaclePixelLoaded)
		{
			addObstaclePixel(roundFloatToInt(x - cos(i) * radius), roundFloatToInt(y - sin(i) * radius));
		}
	}

	glEnd();
}

void drawStar(float x, float y, float size, float red, float green, float blue)
{
	float halfSize = size / 2.0f;
	float quarterSize = size / 4.0f;

	glBegin(GL_TRIANGLES);
	glColor3f(red, green, blue);
	// Lower left triangle
	glVertex2f(x - halfSize, y);
	glVertex2f(x - quarterSize, y - size);
	glVertex2f(x + quarterSize, y - size);

	// Upper triangle
	glVertex2f(x - quarterSize, y - size);
	glVertex2f(x + quarterSize, y - size);
	glVertex2f(x, y - size / 2.0f);

	// Lower right triangle
	glVertex2f(x + quarterSize, y - size);
	glVertex2f(x + halfSize, y);
	glVertex2f(x, y - size / 2.0f);
	glEnd();
}

void drawTriangleFree(float x0, float y0, float x1, float y1, float x2, float y2, float red, float green, float blue)
{
	float minX = std::min({ x0, x1, x2 });
	float maxX = std::max({ x0, x1, x2 });
	float minY = std::min({ y0, y1, y2 });
	float maxY = std::max({ y0, y1, y2 });

	if (!isObstaclePixelLoaded)
	{
		for (float i = minX; i <= maxX; i += 1.0f)
		{
			for (float j = minY; j <= maxY; j += 1.0f)
			{
				float w0 = ((y1 - y2) * (i - x2) + (x2 - x1) * (j - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
				float w1 = ((y2 - y0) * (i - x2) + (x0 - x2) * (j - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
				float w2 = 1.0f - w0 - w1;

				if (w0 >= 0 && w1 >= 0 && w2 >= 0)
				{
					addObstaclePixel(i, j);
				}
			}
		}
	}

	glBegin(GL_TRIANGLES);
	glColor3f(red, green, blue);
	glVertex2f(x0, y0);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();
}

void drawTriangleOutlineFree(float x, float y, float length, float red, float green, float blue)
{
	float halfLength = length / 2.0f;
	float x0 = x;
	float y0 = y;
	float x1 = x + length;
	float y1 = y;
	float x2 = x + halfLength;
	float y2 = y + length;

	float minX = std::min({ x0, x1, x2 });
	float maxX = std::max({ x0, x1, x2 });
	float minY = std::min({ y0, y1, y2 });
	float maxY = std::max({ y0, y1, y2 });

	if (!isObstaclePixelLoaded)
	{
		for (float i = minX; i <= maxX; i += 1.0f)
		{
			for (float j = minY; j <= maxY; j += 1.0f)
			{
				float w0 = ((y1 - y2) * (i - x2) + (x2 - x1) * (j - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
				float w1 = ((y2 - y0) * (i - x2) + (x0 - x2) * (j - y2)) / ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
				float w2 = 1.0f - w0 - w1;

				if (w0 >= 0 && w1 >= 0 && w2 >= 0)
				{
					addObstaclePixel(i, j);
				}
			}
		}
	}

	glBegin(GL_LINE_LOOP);
	glColor3f(red, green, blue);
	glVertex2f(x0, y0);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();
}

#pragma endregion

void DrawMap()
{

	// mid - sun
	drawCirclePlanet(0, 0, 500, 1, 0.8, 0.2); // Adjusted sun color

	//top left - planet
	drawCirclePlanet(-987.81131, 260.34389, 75, 0.2, 0.6, 1);  // Adjusted color
	drawCirclePlanet(-1342.52677, 702.36544, 30, 0.5, 0.2, 0.8);  // Adjusted color
	drawCirclePlanet(-800, 800, 100, 0.8, 0.4, 0);  // Adjusted color
	drawCirclePlanet(-1882.73658, 567.40596, 175, 0.2, 0.6, 1);  // Adjusted color
	drawCirclePlanet(-1825.23721, 1201.82773, 100, 0.5, 0.2, 0.8);  // Adjusted color
	drawCirclePlanet(-447.793, 1360.65449, 70, 0.8, 0.4, 0);  // Adjusted color
	drawCirclePlanet(-1224.11206, 2069.11799, 225, 0.2, 0.6, 1);  // Adjusted color

	// top left - additional obstacles (more spread out)
	drawSquare(-1300.0, 1300.0, 100, 1, 1, 0);
	drawTriangle(-1300.0, 1400.0, 100, 1, 1, 0);
	drawTriangleFree(-1400, 1350, -1300.0, 1300.0, -1300.0, 1400.0, 1, 1, 0);
	drawTriangleFree(-1200.0, 1400.0, -1200.0, 1300.0, -1100, 1350, 1, 1, 0);
	drawTriangleFree(-1300.0, 1300.0, -1200.0, 1300.0, -1250.7, 1200.3, 1, 1, 0);

	drawQuadFree(-50.000, 2300.0, -100.00, 2250.0, -50.000, 2200.0, 0, 2250.0, 1, 1, 0);
	drawTriangleFree(-100.00, 2250.0, -50.000, 2300.0, -150.00, 2350.0, 1, 1, 0);
	drawTriangleFree(-100.00, 2250.0, -50.000, 2200.0, -150.00, 2150.0, 1, 1, 0);
	drawTriangleFree(-50.000, 2200.0, 0, 2250.0, 50.000, 2150.0, 1, 1, 0);
	drawTriangleFree(-50.000, 2300.0, 0, 2250, 50.000, 2350.0, 1, 1, 0);

	//drawTriangleFree(0, 0, 0, 1, 1, 0);

	//top right
	drawCirclePlanet(617.33692, 2091.01948, 100, 0.8, 0.4, 0);  // Adjusted color
	drawCirclePlanet(2043.47458, 1943.02407, 250, 0.2, 0.6, 1);  // Adjusted color
	drawCirclePlanet(1400, 1200, 100, 0.5, 0.2, 0.8);  // Adjusted color
	drawCirclePlanet(1304.49861, 249.1899, 250, 0.8, 0.4, 0);  // Adjusted color
	drawCirclePlanet(303.74193, 771.32134, 150, 0.2, 0.6, 1);  // Adjusted color
	drawCirclePlanet(2339.46541, 745.60661, 125, 0.5, 0.2, 0.8);  // Adjusted color

	// top right - additional obstacles 
	drawQuadFree(1600.0, 800.00, 1650.0, 750.00, 1700.0, 800.00, 1650.0, 850.00, 1, 1, 0);
	drawTriangleFree(1600.0, 800.00, 1650.0, 850.00, 1550.0, 900.00, 1, 1, 0);
	drawTriangleFree(1600.0, 800.00, 1650.0, 750.00, 1550.0, 700.00, 1, 1, 0);
	drawTriangleFree(1650.0, 750.00, 1700.0, 800.00, 1750.0, 700.00, 1, 1, 0);
	drawTriangleFree(1650.0, 850.00, 1700.0, 800.00, 1750.0, 900.00, 1, 1, 0);

	// bot left
	drawCirclePlanet(-1444.45373, -345.23489, 100, 0.8, 0.4, 0);  // Adjusted color
	drawCirclePlanet(-589.32584, -651.58874, 110, 0.2, 0.6, 1);  // Adjusted color
	drawCirclePlanet(-1154.89441, -1169.18417, 70, 0.5, 0.2, 0.8);  // Adjusted color
	drawCirclePlanet(-692.19828, -1801.70845, 240, 0.8, 0.4, 0);  // Adjusted color
	drawCirclePlanet(-2172.15245, -2205.33231, 110, 0.2, 0.6, 1);  // Adjusted color
	drawCirclePlanet(-2051.06529, -1146.94084, 125, 0.5, 0.2, 0.8);  // Adjusted color

	// bot left - additional obstacles (more spread out) 
	drawQuadFree(-1800.0, -1600.0, -1750.0, -1650.0, -1700.0, -1600.0, -1750.0, -1550.0, 1, 1, 0);
	drawTriangleFree(-1800.0, -1600.0, -1750.0, -1550.0, -1850.0, -1500.0, 1, 1, 0);
	drawTriangleFree(-1800.0, -1600.0, -1750.0, -1650.0, -1850.0, -1700.0, 1, 1, 0);
	drawTriangleFree(-1750.0, -1650.0, -1700.0, -1600.0, -1650.0, -1700.0, 1, 1, 0);
	drawTriangleFree(-1750.0, -1550.0, -1700.0, -1600.0, -1650.0, -1500.0, 1, 1, 0);

	//bot right
	drawCirclePlanet(890.62105, -255.19381, 125, 0.8, 0.4, 0);  // Adjusted color
	drawCirclePlanet(153.72137, -614.8411, 80, 0.2, 0.6, 1);  // Adjusted color
	drawCirclePlanet(1028.94693, -1117.84429, 150, 0.5, 0.2, 0.8);  // Adjusted color
	drawCirclePlanet(294.43783, -1635.77419, 100, 0.8, 0.4, 0);  // Adjusted color
	drawCirclePlanet(1882.02503, -2016.97451, 225, 0.2, 0.6, 1);  // Adjusted color
	drawCirclePlanet(2146.6229, -1191.78794, 90, 0.5, 0.2, 0.8);  // Adjusted color

	// bot right - additional obstacles (more spread out)
	drawQuadFree(600.00, -2100.0, 700.00, -2200.0, 800.00, -2100.0, 700.00, -2000.0, 1, 1, 0);
	drawTriangleFree(600.00, -2100.0, 700.00, -2000.0, 500.00, -1900.0, 1, 1, 0);
	drawTriangleFree(600.00, -2100.0, 700.00, -2200.0, 500.00, -2300.0, 1, 1, 0);
	drawTriangleFree(700.00, -2000.0, 800.00, -2100.0, 900.00, -1900.0, 1, 1, 0);
	drawTriangleFree(700.00, -2200.0, 800.00, -2100.0, 900.00, -2300.0, 1, 1, 0);

	isObstaclePixelLoaded = true;

	//drawCirclePlanet(0, 0, 700, 1, 0.8, 0.2);
	drawQuadOutline(-(MapLength / 2), -(MapHeight / 2), MapLength, MapHeight, 1, 1, 1); // map boundary
}

void ExplosionAnimation()
{
	static int lastTime = glutGet(GLUT_ELAPSED_TIME);

	// Get the elapsed time since the last frame
	int currentTime = glutGet(GLUT_ELAPSED_TIME);
	float deltaTime = (currentTime - lastTime) / 50000.0f;

	for (auto& explosion : allExplosion)
	{
		drawCircle(explosion.x - (explosion.tempX * deltaTime), explosion.y, 2, 1, 0.388, 0.278);
		drawCircle(explosion.x + (explosion.tempX * deltaTime), explosion.y, 2, 1, 0.388, 0.278);
		drawCircle(explosion.x, explosion.y - (explosion.tempY * deltaTime), 2, 1, 0.388, 0.278);
		drawCircle(explosion.x, explosion.y + (explosion.tempY * deltaTime), 2, 1, 0.388, 0.278);
		drawCircle(explosion.x - (explosion.tempX * deltaTime), explosion.y - (explosion.tempY * deltaTime), 2, 1, 0.388, 0.278);
		drawCircle(explosion.x + (explosion.tempX * deltaTime), explosion.y - (explosion.tempY * deltaTime), 2, 1, 0.388, 0.278);
		drawCircle(explosion.x - (explosion.tempX * deltaTime), explosion.y + (explosion.tempY * deltaTime), 2, 1, 0.388, 0.278);
		drawCircle(explosion.x + (explosion.tempX * deltaTime), explosion.y + (explosion.tempY * deltaTime), 2, 1, 0.388, 0.278);

		explosion.tempX++;
		explosion.tempY++;
		//explosion.radius--;

		if (explosion.radius == explosion.tempX)
			allExplosion.erase(std::remove(allExplosion.begin(), allExplosion.end(), explosion), allExplosion.end());
	}



}

void AircraftAnimation()
{
	if (!aircraftEndGame)
	{
		glPushMatrix();

		/* aircraft rotation */
		glTranslatef(aircraftX, aircraftY, 0);
		glRotatef(aircraftRotation, 0, 0, 1);
		glTranslatef(-aircraftX - aircraftSize / 2, -aircraftY - aircraftSize / 2, 0);

		/* aircraft shape */
		drawIsoscelesTriangle(aircraftX - aircraftSize / 2, aircraftY + aircraftSize / 2, aircraftSize * 2, aircraftSize, 0, 0, 1);
		drawSquare(aircraftX, aircraftY - aircraftSize / 4, aircraftSize, 0, 0, 1);
		if (aircraftRotation == 0 || aircraftRotation == 270)
			drawCircle(aircraftX + aircraftSize / 2.1, aircraftY - aircraftSize / 4, aircraftSize / 2, 0, 0, 1);
		if (aircraftRotation == 90 || aircraftRotation == 180)
			drawCircle(aircraftX + aircraftSize / 1.9, aircraftY - aircraftSize / 4, aircraftSize / 2, 0, 0, 1);

		glPopMatrix();

		/* aircraft bullet */
		static int lastTime = glutGet(GLUT_ELAPSED_TIME); // get the current time

		// Get the elapsed time since the last frame
		int currentTime = glutGet(GLUT_ELAPSED_TIME);
		float deltaTime = (currentTime - lastTime) / 1000.0f; // convert to seconds

		for (auto& bullet : allBullets)
		{
			if (bullet.direction == 90)	// update bullet position based on the elapsed time
			{
				bullet.x -= bullet.speed * deltaTime;
				drawCircle(bullet.x, bullet.y, bullet.radius, 1, 0, 0);
			}
			if (bullet.direction == 270)
			{
				bullet.x += bullet.speed * deltaTime;
				drawCircle(bullet.x, bullet.y, bullet.radius, 1, 0, 0);
			}
			if (bullet.direction == 180)
			{
				bullet.y -= bullet.speed * deltaTime;
				drawCircle(bullet.x, bullet.y, bullet.radius, 1, 0, 0);
			}
			if (bullet.direction == 0)
			{
				bullet.y += bullet.speed * deltaTime;
				drawCircle(bullet.x, bullet.y, bullet.radius, 1, 0, 0);
			}

		}
		lastTime = currentTime; // set current frame to last frame
	}
}

void EnemyAircraftAnimation()
{
	static int lastTime = glutGet(GLUT_ELAPSED_TIME);
	int currentTime = glutGet(GLUT_ELAPSED_TIME);
	float deltaTime = (currentTime - lastTime) / 1000.0f;

	/* enemy aircraft automatic forward movement, automatic rotation and dirstance*/
	for (auto& enemyaircraft : allEnemyAircraft)
	{
		if (enemyaircraft.x > aircraftX - UserViewRadiusX && enemyaircraft.x < aircraftX + UserViewRadiusX && enemyaircraft.y > aircraftY - UserViewRadiusY && enemyaircraft.y < aircraftY + UserViewRadiusY)
		{
			glPushMatrix();

			/* aircraft rotation */
			glTranslatef(enemyaircraft.x, enemyaircraft.y, 0);
			glRotatef(enemyaircraft.currentdirection, 0, 0, 1);
			glTranslatef(-enemyaircraft.x - enemyaircraft.size / 2, -enemyaircraft.y - enemyaircraft.size / 2, 0);

			/* aircraft shape */
			drawIsoscelesTriangle(enemyaircraft.x - enemyaircraft.size / 2, enemyaircraft.y + enemyaircraft.size / 2, enemyaircraft.size * 2, enemyaircraft.size, 1, 0, 0);
			drawSquare(enemyaircraft.x, enemyaircraft.y - enemyaircraft.size / 4, enemyaircraft.size, 1, 0, 0);
			if (aircraftRotation == 0 || aircraftRotation == 270)
				drawCircle(enemyaircraft.x + enemyaircraft.size / 2.1, enemyaircraft.y - enemyaircraft.size / 4, enemyaircraft.size / 2, 1, 0, 0);
			if (aircraftRotation == 90 || aircraftRotation == 180)
				drawCircle(enemyaircraft.x + enemyaircraft.size / 1.9, enemyaircraft.y - enemyaircraft.size / 4, enemyaircraft.size / 2, 1, 0, 0);

			glPopMatrix();

			if (enemyaircraft.currentdirection == 90)
			{
				enemyaircraft.x -= enemyaircraft.speed * deltaTime;

				//drawCircle(enemyaircraft.x, enemyaircraft.y, enemyaircraft.size, 1, 0, 0);

				if (roundFloatToInt(enemyaircraft.x) % 40 == 0)
					allEnemyBullets.push_back({ enemyaircraft.x, enemyaircraft.y, 100, 4, enemyaircraft.currentdirection });
			}
			if (enemyaircraft.currentdirection == 270)
			{
				enemyaircraft.x += enemyaircraft.speed * deltaTime;
				//drawCircle(enemyaircraft.x, enemyaircraft.y, enemyaircraft.size, 1, 0, 0);
				if (roundFloatToInt(enemyaircraft.x) % 40 == 0)
					allEnemyBullets.push_back({ enemyaircraft.x, enemyaircraft.y, 100, 4, enemyaircraft.currentdirection });
			}
			if (enemyaircraft.currentdirection == 180)
			{
				enemyaircraft.y -= enemyaircraft.speed * deltaTime;
				//drawCircle(enemyaircraft.x, enemyaircraft.y, enemyaircraft.size, 1, 0, 0);
				if (roundFloatToInt(enemyaircraft.y) % 40 == 0)
					allEnemyBullets.push_back({ enemyaircraft.x, enemyaircraft.y, 100, 4, enemyaircraft.currentdirection });
			}
			if (enemyaircraft.currentdirection == 0)
			{
				enemyaircraft.y += enemyaircraft.speed * deltaTime;
				//drawCircle(enemyaircraft.x, enemyaircraft.y, enemyaircraft.size, 1, 0, 0);
				if (roundFloatToInt(enemyaircraft.y) % 40 == 0)
					allEnemyBullets.push_back({ enemyaircraft.x, enemyaircraft.y, 100, 4, enemyaircraft.currentdirection });
			}

			/* enter first direction path */
			if (enemyaircraft.currentdirection == enemyaircraft.firstdirection)
			{
				/* x-axis */
				if (enemyaircraft.firstdirection == 90 || enemyaircraft.firstdirection == 270)
				{
					if (roundFloatToInt(enemyaircraft.x) == roundFloatToInt(enemyaircraft.turningpointx + enemyaircraft.firstdistance))
						enemyaircraft.currentdirection = flipDirection(enemyaircraft.firstdirection);
				}

				/* y-axis */
				if (enemyaircraft.firstdirection == 0 || enemyaircraft.firstdirection == 180)
				{
					if (roundFloatToInt(enemyaircraft.y) == roundFloatToInt(enemyaircraft.turningpointy + enemyaircraft.firstdistance))
						enemyaircraft.currentdirection = flipDirection(enemyaircraft.firstdirection);
				}
			}
			/* enter second direction path */
			else if (enemyaircraft.currentdirection == flipDirection(enemyaircraft.firstdirection))
			{
				/* x-axis */
				if (flipDirection(enemyaircraft.firstdirection) == 90 || flipDirection(enemyaircraft.firstdirection) == 270)
				{
					if (roundFloatToInt(enemyaircraft.x) == roundFloatToInt(enemyaircraft.turningpointx))
						enemyaircraft.currentdirection = enemyaircraft.seconddirection;
				}

				/* y-axis */
				if (flipDirection(enemyaircraft.firstdirection) == 0 || flipDirection(enemyaircraft.firstdirection) == 180)
				{
					if (roundFloatToInt(enemyaircraft.y) == roundFloatToInt(enemyaircraft.turningpointy))
						enemyaircraft.currentdirection = enemyaircraft.seconddirection;
				}
			}
			/* enter third direction path */
			else if (enemyaircraft.currentdirection == enemyaircraft.seconddirection)
			{
				/* x-axis */
				if (enemyaircraft.seconddirection == 90 || enemyaircraft.seconddirection == 270)
				{
					if (roundFloatToInt(enemyaircraft.x) == roundFloatToInt(enemyaircraft.turningpointx + enemyaircraft.seconddistance))
						enemyaircraft.currentdirection = flipDirection(enemyaircraft.seconddirection);
				}

				/* y-axis */
				if (enemyaircraft.seconddirection == 0 || enemyaircraft.seconddirection == 180)
				{
					if (roundFloatToInt(enemyaircraft.y) == roundFloatToInt(enemyaircraft.turningpointy + enemyaircraft.seconddistance))
						enemyaircraft.currentdirection = flipDirection(enemyaircraft.seconddirection);
				}
			}
			/* enter fourth direction path */
			else if (enemyaircraft.currentdirection == flipDirection(enemyaircraft.seconddirection))
			{
				/* x-axis */
				if (flipDirection(enemyaircraft.seconddirection) == 90 || flipDirection(enemyaircraft.seconddirection) == 270)
				{
					if (roundFloatToInt(enemyaircraft.x) == roundFloatToInt(enemyaircraft.turningpointx))
						enemyaircraft.currentdirection = enemyaircraft.firstdirection;
				}

				/* y-axis */
				if (flipDirection(enemyaircraft.seconddirection) == 0 || flipDirection(enemyaircraft.seconddirection) == 180)
				{
					if (roundFloatToInt(enemyaircraft.y) == roundFloatToInt(enemyaircraft.turningpointy))
						enemyaircraft.currentdirection = enemyaircraft.firstdirection;
				}
			}

		}
	}

	/* enemy aircraft automaatic bullet*/
	for (auto& bullet : allEnemyBullets)
	{
		if (bullet.direction == 90)
		{
			bullet.x -= bullet.speed * deltaTime;
			drawCircle(bullet.x, bullet.y, bullet.radius, 1, 0, 0);
		}
		if (bullet.direction == 270)
		{
			bullet.x += bullet.speed * deltaTime;
			drawCircle(bullet.x, bullet.y, bullet.radius, 1, 0, 0);
		}
		if (bullet.direction == 180)
		{
			bullet.y -= bullet.speed * deltaTime;
			drawCircle(bullet.x, bullet.y, bullet.radius, 1, 0, 0);
		}
		if (bullet.direction == 0)
		{
			bullet.y += bullet.speed * deltaTime;
			drawCircle(bullet.x, bullet.y, bullet.radius, 1, 0, 0);
		}
	}
	lastTime = currentTime;
}

/* load enemy one point using EnemyAircraftSpawn(). to lock use isEnemyLoaded = true, to unlock use isEnemyLoaded = false */
bool isEnemyLoaded = false;

/* if first direction is 90(going left), first distance ro go left should be negative n vice versa */
void EnemyAircraftSpawn(float x, float y, float speed, float size, int firstdirection, float firstdistance, int seconddirection, float seconddistance)
{
	allEnemyAircraft.push_back({ x, y, speed, size , firstdirection ,firstdistance,  seconddirection, seconddistance });
}

/* pause game */
bool isPaused = false;
bool isStarDrew = false;
int totalEnemyAircraftLoaded = 0;
void display()
{
	glClearColor(0.02, 0.0, 0.07, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	/* if game is not pause */

	if (!isPaused)
	{
		if (!aircraftEndGame)
		{
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			//gluOrtho2D(-600, 600, -350, 350);
			gluOrtho2D(aircraftX - 400, aircraftX + 400, aircraftY - 233.333, aircraftY + 233.333);
			//gluOrtho2D(aircraftX -2500, aircraftX + 2500, aircraftY - 1458, aircraftY + 1458);

			if (isStarDrew)
			{
				for (auto& star : allStar)
				{
					drawStar(star.x, star.y, star.size, 1, 1, 1);
				}
			}
			else // false
			{
				int i = 0;
				while (i < 1000)
				{
					float starX = GeneraterRandomInt(-(MapLength / 2), (MapLength / 2));
					float starY = GeneraterRandomInt(-(MapHeight / 2), (MapHeight / 2));
					allStar.push_back({ starX,starY,4 });
					i++;
				}
				isStarDrew = true;
			}

			DrawMap();

			AircraftAnimation();

			if (!isEnemyLoaded)
			{
				for (int i = 0; i < 10; i++)
				{
					int randX = GeneraterRandomInt(-2450, 2450);
					int randY = GeneraterRandomInt(-2450, 2450);

					for (int i = 0; i < numOfObstaclePixel; i++)
					{
						while (randX == ObstaclesPixel[i][0] && randY == ObstaclesPixel[i][1])
						{
							randX = GeneraterRandomInt(-2450, 2450);
							randY = GeneraterRandomInt(-2450, 2450);
							i = 0;
						}
					}

					int randFirstDirection = 0;
					int randGenFirstDirection = GeneraterRandomInt(1, 4);
					if (randGenFirstDirection == 1)
						randFirstDirection = 0;
					else if (randGenFirstDirection == 2)
						randFirstDirection = 90;
					else if (randGenFirstDirection == 3)
						randFirstDirection = 180;
					else if (randGenFirstDirection == 4)
						randFirstDirection = 270;

					int randFirstDistance = 0;
					if (randGenFirstDirection == 90 || randGenFirstDirection == 180)
						randFirstDistance = GeneraterRandomInt(-400, -50);
					else if (randGenFirstDirection == 270 || randGenFirstDirection == 0)
						randFirstDistance = GeneraterRandomInt(50, 400);

					int randSecondDirection = 0;
					int randGenSecondDirection = GeneraterRandomInt(1, 4);
					if (randGenSecondDirection == 1)
						randSecondDirection = 0;
					else if (randGenSecondDirection == 2)
						randSecondDirection = 90;
					else if (randGenSecondDirection == 3)
						randSecondDirection = 180;
					else if (randGenSecondDirection == 4)
						randSecondDirection = 270;

					int randSecondDistance = 0;
					if (randSecondDirection == 90 || randSecondDirection == 180)
						randSecondDistance = GeneraterRandomInt(-400, -50);
					else if (randSecondDirection == 270 || randSecondDirection == 0)
						randSecondDistance = GeneraterRandomInt(50, 400);


					EnemyAircraftSpawn(randX, randY, 30, 15, randFirstDirection, randFirstDistance, randSecondDirection, randSecondDistance);
				}

				totalEnemyAircraftLoaded = allEnemyAircraft.size();
				//EnemyAircraftSpawn(xaxis, yaxis, 30, 15, firstdirec, firstdis, seconddirec, seconddist);

			}
			isEnemyLoaded = true;



			EnemyAircraftAnimation();

			CheckCollisionConstantly();

			ExplosionAnimation();

			drawText(aircraftX + UserViewRadiusX - 100, aircraftY + UserViewRadiusY - 30, 1, 0.33, 1, GLUT_BITMAP_HELVETICA_12, "Press M to Main Menu");
			drawText(aircraftX + UserViewRadiusX - 200, aircraftY - UserViewRadiusY + 30, 1, 0.33, 1, GLUT_BITMAP_HELVETICA_12, "by Amier");

			// prevent aircraft from going outside map
		}
		else
		{
			isPaused = true;
		}
		if (allEnemyAircraft.size() == 0)
		{
			isPaused = true;
		}
	}
	/* if game paused */
	else
	{


		if (aircraftEndGame)
		{
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluOrtho2D(-600, 600, -350, 350);
			drawText(-400, 200, 1, 0, 0, GLUT_BITMAP_9_BY_15, "YOU LOST !");
			drawText(-400, 50, 1, 0, 0, GLUT_BITMAP_9_BY_15, (std::to_string(totalEnemyAircraftLoaded - allEnemyAircraft.size()) + "/" + std::to_string(totalEnemyAircraftLoaded)) + " enemy aircraft were destroyed");
			drawText(-400, -200, 1, 0.33, 1, GLUT_BITMAP_9_BY_15, "82688 - Amier");
		}
		else if (allEnemyAircraft.size() == 0)
		{
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluOrtho2D(-600, 600, -350, 350);
			drawText(-400, 200, 0, 1, 0, GLUT_BITMAP_9_BY_15, "YOU WON !");
			drawText(-400, 50, 0, 1, 0, GLUT_BITMAP_9_BY_15, (std::to_string(totalEnemyAircraftLoaded - allEnemyAircraft.size()) + "/" + std::to_string(totalEnemyAircraftLoaded)) + " enemy aircraft were destroyed");
			drawText(-400, -200, 1, 0.33, 1, GLUT_BITMAP_9_BY_15, "82688 - Amier");
		}
		else
		{
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluOrtho2D(-600, 600, -350, 350);
			drawText(-400, 260, 1, 1, 1, GLUT_BITMAP_9_BY_15, "Control : ");
			drawText(-400, 230, 1, 1, 1, GLUT_BITMAP_9_BY_15, "WASD or UP/DOWN/LEFT/ARIGHT arrow - Move");
			drawText(-400, 200, 1, 1, 1, GLUT_BITMAP_9_BY_15, "F - Shoot");
			drawText(-400, 110, 1, 1, 1, GLUT_BITMAP_9_BY_15, "Objective : ");
			drawText(-400, 80, 0, 1, 0, GLUT_BITMAP_9_BY_15, "Find and destroy all aircraft to win");

			drawText(-400, 50, 1, 0, 0, GLUT_BITMAP_9_BY_15, (std::to_string(totalEnemyAircraftLoaded - allEnemyAircraft.size()) + "/" + std::to_string(totalEnemyAircraftLoaded)) + " enemy aircraft were destroyed");

			drawText(-400, -200, 1, 0.33, 1, GLUT_BITMAP_9_BY_15, "82688 - Amier");
		}
	}


	glutPostRedisplay();
	glutSwapBuffers();
	glFlush();


	//std::cout << aircraftX << ", " << aircraftY << std::endl;;
}

/* use ascii */
void keyboardfunc(unsigned char key, int x, int y)
{
	//any key is press

	switch (key)
	{
	case 'W':
	case 'w':
		aircraftY += aircraftSpeed;
		aircraftRotation = 0;
		break;
	case 'S':
	case 's':
		aircraftY -= aircraftSpeed;
		aircraftRotation = 180;
		break;
	case 'A':
	case 'a':
		aircraftX -= aircraftSpeed;
		aircraftRotation = 90;
		break;
	case 'D':
	case 'd':
		aircraftX += aircraftSpeed;
		aircraftRotation = 270;
		break;
	case 'F':
	case 'f':
		allBullets.push_back({ aircraftX, aircraftY, 400, 4 , aircraftRotation });
		break;

	case '[':
		aircraftSpeed -= 2;
		break;
	case ']':
		aircraftSpeed += 2;
		break;
	case 'M':
	case 'm':
		isPaused = !isPaused;
		break;
	}

}

/* only sepcial keyboard */
void specialfunc(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP://up
		aircraftY += 10;
		aircraftRotation = 0;
		break;
	case GLUT_KEY_DOWN://down
		aircraftY -= 10;
		aircraftRotation = 180;
		break;
	case GLUT_KEY_LEFT://left
		aircraftX -= 10;
		aircraftRotation = 90;
		break;
	case GLUT_KEY_RIGHT:;//right
		aircraftX += 10;
		aircraftRotation = 270;
		break;
	}
}

void reshape(int w, int h)
{
	glutReshapeWindow(1200, 700);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluOrtho2D(-600, 600, -350, 350);
	gluOrtho2D(aircraftX - 600, aircraftX + 600, aircraftY - 350, aircraftY + 350);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(1200, 700);
	glutCreateWindow("Fighting Game");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboardfunc);
	glutSpecialFunc(specialfunc);

	glutMainLoop();

	return 0;
}