
#ifndef FOOBOT_H_
# define FOOBOT_H_

# include <iostream>
# include <ctime>
# include <chrono>

# include "sc2api/sc2_api.h"
# include "sc2lib/sc2_lib.h"
# include "sc2api/sc2_interfaces.h"
# include "sc2utils/sc2_manage_process.h"
# include "sc2api/sc2_map_info.h"
# include "SCV.h"

/*
** This is the Core of IA settings
*/
namespace masterchief
{
    using namespace masterchiefTool;

    class FooBot : public sc2::Agent
    {
    private:
        SCV         mc_scv;

        int mv_nb_ccs;
        int mv_current_nb_ccs;

        int mv_nb_barracks;
        int mv_current_nb_barracks;

        int mv_nb_supplies;
        int mv_current_nb_supplies;

        int mv_nb_refinery;
        int mv_current_nb_refinery;

        int mv_nb_armory;
        int mv_current_nb_armory;

        int mv_nb_engine;
        int mv_current_nb_engine;

        int mv_nb_factory;
        int mv_current_nb_factory;

        sc2::ActionInterface*               mpc_action;
        const sc2::ObservationInterface*    mpc_observation;
        sc2::ObserverActionInterface*       mpc_action_observation;
        sc2::QueryInterface*                mpc_query;
        sc2::GameInfo                       mc_gameinfo;
        std::vector<sc2::Point3D>           mpc_expansions;

        sc2::Point3D                        mv_startLocation;

        int     mv_timer_rebuild;

    public:
        // Member VAR
        uint32_t    mv_restarts;

        // CTOR & DTOR
        FooBot();
        ~FooBot();

        // Member Funcs
        virtual void OnGameStart() final;
        virtual void OnUnitIdle(const sc2::Unit* p_unit) final;
        virtual void OnUnitCreated(const sc2::Unit* p_unit) final;
        virtual void OnUnitDestroyed(const sc2::Unit* p_unit) final;
        virtual void OnBuildingConstructionComplete(const sc2::Unit* p_unit) final;
        virtual void OnStep() final;
        virtual void OnGameEnd() final;
    };
}

#endif //!FOOBOT_H_