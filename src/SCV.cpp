
#include "SCV.h"

using namespace masterchiefTool;

SCV::SCV(sc2::ActionInterface* action, const sc2::ObservationInterface* observation, sc2::QueryInterface* query)
	: mpc_action(action), mpc_observation(observation), mpc_query(query)
{

}

SCV::SCV() {}

SCV::~SCV() {}

/*
** Try to build a CommandCenter to the nearest pos
*/
bool    SCV::TryExpand(std::vector<sc2::Point3D> vc_expansions)
{
	float			minimum_distance = std::numeric_limits<float>::max();
	sc2::Point3D	closest_expansion(0, 0, 0);

	std::cout << "------------" << std::endl;
	std::cout << "TRY EXPAND" << std::endl;
	std::cout << "------------" << std::endl;
	for (const sc2::Point3D& expansion : vc_expansions)
	{
		float current_distance = Distance2D(mv_startLocation, expansion);
		if (current_distance < .01f)
		{
			continue;
		}
		if (current_distance < minimum_distance)
		{
			//check if we can place a cc
			if (mpc_query->Placement(sc2::ABILITY_ID::BUILD_COMMANDCENTER, expansion))
			{
				closest_expansion = expansion;
				minimum_distance = current_distance;
			}
		}
	}
	if (closest_expansion == sc2::Point3D(0, 0, 0))
	{
		std::cout << "FAIL" << std::endl;
		return false;
	}
	if (this->TryBuildStructure(sc2::ABILITY_ID::BUILD_COMMANDCENTER, sc2::Point2D(closest_expansion)) == true)
	{
		std::cout << "SUCCESS" << std::endl;
		return true;
	}
	std::cout << "FAIL" << std::endl;
	return false;
}

int	rand_a_b(int a, int b)
{
	return(rand() % (b - a) + a);
}

/*
** Try to build a structure at a random pos
*/
bool    SCV::TryBuildStructure(sc2::ABILITY_ID structure)
{
	const sc2::Unit*	p_unit_to_build = nullptr;
	sc2::Units			p_units = mpc_observation->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::TERRAN_SCV));
	sc2::Units			p_ccs = mpc_observation->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER));

	if (p_units.empty() || p_ccs.empty())
		return false;

	for (const auto& rp_unit : p_units)
	{
		if (rp_unit != nullptr
			&& rp_unit->orders.size() > 0
			&& rp_unit->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_GATHER
			&& mpc_observation->GetUnit(rp_unit->orders[0].target_unit_tag)->unit_type == sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD)
		{
			p_unit_to_build = rp_unit;
			break;
		}
	}

	if (p_unit_to_build == nullptr)
		return false;

	float rx = sc2::GetRandomScalar();
	float ry = sc2::GetRandomScalar();
	sc2::Point2D	build_location(mv_startLocation);
	sc2::Point2D	target(build_location.x + rx * 15.0f, build_location.y + ry * 15.0f);

	while (mpc_query->Placement(structure, target) == false)
	{
		std::cout << "Try again" << std::endl;
		rx = sc2::GetRandomScalar();
		ry = sc2::GetRandomScalar();
		//build_location = p_ccs[rand_a_b(0, p_ccs.size())]->pos;
		target = sc2::Point2D(build_location.x + rx * 15.0f, build_location.y + ry * 15.0f);
	}
	mpc_action->UnitCommand(p_unit_to_build,
		structure,
		target);

	return true;
}

/*
** Try to build a structure to the target
*/
bool    SCV::TryBuildStructure(sc2::ABILITY_ID structure, sc2::Point2D target)
{
	const sc2::Unit*	p_unit_to_build = nullptr;
	sc2::Units			p_units = mpc_observation->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::TERRAN_SCV));

	if (p_units.empty())
		return false;

	for (const auto& rp_unit : p_units)
	{
		if (rp_unit != nullptr
			&& rp_unit->orders.size() > 0
			&& rp_unit->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_GATHER
			&& mpc_observation->GetUnit(rp_unit->orders[0].target_unit_tag)->unit_type == sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD)
		{
			p_unit_to_build = rp_unit;
			break;
		}
	}
	if (p_unit_to_build == nullptr)
		return false;

	mpc_action->UnitCommand(p_unit_to_build,
		structure,
		target);

	return true;
}

bool    SCV::TryBuildSupplyDepot(void)
{
	std::cout << "------------" << std::endl;
	std::cout << "TRY SUPPLY" << std::endl;
	std::cout << "------------" << std::endl;
	// Try to build a supply depot only if the population need it
	if (mpc_observation->GetMinerals() >= 100)
	{
		if (TryBuildStructure(sc2::ABILITY_ID::BUILD_SUPPLYDEPOT) == true)
		{
			std::cout << "SUCCESS" << std::endl;
			return true;
		}
		
	}
	std::cout << "FAIL" << std::endl;
	return false;
}

bool    SCV::TryBuildEngineeringBay(void)
{
	std::cout << "------------" << std::endl;
	std::cout << "TRY ENGINEERINGBAY" << std::endl;
	std::cout << "------------" << std::endl;

	if (mpc_observation->GetMinerals() >= 100)
	{
		if (TryBuildStructure(sc2::ABILITY_ID::BUILD_ENGINEERINGBAY) == true)
		{
			std::cout << "SUCCESS" << std::endl;
			return true;
		}

	}
	std::cout << "FAIL" << std::endl;
	return false;
}

bool    SCV::TryBuildArmory(void)
{
	std::cout << "------------" << std::endl;
	std::cout << "TRY ARMORY" << std::endl;
	std::cout << "------------" << std::endl;

	if (mpc_observation->GetMinerals() >= 150 &&
		mpc_observation->GetVespene() >= 100)
	{
		if (TryBuildStructure(sc2::ABILITY_ID::BUILD_ARMORY) == true)
		{
			std::cout << "SUCCESS" << std::endl;
			return true;
		}

	}
	std::cout << "FAIL" << std::endl;
	return false;
}

bool    SCV::TryBuildBarracks(void)
{
	std::cout << "------------" << std::endl;
	std::cout << "TRY BARRACK" << std::endl;
	std::cout << "------------" << std::endl;
	// Try to build a supply depot only if the population need it
	if (mpc_observation->GetMinerals() >= 150)
	{
		if (TryBuildStructure(sc2::ABILITY_ID::BUILD_BARRACKS) == true)
		{
			std::cout << "SUCCESS" << std::endl;
			return true;
		}

	}
	std::cout << "FAIL" << std::endl;
	return false;
}

bool    SCV::TryBuildFactory(void)
{
	std::cout << "------------" << std::endl;
	std::cout << "TRY FACTORY" << std::endl;
	std::cout << "------------" << std::endl;
	// Try to build a supply depot only if the population need it
	if (mpc_observation->GetMinerals() >= 150)
	{
		if (TryBuildStructure(sc2::ABILITY_ID::BUILD_FACTORY) == true)
		{
			std::cout << "SUCCESS" << std::endl;
			return true;
		}

	}
	std::cout << "FAIL" << std::endl;
	return false;
}

bool    SCV::TryBuildVespeneExtractor(void)
{
	const sc2::Unit*	p_unit_to_build = nullptr;
	const sc2::Unit*	p_vespene = nullptr;
	sc2::Units			p_units = mpc_observation->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::TERRAN_SCV));

	std::cout << "------------" << std::endl;
	std::cout << "TRY REFINERY" << std::endl;
	std::cout << "------------" << std::endl;
	if (p_units.empty() || mpc_observation->GetMinerals() < 75)
	{
		std::cout << "FAIL" << std::endl;
		return false;
	}

	for (const auto& rp_unit : p_units)
	{
		if (rp_unit != nullptr
			&& rp_unit->orders.size() > 0
			&& rp_unit->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_GATHER
			&& mpc_observation->GetUnit(rp_unit->orders[0].target_unit_tag)->unit_type == sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD)
		{
			p_unit_to_build = rp_unit;
		}
	}
	if ((p_unit_to_build == nullptr)
		|| (p_vespene = this->FindNearestGasPatch(p_unit_to_build->pos)) == nullptr)
	{
		std::cout << "FAIL" << std::endl;
		return false;
	}

	mpc_action->UnitCommand(p_unit_to_build, sc2::ABILITY_ID::BUILD_REFINERY, p_vespene);

	std::cout << "SUCCESS" << std::endl;
	return true;
}

/*
** Fill the Refinery with SCVs mining minerals
** We check with an "End of Construction" flag if the refinery being filled has just finish to being construct,
** because the SCV who finished the construction is going to work on but he's not already set as a "refinery worker"
*/
void	SCV::FillRefinery(const sc2::Unit* p_vespene, const bool EOC = false)
{
	sc2::Units		p_units = mpc_observation->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::TERRAN_SCV));

	int	scv_count = 0;
	int	max_scv = p_vespene->ideal_harvesters;

	if (p_units.empty())
		return;

	if (EOC == true)
		--max_scv;

	for (const auto& rp_unit : p_units)
	{
		if (rp_unit != nullptr
			&& rp_unit->orders.size() > 0
			&& rp_unit->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_GATHER
			&& mpc_observation->GetUnit(rp_unit->orders[0].target_unit_tag)->unit_type == sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD)
		{
			if (scv_count < max_scv)
			{
				++scv_count;
				mpc_action->UnitCommand(rp_unit, sc2::ABILITY_ID::HARVEST_GATHER, p_vespene);
			}
		}
	}
}

//Add a define SCV to a define refinery if possible
bool	SCV::AddRefinery(const sc2::Unit* p_vespene, const sc2::Unit* p_scv)
{
	int		scv_count = p_vespene->assigned_harvesters;
	int		max_scv = p_vespene->ideal_harvesters;

	if  (scv_count >= max_scv)
		return false;

	mpc_action->UnitCommand(p_scv, sc2::ABILITY_ID::HARVEST_GATHER, p_vespene);
	return true;
}

/*
** Find the nearest Mineral Patch in our Vision (or explored Vision but not War Fog) and near a CC
** We prefer the CC with the lowest count of harvesters
*/
const sc2::Unit* SCV::FindNearestMineralPatchInLowestCC(const sc2::Point2D& r_start)
{
	sc2::Units			p_ccs = mpc_observation->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER));
	sc2::Units			p_minerals = mpc_observation->GetUnits(sc2::Unit::Alliance::Neutral, sc2::IsUnit(sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD));
	float				distance = std::numeric_limits<float>::max();
	int					nb_harvesters = 1000;
	
	const sc2::Unit*		p_target = nullptr;
	static const sc2::Unit*	p_cc;
	
	if (p_ccs.empty())
		return nullptr;

	for (const auto& rp_cc : p_ccs)
	{
		if (rp_cc != nullptr
			&& rp_cc->assigned_harvesters < (rp_cc->ideal_harvesters - 2))
		{
			p_cc = rp_cc;
			nb_harvesters = rp_cc->assigned_harvesters;
		}
	}

	for (const auto& rp_mineral : p_minerals)
	{
		float d = DistanceSquared2D(rp_mineral->pos, p_cc->pos);
		if (d < distance)
		{
			distance = d;
			p_target = rp_mineral;
		}
	}
	return p_target;
}

const sc2::Unit* SCV::FindNearestMineralPatch(const sc2::Point2D& r_start)
{
	sc2::Units			p_ccs = mpc_observation->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER));
	sc2::Units			p_minerals = mpc_observation->GetUnits(sc2::Unit::Alliance::Neutral, sc2::IsUnit(sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD));
	float				distance = std::numeric_limits<float>::max();
	int					nb_harvesters = 1000;

	const sc2::Unit*		p_target = nullptr;
	static const sc2::Unit*	p_cc;

	if (p_ccs.empty())
		return nullptr;

	for (const auto& rp_mineral : p_minerals)
	{
		float d = DistanceSquared2D(rp_mineral->pos, r_start);
		if (d < distance)
		{
			distance = d;
			p_target = rp_mineral;
		}
	}
	return p_target;
}


/*
** Find the nearest gas Patch in our Vision (or explored Vision but not War Fog)
*/
const sc2::Unit* SCV::FindNearestGasPatch(const sc2::Point2D& r_start)
{
	sc2::Units			p_ccs = mpc_observation->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER));
	sc2::Units			p_vespenes = mpc_observation->GetUnits(sc2::Unit::Alliance::Neutral, sc2::IsUnit(sc2::UNIT_TYPEID::NEUTRAL_VESPENEGEYSER));
	sc2::Units			p_refineries = mpc_observation->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::TERRAN_REFINERY));

	float				distance = std::numeric_limits<float>::max();
	const sc2::Unit*	p_target = nullptr;

	bool	isAvailable;

	if (p_ccs.empty())
		return nullptr;
	
	for (const auto& rp_cc : p_ccs)
	{
		for (const auto& rp_vespene : p_vespenes)
		{
			isAvailable = true;
			for (const auto& rp_refinery : p_refineries)
			{
				if (sc2::Point2D(rp_refinery->pos) == sc2::Point2D(rp_vespene->pos))
					isAvailable = false;
			}
			if (isAvailable == true)
			{
				float d = DistanceSquared2D(rp_vespene->pos, rp_cc->pos);
				if (d < distance)
				{
					distance = d;
					p_target = rp_vespene;
				}
			}
		}
	}
	return p_target;
}

void	SCV::Harvest(const sc2::Unit* p_scv)
{
	sc2::Units	p_refineries = mpc_observation->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::TERRAN_REFINERY));

	//First, check if there is a fillable refinery
	if (!p_refineries.empty())
		for (const auto& rp_refinery : p_refineries)
			if (this->AddRefinery(rp_refinery, p_scv) == true)
				return;

	//else just mine some minerals
	const sc2::Unit* mineral_target = this->FindNearestMineralPatch(p_scv->pos);

	if (!mineral_target)
		return;

	mpc_action->UnitCommand(p_scv, sc2::ABILITY_ID::SMART, mineral_target);
}

/* 
** Cut the SCVs and reassign them (3 per vespene, the other ones on minerals),
** this is mostly made to respond as a new Command Center build.
*/
void    SCV::divide(void)
{
	sc2::Units	p_ccs = mpc_observation->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER));
	sc2::Units	p_scvs = mpc_observation->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::TERRAN_SCV));
	sc2::Units	p_minerals = mpc_observation->GetUnits(sc2::Unit::Alliance::Neutral, sc2::IsUnit(sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD));

	float				distance;
	const sc2::Unit*	p_target;

	if (p_ccs.empty() || p_scvs.empty() || p_minerals.empty())
		return;

	std::size_t	max((float)(1.f / (float)p_ccs.size() * (float)p_scvs.size()));
	std::size_t	i(0);

	for (const sc2::Unit*& rp_cc : p_ccs)
	{
		p_target = nullptr;
		distance = std::numeric_limits<float>::max();
		for (const sc2::Unit*& rp_mineral : p_minerals)
		{
			float d = DistanceSquared2D(rp_mineral->pos, rp_cc->pos);
			if (d < distance)
			{
				distance = d;
				p_target = rp_mineral;
			}
		}
		while (i < p_scvs.size() && i < max)
		{
			if (p_scvs[i]->orders.size() > 0
				&& p_scvs[i]->orders[0].ability_id == sc2::ABILITY_ID::HARVEST_GATHER
				&& mpc_observation->GetUnit(p_scvs[i]->orders[0].target_unit_tag)->unit_type == sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD)
			{
				mpc_action->UnitCommand(p_scvs[i], sc2::ABILITY_ID::SMART, p_target->pos);
			}
			++i;
		}
		max += (float)(1.f / (float)p_ccs.size() * (float)p_scvs.size());
	}

	mpc_action->SendChat("SCVs: one two three four, one two three four...");
}

/*
** All SCVs are attacking the last "SCV destroyed" pos
*/
void	SCV::revengeSCV(const sc2::Point2D& r_target)
{
	sc2::Units	p_scvs = mpc_observation->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::TERRAN_SCV));
	sc2::Units	p_marines = mpc_observation->GetUnits(sc2::Unit::Alliance::Self, sc2::IsUnit(sc2::UNIT_TYPEID::TERRAN_MARINE));
	bool		toFar = true;

	for (const auto& rp_marine : p_marines)
	{
		//If they are already attacking, break
		if (rp_marine->orders.size() == 0)
			mpc_action->UnitCommand(rp_marine, sc2::ABILITY_ID::ATTACK, r_target);
	}

	for (const auto& rp_scv : p_scvs)
	{
		//If they are already attacking, return
		if (rp_scv->orders.size() > 0
			&& rp_scv->orders[0].ability_id == sc2::ABILITY_ID::ATTACK)
			return;

		//Check the distance to not attack to far away if a SCV die
		if (DistanceSquared2D(r_target, rp_scv->pos) <= 200.f)
		{
			mpc_action->UnitCommand(rp_scv, sc2::ABILITY_ID::ATTACK, r_target);
			toFar = false;
		}
	}
	//Message only if the dead SCV can be revenge (depending of the distance from mining SCVs)
	if (toFar == false)
		mpc_action->SendChat("SCVs: Exterminate! Exterminate!");
}