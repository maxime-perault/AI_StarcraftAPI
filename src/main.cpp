
#include <iostream>

#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"

#include "FooBot.h"

int main(int argc, char* argv[])
{
	sc2::Coordinator coordinator;

	if (!coordinator.LoadSettings(argc, argv))
	{
		return 1;
	}

	// Add the custom bots, it will control the players.
	FooBot bot;

	coordinator.SetParticipants({
		CreateParticipant(sc2::Race::Terran, &bot),
		CreateComputer(sc2::Race::Terran, sc2::Difficulty::VeryHard)
	});

	// Start the game.
	coordinator.LaunchStarcraft();

	// Step forward the game simulation.
	bool do_break = false;

	while (!do_break)
	{
		coordinator.StartGame(sc2::kMapBelShirVestigeLE);
		while (coordinator.Update() && !do_break)
		{
			if (sc2::PollKeyPress())
			{
				do_break = true;
			}
		}
	}

	return 0;
}
