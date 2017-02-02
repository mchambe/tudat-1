/*    Copyright (c) 2010-2016, Delft University of Technology
 *    All rigths reserved
 *
 *    This file is part of the Tudat. Redistribution and use in source and
 *    binary forms, with or without modification, are permitted exclusively
 *    under the terms of the Modified BSD license. You should have received
 *    a copy of the license with this file. If not, please or visit:
 *    http://tudat.tudelft.nl/LICENSE.
 */

#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include "Tudat/Basics/testMacros.h"

#include "Tudat/Astrodynamics/BasicAstrodynamics/unitConversions.h"
#include "Tudat/Astrodynamics/GroundStations/groundStationState.h"
#include "Tudat/Astrodynamics/BasicAstrodynamics/oblateSpheroidBodyShapeModel.h"
#include "Tudat/Astrodynamics/BasicAstrodynamics/stateRepresentationConversions.h"
#include "Tudat/Mathematics/BasicMathematics/coordinateConversions.h"
#include "Tudat/External/SpiceInterface/spiceEphemeris.h"
#include "Tudat/External/SpiceInterface/spiceRotationalEphemeris.h"
#include "Tudat/SimulationSetup/EnvironmentSetup/body.h"
#include "Tudat/SimulationSetup/EnvironmentSetup/createGroundStations.h"
#include "Tudat/SimulationSetup/EstimationSetup/createLightTimeCalculator.h"
#include "Tudat/InputOutput/basicInputOutput.h"

namespace tudat
{
namespace unit_tests
{

using namespace tudat::coordinate_conversions;
using namespace tudat::basic_astrodynamics;
using namespace tudat::ground_stations;
using namespace tudat::simulation_setup;
using namespace tudat::unit_conversions;
using namespace tudat::spice_interface;


BOOST_AUTO_TEST_SUITE( test_ground_station_state )

//! Test if ground stations are correctly created.
BOOST_AUTO_TEST_CASE( test_GroundStationState )
{
    // Create Earth object
    boost::shared_ptr< Body > earth = boost::make_shared< Body >( );
    NamedBodyMap bodyMap;
    bodyMap[ "Earth" ] = earth;

    // Central body characteristics (WGS84 Earth ellipsoid).
    const double flattening = 1.0 / 298.257223563;
    const double equatorialRadius = 6378137.0;

    boost::shared_ptr< basic_astrodynamics::OblateSpheroidBodyShapeModel > oblateSpheroidModel =
            boost::make_shared< basic_astrodynamics::OblateSpheroidBodyShapeModel >(
                equatorialRadius, flattening );

    earth->setShapeModel( oblateSpheroidModel );


    // Expected Cartesian state, Montenbruck & Gill (2000) Exercise 5.3.
    const Eigen::Vector3d testCartesianPosition( 1917032.190, 6029782.349, -801376.113 );

    // Expected Cartesian state, Montenbruck & Gill (2000) Exercise 5.3.
    const Eigen::Vector3d testGeodeticPosition( -63.667,
                                                convertDegreesToRadians( -7.26654999 ),
                                                convertDegreesToRadians( 72.36312094 ) );

    // Manually compute associated spherical position
    Eigen::Vector3d testSphericalPosition = coordinate_conversions::convertCartesianToSpherical(
                testCartesianPosition );
    testSphericalPosition( 1 ) = mathematical_constants::PI / 2.0 - testSphericalPosition( 1 );

    // Creatre ground stations: same position, but different representation
    createGroundStation( earth, "Station1", testCartesianPosition, cartesian_position );
    createGroundStation( earth, "Station2", testSphericalPosition, spherical_position );
    createGroundStation( earth, "Station3", testGeodeticPosition, geodetic_position );

    boost::shared_ptr< GroundStationState > station1 = earth->getGroundStation( "Station1" )->getNominalStationState( );
    boost::shared_ptr< GroundStationState > station2 = earth->getGroundStation( "Station2" )->getNominalStationState( );
    boost::shared_ptr< GroundStationState > station3 = earth->getGroundStation( "Station3" )->getNominalStationState( );

    // Check if ground station representations are correctly converted
    Eigen::Vector3d position1, position2;

    {
        position1 = station1->getNominalCartesianPosition( );
        position2 = station2->getNominalCartesianPosition( );

        for( unsigned int i = 0; i < 3; i++ )
        {
            BOOST_CHECK_SMALL( position1( i ) - position2( i ), 1.0E-3 );
        }

        position1 = station1->getNominalCartesianPosition( );
        position2 = station3->getNominalCartesianPosition( );

        for( unsigned int i = 0; i < 3; i++ )
        {
            BOOST_CHECK_SMALL( position1( i ) - position2( i ), 1.0E-3 );
        }

        position1 = station2->getNominalCartesianPosition( );
        position2 = station3->getNominalCartesianPosition( );

        for( unsigned int i = 0; i < 3; i++ )
        {
            BOOST_CHECK_SMALL( position1( i ) - position2( i ), 1.0E-3 );
        }
    }

    {
        position1 = station1->getNominalGeodeticPosition( );
        position2 = station2->getNominalGeodeticPosition( );

        BOOST_CHECK_SMALL( position1( 0 ) - position2( 0 ), 1.0E-3 );
        BOOST_CHECK_SMALL( position1( 1 ) - position2( 1 ), 1.0E-3 / equatorialRadius );
        BOOST_CHECK_SMALL( position1( 2 ) - position2( 2 ), 1.0E-3 / equatorialRadius );

        position1 = station1->getNominalGeodeticPosition( );
        position2 = station3->getNominalGeodeticPosition( );

        BOOST_CHECK_SMALL( position1( 0 ) - position2( 0 ), 1.0E-3 );
        BOOST_CHECK_SMALL( position1( 1 ) - position2( 1 ), 1.0E-3 / equatorialRadius );
        BOOST_CHECK_SMALL( position1( 2 ) - position2( 2 ), 1.0E-3 / equatorialRadius );

        position1 = station2->getNominalGeodeticPosition( );
        position2 = station3->getNominalGeodeticPosition( );

        BOOST_CHECK_SMALL( position1( 0 ) - position2( 0 ), 1.0E-3 );
        BOOST_CHECK_SMALL( position1( 1 ) - position2( 1 ), 1.0E-3 / equatorialRadius );
        BOOST_CHECK_SMALL( position1( 2 ) - position2( 2 ), 1.0E-3 / equatorialRadius );
    }

    {
        position1 = station1->getNominalSphericalPosition( );
        position2 = station2->getNominalSphericalPosition( );

        BOOST_CHECK_SMALL( position1( 0 ) - position2( 0 ), 1.0E-3 );
        BOOST_CHECK_SMALL( position1( 1 ) - position2( 1 ), 1.0E-3 / equatorialRadius );
        BOOST_CHECK_SMALL( position1( 2 ) - position2( 2 ), 1.0E-3 / equatorialRadius );

        position1 = station1->getNominalSphericalPosition( );
        position2 = station3->getNominalSphericalPosition( );

        BOOST_CHECK_SMALL( position1( 0 ) - position2( 0 ), 1.0E-3 );
        BOOST_CHECK_SMALL( position1( 1 ) - position2( 1 ), 1.0E-3 / equatorialRadius );
        BOOST_CHECK_SMALL( position1( 2 ) - position2( 2 ), 1.0E-3 / equatorialRadius );

        position1 = station2->getNominalSphericalPosition( );
        position2 = station3->getNominalSphericalPosition( );

        BOOST_CHECK_SMALL( position1( 0 ) - position2( 0 ), 1.0E-3 );
        BOOST_CHECK_SMALL( position1( 1 ) - position2( 1 ), 1.0E-3 / equatorialRadius );
        BOOST_CHECK_SMALL( position1( 2 ) - position2( 2 ), 1.0E-3 / equatorialRadius );
    }
}

//! Test if global state function for ground station is correctly created.
BOOST_AUTO_TEST_CASE( test_GroundStationGlobalState )
{
    // Load Spice kernels
    loadSpiceKernelInTudat( input_output::getSpiceKernelPath( ) + "pck00009.tpc" );
    loadSpiceKernelInTudat( input_output::getSpiceKernelPath( ) + "de421.bsp" );
    loadSpiceKernelInTudat( input_output::getSpiceKernelPath( ) + "naif0009.tls" );

    // Create Earth object
    boost::shared_ptr< Body > earth = boost::make_shared< Body >( );
    NamedBodyMap bodyMap;
    bodyMap[ "Earth" ] = earth;

    // Central body characteristics (WGS84 Earth ellipsoid).
    const double flattening = 1.0 / 298.257223563;
    const double equatorialRadius = 6378137.0;

    boost::shared_ptr< basic_astrodynamics::OblateSpheroidBodyShapeModel > oblateSpheroidModel =
            boost::make_shared< basic_astrodynamics::OblateSpheroidBodyShapeModel >(
                equatorialRadius, flattening );

    earth->setShapeModel( oblateSpheroidModel );
    earth->setEphemeris( boost::make_shared< ephemerides::SpiceEphemeris >(
                             "Earth", "SSB", false, true, true, "ECLIPJ2000" ) );
    earth->setRotationalEphemeris( boost::make_shared< ephemerides::SpiceRotationalEphemeris >(
                                       "ECLIPJ2000", "IAU_Earth" ) );

    // Define ground station state
    const Eigen::Vector3d groundStationPosition( 1917032.190, 6029782.349, -801376.113 );
    basic_mathematics::Vector6d groundStationState;
    groundStationState << groundStationPosition, 0.0, 0.0, 0.0;

    // Create ground station
    createGroundStation( earth, "Station1", groundStationPosition, cartesian_position );

    // Make state function of ground station w.r.t. SSB in inertial frame
    boost::function< Eigen::Matrix< double, 6, 1 >( const double ) > stateFunction =
            observation_models::getLinkEndCompleteEphemerisFunction(
                std::make_pair( "Earth", "Station1" ), bodyMap );

    // Compare state function with manual computation.
    basic_mathematics::Vector6d currentGlobalState, currentGlobalStateFromFunction;
    for( double testTime = 1.0E7; testTime < 5.0E7; testTime += 2.5E6 )
    {
        currentGlobalState = earth->getEphemeris( )->getCartesianState( testTime ) +
                ephemerides::transformStateToGlobalFrame( groundStationState, testTime, earth->getRotationalEphemeris( ) );
        currentGlobalStateFromFunction = stateFunction( testTime );
        for( unsigned int i = 0; i < 6; i++ )
        {
            BOOST_CHECK_EQUAL( currentGlobalState( i ), currentGlobalStateFromFunction( i ) );
        }
    }
}

BOOST_AUTO_TEST_SUITE_END( )

}

}