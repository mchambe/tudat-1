/*    Copyright (c) 2010-2018, Delft University of Technology
 *    All rigths reserved
 *
 *    This file is part of the Tudat. Redistribution and use in source and
 *    binary forms, with or without modification, are permitted exclusively
 *    under the terms of the Modified BSD license. You should have received
 *    a copy of the license with this file. If not, please or visit:
 *    http://tudat.tudelft.nl/LICENSE.
 */

#ifndef SIMS_FLANAGAN_OPTIMISATION_SETUP_H
#define SIMS_FLANAGAN_OPTIMISATION_SETUP_H

#include <vector>
#include <utility>
#include <limits>

#include <Eigen/Core>

#include "pagmo/island.hpp"
#include "pagmo/io.hpp"
#include "pagmo/serialization.hpp"
#include "pagmo/problem.hpp"

#include "Tudat/SimulationSetup/tudatSimulationHeader.h"


/*!
 *  The class defined in this file is to be used in a Pagmo optimization. It defines the objective function for a Sims-Flanagan direct method.
 *  The independent variables are:
 *
 *  1) Throttles for each segment
 *  2) ?
 *
 *  The problem minimized the Delta V (ADD CHOICE?)
 */

using namespace pagmo;
using namespace tudat;

struct SimsFlanaganProblem
{

    typedef Eigen::Matrix< double, 6, 1 > StateType;

    //! Default constructor, required for Pagmo compatibility
    SimsFlanaganProblem( ):  propagatorType_( propagators::cowell ), useHighOrderSolution_( false ){ }

    //! Constructor.
    SimsFlanaganProblem( const Eigen::Vector6d& stateAtDeparture,
                         const Eigen::Vector6d& stateAtArrival,
                         const double maximumThrust,
                         const std::function< double ( const double ) > specificImpulseFunction,
                         const int numberSegments,
                         const double timeOfFlight,
                         simulation_setup::NamedBodyMap bodyMap,
                         const std::string bodyToPropagate,
                         const std::string centralBody,
                         std::shared_ptr< numerical_integrators::IntegratorSettings< double > > integratorSettings,
                         const propagators::TranslationalPropagatorType propagatorType = propagators::cowell,
                         const bool useHighOrderSolution = false );

    //! Calculate the fitness as a function of the parameter vector x
    std::vector< double > fitness( const std::vector< double > &x ) const;

    //! Retrieve the allowable limits of the parameter vector x: pair containing minima and maxima of parameter values
    std::pair< std::vector< double >, std::vector< double > > get_bounds() const;

    //! Retrieve the name of the problem
    std::string get_name( ) const;

    //! Retrieve the number of objectives in problem, e.g. the size of the vector returned by the fitness function
    vector_double::size_type get_nobj() const
    {
        return 1u;
    }

    vector_double::size_type get_nic() const
    {
        return numberSegments_;
    }

    vector_double::size_type get_nec() const
    {
        return 6u;
    }

//    //! Serialization function for Pagmo compatibility
//    template <typename Archive>
//    void serialize(Archive &ar)
//    {
//        ar(problemBounds_);
//    }

protected:


private:

    //! State vector of the vehicle at the leg departure.
    Eigen::Vector6d stateAtDeparture_;

    //! State vector of the vehicle at the leg arrival.
    Eigen::Vector6d stateAtArrival_;

    //! Maximum allowed thrust.
    double maximumThrust_;

    //! Specific impulse function.
    std::function< double ( const double ) > specificImpulseFunction_;

    //! Number of segments into which the leg is subdivided.
    int numberSegments_;

    //! Time of flight for the leg.
    double timeOfFlight_;

    //! Body map.
    mutable simulation_setup::NamedBodyMap bodyMap_;

    //! Name of the body to be propagated.
    const std::string bodyToPropagate_;

    //! Name of the central body.
    const std::string centralBody_;

    //! Integrator settings (for high order solution).
    mutable std::shared_ptr< numerical_integrators::IntegratorSettings< double > > integratorSettings_;

    //! Initial spacecraft mass.
    double initialSpacecraftMass_;

    //! Propagator type.
    const propagators::TranslationalPropagatorType propagatorType_;

    //! Boolean defining which of the low or high order solutions is used.
    const bool useHighOrderSolution_;

//    //! Propagator settings (for high order solution).
//    mutable std::shared_ptr< propagators::TranslationalStatePropagatorSettings< double > > propagatorSettings_;


};

#endif // SIMS_FLANAGAN_OPTIMISATION_SETUP_H
