/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

class FleePoint
{
	public:
        FleePoint(PlayerbotAI* botAI, float x, float y, float z) : botAI(botAI), sumDistance(0.0f), minDistance(0.0f), x(x), y(y), z(z) { }

		float x;
		float y;
		float z;

		float sumDistance;
		float minDistance;

	private:
		PlayerbotAI* botAI;
};

class FleeManager
{
	public:
        FleeManager(Player* bot, float maxAllowedDistance, float followAngle, bool forceMaxDistance = false) : bot(bot),
            maxAllowedDistance(maxAllowedDistance), followAngle(followAngle), forceMaxDistance(forceMaxDistance) { }

		bool CalculateDestination(float* rx, float* ry, float* rz);
		bool isUseful();

	private:
		void calculatePossibleDestinations(std::vector<FleePoint*> &points);
		void calculateDistanceToCreatures(FleePoint *point);
		void cleanup(std::vector<FleePoint*> &points);
		FleePoint* selectOptimalDestination(std::vector<FleePoint*> &points);
		bool isBetterThan(FleePoint* point, FleePoint* other);

		Player* bot;
		float maxAllowedDistance;
		float followAngle;
		bool forceMaxDistance;
};
