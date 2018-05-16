
#ifndef SCV_H_
# define SCV_H_

# include <iostream>

# include "sc2api/sc2_api.h"
# include "sc2lib/sc2_lib.h"
# include "sc2api/sc2_interfaces.h"
# include "sc2utils/sc2_manage_process.h"

/*
** This is the Core of IA settings
*/
namespace masterchiefTool
{
    class SCV : public sc2::Agent
    {
    public:
        //Member Vars
        sc2::ActionInterface*               mpc_action;
        const sc2::ObservationInterface*    mpc_observation;
        sc2::QueryInterface*                mpc_query;

        //area where to build, same at start position at the beginning
        sc2::Point3D    mv_startLocation;

        // CTOR & DTOR
        SCV();
        SCV(sc2::ActionInterface* action, const sc2::ObservationInterface* observation, sc2::QueryInterface* query);
        ~SCV();

        //Member Funcs
        bool    TryBuildStructure(sc2::ABILITY_ID structure);
        bool    TryBuildStructure(sc2::ABILITY_ID structure, sc2::Point2D target);
        bool    TryBuildSupplyDepot(void);
        bool    TryBuildBarracks(void);
        bool    TryBuildFactory(void);
        bool    TryBuildEngineeringBay(void);
        bool    TryBuildArmory(void);
        bool    TryBuildVespeneExtractor(void);
        bool    TryExpand(std::vector<sc2::Point3D> vc_expansions);

        const sc2::Unit*    FindNearestMineralPatchInLowestCC(const sc2::Point2D& r_start);
        const sc2::Unit*    FindNearestMineralPatch(const sc2::Point2D& r_start);
        const sc2::Unit*    FindNearestGasPatch(const sc2::Point2D& r_start);
        void                FillRefinery(const sc2::Unit* p_vespene, const bool EOC);
        bool                AddRefinery(const sc2::Unit* p_vespene, const sc2::Unit* p_scv);
        void                Harvest(const sc2::Unit* p_scv);

        void    divide(void);

        void    revengeSCV(const sc2::Point2D& r_target);
    };
}

#endif //!SCV_H_