
#include "FooBot.h"

FooBot::FooBot() : mv_restarts(0)
{
	mpc_action = Actions();
	mpc_observation = Observation();
	mpc_query = Query();

	mc_scv = SCV(mpc_action, mpc_observation, mpc_query);
}

FooBot::~FooBot() {}

void FooBot::OnGameStart()
{
	mpc_expansions = sc2::search::CalculateExpansionLocations(mpc_observation, mpc_query);
	mv_startLocation = mpc_observation->GetStartLocation();
	mc_scv.mv_startLocation = mv_startLocation;

	mc_gameinfo = Observation()->GetGameInfo();

	mv_nb_ccs = 1;
	mv_current_nb_ccs = 1;

	mv_nb_barracks = 0;
	mv_current_nb_barracks = 0;

	mv_nb_supplies = 0;
	mv_current_nb_supplies = 0;

	mv_nb_refinery = 0;
	mv_current_nb_refinery = 0;

	mv_timer_rebuild = 0;

	Debug()->DebugTextOut("MasterChief");
	Debug()->SendDebug();
	std::cout << "Starting a new game (" << mv_restarts << " restarts)" << std::endl;

	mpc_action->SendChat("CommandCenter: Strength and Honor!");
}

void FooBot::OnGameEnd()
{
	++mv_restarts;
	std::cout << "Game ended after: " << mpc_observation->GetGameLoop() << " loops " << std::endl;
}

/*
** Handle units or buildings which are waiting for orders
*/
void	FooBot::OnUnitIdle(const sc2::Unit* p_unit)
{
	switch (p_unit->unit_type.ToType())
	{
		case sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER:
		{
			if (p_unit->assigned_harvesters < (p_unit->ideal_harvesters - 2))
				mpc_action->UnitCommand(p_unit, sc2::ABILITY_ID::TRAIN_SCV);
			break;
		}
		case sc2::UNIT_TYPEID::TERRAN_SCV:
		{
			mc_scv.Harvest(p_unit);
			break;
		}
		case sc2::UNIT_TYPEID::TERRAN_BARRACKS:
		{
			mpc_action->UnitCommand(p_unit, sc2::ABILITY_ID::TRAIN_MARINE);
			mpc_action->UnitCommand(p_unit, sc2::ABILITY_ID::RALLY_BUILDING, mv_startLocation);
		}
		default: break;
	}
}

void	FooBot::OnBuildingConstructionComplete(const sc2::Unit* p_unit)
{
	switch (p_unit->unit_type.ToType())
	{
		case sc2::UNIT_TYPEID::TERRAN_REFINERY:
		{
			++mv_nb_refinery;
			mc_scv.FillRefinery(p_unit, true);
			break;
		}
		case sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER:
		{
			mpc_action->SendChat("CommandCenter: Split Up!");
			mc_scv.divide();
			++mv_nb_ccs;
			break;
		}
		case sc2::UNIT_TYPEID::TERRAN_BARRACKS:
		{
			++mv_nb_barracks;
			break;
		}
		case sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT:
		{
			++mv_nb_supplies;
			break;
		}
		default: break;
	}
}

void	FooBot::OnUnitCreated(const sc2::Unit* p_unit)
{

}

void	FooBot::OnUnitDestroyed(const sc2::Unit* p_unit)
{
	switch (p_unit->unit_type.ToType())
	{
		case sc2::UNIT_TYPEID::TERRAN_SCV:
		{
			mc_scv.revengeSCV(p_unit->pos);
			break;
		}
		case sc2::UNIT_TYPEID::TERRAN_BARRACKS:
		{
			if (p_unit->build_progress == 1.f)
			{
				--mv_nb_barracks;
			}
			--mv_current_nb_barracks;

			if (mv_current_nb_barracks < 0
				|| mv_nb_barracks < 0)
			{
				mv_nb_barracks = 0;
				mv_current_nb_barracks = 0;
			}

			mv_timer_rebuild = 20;
			break;
		}
		case sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER:
		{
			if (p_unit->build_progress == 1.f)
			{
				--mv_nb_ccs;
			}
			--mv_current_nb_ccs;

			if (mv_current_nb_ccs < 0
				|| mv_nb_ccs < 0)
			{
				mv_nb_ccs = 0;
				mv_current_nb_ccs = 0;
			}

			mv_timer_rebuild = 20;
			break;
		}
		case sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT:
		{
			if (p_unit->build_progress == 1.f)
			{
				--mv_nb_supplies;
			}
			--mv_current_nb_supplies;

			if (mv_current_nb_supplies < 0
				|| mv_nb_supplies < 0)
			{
				mv_nb_supplies = 0;
				mv_current_nb_supplies = 0;
			}

			mv_timer_rebuild = 20;
			break;
		}
		case sc2::UNIT_TYPEID::TERRAN_REFINERY:
		{
			if (p_unit->build_progress == 1.f)
			{
				--mv_nb_refinery;
			}
			--mv_current_nb_refinery;

			if (mv_current_nb_refinery < 0
				|| mv_nb_refinery < 0)
			{
				mv_nb_refinery = 0;
				mv_current_nb_refinery = 0;
			}

			mv_timer_rebuild = 20;
			break;
		}
		default: break;
	}
}

/*
** Game Loop
*/
void	FooBot::OnStep()
{
	/*
	** Actualize each 0.5sec
	*/
	static auto t_start = std::chrono::high_resolution_clock::now();
	
	auto t_end = std::chrono::high_resolution_clock::now();

	if (std::chrono::duration<double, std::milli>(t_end - t_start).count() < 500)
		return;
	else
		t_start = std::chrono::high_resolution_clock::now();
	/*
	** !Actualize each 0.5sec
	*/

	if (mv_timer_rebuild > 0)
	{
		--mv_timer_rebuild;
	}

	/*
	** Try to build new scvs when the cc is idle
	** this is done because we only can get the CC at the frame when its idle but not after
	** so if a SCV die or if we split SCVs between our ccs, we cant continue building SCVs from "On Idle" function
	** This part just load one SCV, the rest of the job is done from "On Idle" function
	*/
	sc2::Units	p_ccs = mpc_observation->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER));

	for (const auto& rp_cc : p_ccs)
	{
		if (rp_cc->orders.empty()
			&& rp_cc->assigned_harvesters < (rp_cc->ideal_harvesters - 2))
			mpc_action->UnitCommand(rp_cc, sc2::ABILITY_ID::TRAIN_SCV);
	}
	/*
	** !Try to build new scvs when the cc is idle
	*/


	int			nb_supplies = mpc_observation->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::TERRAN_SUPPLYDEPOT)).size();

	//nb_ccs is all css, built at any % and mv_nb_ccs are only the ccs built at 100%
	int			nb_ccs = p_ccs.size();

	int			current_food = mpc_observation->GetFoodCap();
	int			next_food = nb_supplies * 8 + mv_nb_ccs * 15;

	std::cout << "------------------" << std::endl;
	std::cout << "food used: " << mpc_observation->GetFoodUsed() << std::endl;
	std::cout << "next food: " << next_food << std::endl;
	std::cout << "timer: " << mv_timer_rebuild << std::endl;
	std::cout << "nb ccs: " << mv_nb_ccs << std::endl;
	std::cout << "nb refineries: " << mv_nb_refinery << std::endl;
	std::cout << "nb supplies: " << mv_nb_supplies << std::endl;
	std::cout << "nb barracks: " << mv_nb_barracks << std::endl;
	std::cout << "------------------" << std::endl;

	if (mv_timer_rebuild == 0)
	{
		//BUILD SUPPLY
		if ((mpc_observation->GetMinerals() >= 100)
			&& (mpc_observation->GetFoodUsed() >= (next_food - 2))
			&& (mv_current_nb_supplies == mv_nb_supplies))
		{
			if (mc_scv.TryBuildSupplyDepot() == true)
				++mv_current_nb_supplies;
		}

		//BUILD REFINERY
		else if ((mpc_observation->GetMinerals() >= 75)
			&& (mv_current_nb_refinery < (mv_nb_ccs * 2))
			&& (mv_current_nb_refinery == mv_nb_refinery))
		{
			if (mc_scv.TryBuildVespeneExtractor() == true)
				++mv_current_nb_refinery;
		}

		//BUILD BARRACKS
		else if ((mpc_observation->GetMinerals() >= 150)
			&& (mv_current_nb_barracks < (mv_nb_ccs * 2))
			&& (mv_current_nb_barracks == mv_nb_barracks))
		{
			if (mc_scv.TryBuildBarracks() == true)
				++mv_current_nb_barracks;
		}

		//BUILD CC
		else if ((mpc_observation->GetMinerals() >= 1000)
			&& (mv_current_nb_ccs < 4)
			&& (mv_current_nb_ccs == mv_nb_ccs))
		{
			if (mc_scv.TryExpand(mpc_expansions) == true)
				++mv_current_nb_ccs;
		}

		//ATTACK
		sc2::Units	p_marines = mpc_observation->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::TERRAN_MARINE));
		sc2::Units	p_enemies = mpc_observation->GetUnits(sc2::Unit::Alliance::Enemy);

		if (p_marines.empty() || p_marines.size() < 40)
			return;

		if (p_enemies.empty())
		{
			if (!mc_gameinfo.enemy_start_locations.empty())
			{
				for (const auto& rp_marine : p_marines)
				{
					//If they are already attacking, break
					if (rp_marine != nullptr
						&& rp_marine->orders.size() > 0
						&& rp_marine->orders[0].ability_id == sc2::ABILITY_ID::ATTACK)
						break;
					mpc_action->UnitCommand(rp_marine, sc2::ABILITY_ID::ATTACK, mc_gameinfo.enemy_start_locations.front());
				}
			}
		}
		else
		{
			for (const auto& rp_marine : p_marines)
			{
				//If they are already attacking, break
				if (rp_marine != nullptr
					&& rp_marine->orders.size() > 0
					&& rp_marine->orders[0].ability_id == sc2::ABILITY_ID::ATTACK)
					break;
				mpc_action->UnitCommand(rp_marine, sc2::ABILITY_ID::ATTACK, p_enemies.front()->pos);
			}
		}
	}
}
