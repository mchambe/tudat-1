/*    Copyright (c) 2010-2019, Delft University of Technology
 *    All rigths reserved
 *
 *    This file is part of the Tudat. Redistribution and use in source and
 *    binary forms, with or without modification, are permitted exclusively
 *    under the terms of the Modified BSD license. You should have received
 *    a copy of the license with this file. If not, please or visit:
 *    http://tudat.tudelft.nl/LICENSE.
 */

#ifndef TUDAT_CREATEMUTUALAPPROXIMATIONPARTIALS_H
#define TUDAT_CREATEMUTUALAPPROXIMATIONPARTIALS_H

#include <vector>
#include <map>

#include <memory>

#include <Eigen/Core>

#include "Tudat/Mathematics/Interpolators/interpolator.h"

#include "Tudat/SimulationSetup/EstimationSetup/createCartesianStatePartials.h"
#include "Tudat/Astrodynamics/OrbitDetermination/ObservationPartials/mutualApproximationPartial.h"
#include "Tudat/SimulationSetup/EstimationSetup/createLightTimeCorrectionPartials.h"
#include "Tudat/Astrodynamics/OrbitDetermination/EstimatableParameters/initialTranslationalState.h"
#include "Tudat/Astrodynamics/ObservationModels/linkTypeDefs.h"
#include "Tudat/SimulationSetup/EstimationSetup/variationalEquationsSolver.h"
#include "Tudat/Astrodynamics/ObservationModels/observationModel.h"


namespace tudat
{

namespace observation_partials
{

//! Function to generate mutual approximation partial wrt a position of a body.
/*!
 *  Function to generate mutual approximation partial wrt a position of a body, for a single link ends (which must contain a
 *  transmitter and receiever  linkEndType).
 *  \param mutualApproximationLinkEnds Link ends (transmitter, transmitter2 and receiever) for which partials are to be calculated
 *  (i.e. for which mutual approximation observations are to be processed).
 *  \param bodyMap List of all bodies, for creating mutual approximation partial.
 *  \param bodyToEstimate Name of body wrt position of which a partial is to be created.
 *  \param mutualApproximationScaler Object scale position partials to mutual approximation partials for current link ends.
 *  \param lightTimeCorrectionPartialObjects List of light time correction partials to be used (empty by default)
 *  \return Mutual approximation partial object wrt a current position of a body (is nullptr if no parameter dependency exists).
 */
std::shared_ptr< MutualApproximationPartial > createMutualApproximationPartialWrtBodyPosition(
        const observation_models::LinkEnds mutualApproximationLinkEnds,
        const simulation_setup::NamedBodyMap& bodyMap,
        const std::string bodyToEstimate,
        const std::shared_ptr< MutualApproximationScalingBase > mutualApproximationScaler,
        const std::vector< std::vector< std::shared_ptr< observation_partials::LightTimeCorrectionPartial > > >&
        lightTimeCorrectionPartialObjects =
        std::vector< std::vector< std::shared_ptr< observation_partials::LightTimeCorrectionPartial > > >( ) );

//! Function to generate mutual approximation partial wrt a single  parameter.
/*!
 *  Function to generate mutual approximation partial wrt a single  parameter, for a single link ends (which must contain a
 *  transmitter, transmitter2 and receiever linkEndType).
 *  \tparam ParameterType Type of parameter (double for size 1, VectorXd for larger size).
 *  \param mutualApproximationLinkEnds Link ends (transmitter, transmitter2 and receiever) for which mutual approximation partials are to be
 *  calculated (i.e. for which  mutual approximation observations are to be processed).
 *  \param bodyMap List of all bodies, for creating mutual approximation partial.
 *  \param parameterToEstimate Object of current parameter that is to be estimated.
 *  \param mutualApproximationScaler Object scale position partials to mutual approximation partials for current link ends.
 *  \param lightTimeCorrectionPartialObjects List of light time correction partials to be used (empty by default)
 *  \return Mutual approximation partial object wrt a single parameter (is nullptr if no parameter dependency exists).
 */
template< typename ParameterType >
std::shared_ptr< MutualApproximationPartial > createMutualApproximationPartialWrtParameter(
        const observation_models::LinkEnds mutualApproximationLinkEnds,
        const simulation_setup::NamedBodyMap& bodyMap,
        const std::shared_ptr< estimatable_parameters::EstimatableParameter< ParameterType > > parameterToEstimate,
        const std::shared_ptr< MutualApproximationScalingBase > mutualApproximationScaler,
        const std::vector< std::vector< std::shared_ptr< observation_partials::LightTimeCorrectionPartial > > >&
        lightTimeCorrectionPartialObjects =
        std::vector< std::vector< std::shared_ptr< observation_partials::LightTimeCorrectionPartial > > >( ) )
{
    std::shared_ptr< MutualApproximationPartial > mutualApproximationPartial;

    {
        std::map< observation_models::LinkEndType, std::shared_ptr< CartesianStatePartial > > positionPartials =
                createCartesianStatePartialsWrtParameter( mutualApproximationLinkEnds, bodyMap, parameterToEstimate );

        std::shared_ptr< MutualApproximationPartial > testMutualApproximationPartial = std::make_shared< MutualApproximationPartial >(
                    mutualApproximationScaler, positionPartials, parameterToEstimate->getParameterName( ),
                    lightTimeCorrectionPartialObjects );

        // Create mutual approximation partials if any position partials are created (i.e. if any dependency exists).
        if( positionPartials.size( ) > 0 || testMutualApproximationPartial->getNumberOfLighTimeCorrectionPartialsFunctions( ) )
        {
            mutualApproximationPartial = testMutualApproximationPartial;
        }
    }
    return mutualApproximationPartial;
}

//! Function to generate mutual approximation partials and associated scaler for single link end.
/*!
 *  Function to generate mutual approximation partials and associated scaler for all parameters that are to be estimated,
 *  for a single link ends.
 *  The set of parameters and bodies that are to be estimated, as well as the set of link ends
 *  (each of which must contain a transmitter, transmitter2 and receiver linkEndType) that are to be used.
 *  \param mutualApproximationLinkEnds Link ends (transmitter, transmitter2 and receiver) for which mutual approximation partials are to be
 *  calculated (i.e. for which mutual approximation observations are to be processed).
 *  \param bodyMap List of all bodies, for creating mutual approximation partials.
 *  \param parametersToEstimate Set of parameters that are to be estimated (in addition to initial states of
 *  requested bodies)
 *  \param lightTimeCorrections List of light time correction partials to be used (empty by default)
 *  \return Set of observation partials with associated indices in complete vector of parameters that are estimated,
 *  representing all  necessary mutual approximation partials of a single link end, and MutualApproximationScaling, object, used for
 *  scaling the position partial members of all MutualApproximationPartials in link end.
 */
template< typename ParameterType >
std::pair< std::map< std::pair< int, int >, std::shared_ptr< ObservationPartial< 1 > > >,
std::shared_ptr< PositionPartialScaling > >
createMutualApproximationPartials(
        const observation_models::LinkEnds mutualApproximationLinkEnds,
        const simulation_setup::NamedBodyMap& bodyMap,
        const std::shared_ptr< estimatable_parameters::EstimatableParameterSet< ParameterType > > parametersToEstimate,
        const bool isCentralInstantUsedAsObservable,
        const std::vector< std::vector< std::shared_ptr< observation_models::LightTimeCorrection > > >& lightTimeCorrections =
        std::vector< std::vector< std::shared_ptr< observation_models::LightTimeCorrection > > >( ),
        const std::shared_ptr< propagators::DependentVariablesInterface > dependentVariablesInterface
                = std::shared_ptr< propagators::DependentVariablesInterface >( ) )

{
    std::vector< std::vector< std::shared_ptr< observation_partials::LightTimeCorrectionPartial > > > lightTimeCorrectionPartialObjects;
    if( lightTimeCorrections.size( ) > 0 )
    {
        if( lightTimeCorrections.size( ) != 2 )
        {
            throw std::runtime_error( "Error when making mutual approximation partials, light time corrections for "
                                      + std::to_string( lightTimeCorrections.size( ) ) + " links found, instead of 2.");
        }
        lightTimeCorrectionPartialObjects.push_back(
                    observation_partials::createLightTimeCorrectionPartials( lightTimeCorrections[ 0 ] ) );
        lightTimeCorrectionPartialObjects.push_back(
                    observation_partials::createLightTimeCorrectionPartials( lightTimeCorrections[ 1 ] ) );
    }

    // Create scaling object, to be used for all mutual approximation partials in current link end.
    std::shared_ptr< MutualApproximationScalingBase > mutualApproximationScaling;
    if ( isCentralInstantUsedAsObservable )
    {
        mutualApproximationScaling = std::make_shared< MutualApproximationScaling >( dependentVariablesInterface );
    }
    else
    {
        mutualApproximationScaling = std::make_shared< ModifiedMutualApproximationScaling >( dependentVariablesInterface );
    }

    SingleLinkObservationPartialList mutualApproximationPartials;


    // Initialize vector index variables.
    int currentIndex = 0;
    std::pair< int, int > currentPair = std::pair< int, int >( currentIndex, 1 );

    std::vector< std::shared_ptr< estimatable_parameters::EstimatableParameter<
            Eigen::Matrix< ParameterType, Eigen::Dynamic, 1 > > > > initialDynamicalParameters =
            parametersToEstimate->getEstimatedInitialStateParameters( );

    // Iterate over list of bodies of which the partials of the accelerations acting on them are required.
    for( unsigned int i = 0; i < initialDynamicalParameters.size( ); i++ )
    {

        std::string acceleratedBody;
        if( initialDynamicalParameters.at( i )->getParameterName( ).first == estimatable_parameters::initial_body_state )
        {
            acceleratedBody = initialDynamicalParameters.at( i )->getParameterName( ).second.first;
        }
        else if( initialDynamicalParameters.at( i )->getParameterName( ).first == estimatable_parameters::arc_wise_initial_body_state )
        {
            acceleratedBody = initialDynamicalParameters.at( i )->getParameterName( ).second.first;
        }
        else
        {
            throw std::runtime_error( "Error when making mutual approximation partials, could not identify parameter" );
        }

        // Create position mutual approximation partial for current body
        std::shared_ptr< MutualApproximationPartial > currentMutualApproximationPartial = createMutualApproximationPartialWrtBodyPosition(
                    mutualApproximationLinkEnds, bodyMap, acceleratedBody, mutualApproximationScaling,
                    lightTimeCorrectionPartialObjects );

        // Check if partial is non-nullptr (i.e. whether dependency exists between current mutual approximation and current body)
        if( currentMutualApproximationPartial != nullptr )
        {
            // Add partial to the list.
            currentPair = std::pair< int, int >( currentIndex, 6 );
            mutualApproximationPartials[ currentPair ] = currentMutualApproximationPartial;
        }

        // Increment current index by size of body initial state (6).
        currentIndex += 6;
    }

    // Iterate over all double parameters that are to be estimated.
    std::map< int, std::shared_ptr< estimatable_parameters::EstimatableParameter< double > > > doubleParametersToEstimate =
            parametersToEstimate->getDoubleParameters( );
    for( std::map< int, std::shared_ptr< estimatable_parameters::EstimatableParameter< double > > >::iterator
         parameterIterator = doubleParametersToEstimate.begin( );
         parameterIterator != doubleParametersToEstimate.end( ); parameterIterator++ )
    {
        // Create position mutual approximation partial for current parameter
        std::shared_ptr< MutualApproximationPartial > currentMutualApproximationPartial = createMutualApproximationPartialWrtParameter(
                    mutualApproximationLinkEnds, bodyMap, parameterIterator->second, mutualApproximationScaling,
                    lightTimeCorrectionPartialObjects );

        if( currentMutualApproximationPartial != nullptr )
        {
            // Add partial to the list.
            currentPair = std::pair< int, int >( parameterIterator->first, 1 );
            mutualApproximationPartials[ currentPair ] = currentMutualApproximationPartial;
        }
    }

    // Iterate over all vector parameters that are to be estimated.
    std::map< int, std::shared_ptr< estimatable_parameters::EstimatableParameter< Eigen::VectorXd > > >
            vectorParametersToEstimate = parametersToEstimate->getVectorParameters( );
    for( std::map< int, std::shared_ptr< estimatable_parameters::EstimatableParameter< Eigen::VectorXd  > > >::iterator
         parameterIterator = vectorParametersToEstimate.begin( );
         parameterIterator != vectorParametersToEstimate.end( ); parameterIterator++ )
    {

        // Create position mutual approximation partial for current parameter
        std::shared_ptr< ObservationPartial< 1 > > currentMutualApproximationPartial;

        if( !isParameterObservationLinkProperty( parameterIterator->second->getParameterName( ).first )  )
        {
            currentMutualApproximationPartial = createMutualApproximationPartialWrtParameter(
                        mutualApproximationLinkEnds, bodyMap, parameterIterator->second, mutualApproximationScaling,
                        lightTimeCorrectionPartialObjects );
        }
        else
        {
            currentMutualApproximationPartial = createObservationPartialWrtLinkProperty< 1 >(
                        mutualApproximationLinkEnds, observation_models::mutual_approximation, parameterIterator->second );
        }

        // Check if partial is non-nullptr (i.e. whether dependency exists between current observable and current parameter)
        if( currentMutualApproximationPartial != nullptr )
        {
            // Add partial to the list.
            currentPair = std::pair< int, int >( parameterIterator->first,
                                                 parameterIterator->second->getParameterSize( ) );
            mutualApproximationPartials[ currentPair ] = currentMutualApproximationPartial;
        }

    }
    return std::make_pair( mutualApproximationPartials, mutualApproximationScaling );
}

//! Function to generate mutual approximation partials for all parameters that are to be estimated, for all sets of link ends.
/*!
 *  Function to generate mutual approximation partials for all parameters that are to be estimated, for all sets of link ends.
 *  The mutual approximation partials are generated per set of link ends. The set of parameters and bodies that are to be
 *  estimated, as well as the set of link ends (each of which must contain a transmitter, transmitter2 and receiever linkEndType)
 *  that are to be used.
 *  \param linkEnds Vector of all link ends for which mutual approximation distance partials are to be calculated (i.e. for which one-way
 *  mutual approximation observations are  to be processed).
 *  \param bodyMap List of all bodies, for creating mutual approximation partials.
 *  \param parametersToEstimate Set of parameters that are to be estimated (in addition to initial states
 *  of requested bodies)
 *  \param lightTimeCorrections List of light time correction partials to be used (empty by default)
 *  \return Map of SingleLinkObservationPartialList, representing all necessary mutual approximation partials of a single link
 *  end, and MutualApproximationScaling, object, used for scaling the position partial members of all MutualApproximationPartials in
 *  link end.
 */
template< typename ParameterType >
std::map< observation_models::LinkEnds, std::pair< SingleLinkObservationPartialList,
std::shared_ptr< PositionPartialScaling > > >
createMutualApproximationPartials(
        const std::vector< observation_models::LinkEnds > linkEnds,
        const simulation_setup::NamedBodyMap& bodyMap,
        const std::shared_ptr< estimatable_parameters::EstimatableParameterSet< ParameterType > > parametersToEstimate,
        const bool isCentralInstantUsedAsObservable,
        const std::map< observation_models::LinkEnds,
        std::vector< std::vector< std::shared_ptr< observation_models::LightTimeCorrection > > > >& lightTimeCorrections =
        std::map< observation_models::LinkEnds,
        std::vector< std::vector< std::shared_ptr< observation_models::LightTimeCorrection > > > >( ),
        const std::shared_ptr< propagators::DependentVariablesInterface > dependentVariablesInterface
                = std::shared_ptr< propagators::DependentVariablesInterface >( ) )
{
    // Declare return list.
    std::map< observation_models::LinkEnds, std::pair< SingleLinkObservationPartialList,
            std::shared_ptr< PositionPartialScaling > > > mutualApproximationPartials;

    // Iterate over all link ends.
    for( unsigned int i = 0; i < linkEnds.size( ); i++ )
    {
        // Check if required link end types are present
        if( ( linkEnds[ i ].count( observation_models::receiver ) == 0 ) ||
                ( linkEnds[ i ].count( observation_models::transmitter ) == 0 ) ||
                ( linkEnds[ i ].count( observation_models::transmitter2 ) == 0 ) )
        {
            throw std::runtime_error( "Error when making mutual approximation partials, did not find both transmitter, transmitter2 and receiver in link ends" );

        }

        std::vector< std::vector< std::shared_ptr< observation_models::LightTimeCorrection > > > currentLightTimeCorrections;
        if( lightTimeCorrections.count( linkEnds.at( i ) ) > 0 )
        {
            if( lightTimeCorrections.at( linkEnds.at( i ) ).size( ) != 2 )
            {
                std::cerr << "Error when making mutual approximation partials, light time corrections for " <<
                           lightTimeCorrections.at( linkEnds.at( i ) ).size( ) << " links found, instead of 2." << std::endl;
            }
            currentLightTimeCorrections = lightTimeCorrections.at( linkEnds.at( i ) );
        }
        else
        {
            currentLightTimeCorrections.clear( );
        }

        // Create mutual approximation partials for current link ends
        mutualApproximationPartials[ linkEnds[ i ] ] = createMutualApproximationPartials(
                    linkEnds[ i ], bodyMap, parametersToEstimate, isCentralInstantUsedAsObservable, currentLightTimeCorrections, dependentVariablesInterface );
    }
    return mutualApproximationPartials;
}



//! Function to generate mutual approximation with impact parameter partial wrt a position of a body.
/*!
 *  Function to generate mutual approximation with impact parameter partial wrt a position of a body, for a single link ends (which must contain a
 *  transmitter and receiever  linkEndType).
 *  \param mutualApproximationLinkEnds Link ends (transmitter, transmitter2 and receiever) for which partials are to be calculated
 *  (i.e. for which mutual approximation observations are to be processed).
 *  \param bodyMap List of all bodies, for creating mutual approximation partial.
 *  \param bodyToEstimate Name of body wrt position of which a partial is to be created.
 *  \param mutualApproximationScaler Object scale position partials to mutual approximation with impact parameter partials for current link ends.
 *  \param lightTimeCorrectionPartialObjects List of light time correction partials to be used (empty by default)
 *  \return Mutual approximation with impact parameter partial object wrt a current position of a body (is nullptr if no parameter dependency exists).
 */
std::shared_ptr< MutualApproximationWithImpactParameterPartial > createMutualApproximationWithImpactParameterPartialWrtBodyPosition(
        const observation_models::LinkEnds mutualApproximationLinkEnds,
        const simulation_setup::NamedBodyMap& bodyMap,
        const std::string bodyToEstimate,
        const std::shared_ptr< MutualApproximationWithImpactParameterScaling > mutualApproximationScaler,
        const std::vector< std::vector< std::shared_ptr< observation_partials::LightTimeCorrectionPartial > > >&
        lightTimeCorrectionPartialObjects =
        std::vector< std::vector< std::shared_ptr< observation_partials::LightTimeCorrectionPartial > > >( ) );

//! Function to generate mutual approximation with impact parameter partial wrt a single  parameter.
/*!
 *  Function to generate mutual approximation with impact parameter partial wrt a single  parameter, for a single link ends (which must contain a
 *  transmitter, transmitter2 and receiever linkEndType).
 *  \tparam ParameterType Type of parameter (double for size 1, VectorXd for larger size).
 *  \param mutualApproximationLinkEnds Link ends (transmitter, transmitter2 and receiever) for which mutual approximation partials are to be
 *  calculated (i.e. for which  mutual approximation observations are to be processed).
 *  \param bodyMap List of all bodies, for creating mutual approximation partial.
 *  \param parameterToEstimate Object of current parameter that is to be estimated.
 *  \param mutualApproximationScaler Object scale position partials to mutual approximation with impact parameter partials for current link ends.
 *  \param lightTimeCorrectionPartialObjects List of light time correction partials to be used (empty by default)
 *  \return Mutual approximation with impact parameter partial object wrt a single parameter (is nullptr if no parameter dependency exists).
 */
template< typename ParameterType >
std::shared_ptr< MutualApproximationWithImpactParameterPartial > createMutualApproximationWithImpactParameterPartialWrtParameter(
        const observation_models::LinkEnds mutualApproximationLinkEnds,
        const simulation_setup::NamedBodyMap& bodyMap,
        const std::shared_ptr< estimatable_parameters::EstimatableParameter< ParameterType > > parameterToEstimate,
        const std::shared_ptr< MutualApproximationWithImpactParameterScaling > mutualApproximationScaler,
        const std::vector< std::vector< std::shared_ptr< observation_partials::LightTimeCorrectionPartial > > >&
        lightTimeCorrectionPartialObjects =
        std::vector< std::vector< std::shared_ptr< observation_partials::LightTimeCorrectionPartial > > >( ) )
{
    std::shared_ptr< MutualApproximationWithImpactParameterPartial > mutualApproximationPartial;

    {
        std::map< observation_models::LinkEndType, std::shared_ptr< CartesianStatePartial > > positionPartials =
                createCartesianStatePartialsWrtParameter( mutualApproximationLinkEnds, bodyMap, parameterToEstimate );

        std::shared_ptr< MutualApproximationWithImpactParameterPartial > testMutualApproximationPartial =
                std::make_shared< MutualApproximationWithImpactParameterPartial >(
                    mutualApproximationScaler, positionPartials, parameterToEstimate->getParameterName( ),
                    lightTimeCorrectionPartialObjects );

        // Create mutual approximation partials if any position partials are created (i.e. if any dependency exists).
        if( positionPartials.size( ) > 0 || testMutualApproximationPartial->getNumberOfLighTimeCorrectionPartialsFunctions( ) )
        {
            mutualApproximationPartial = testMutualApproximationPartial;
        }
    }
    return mutualApproximationPartial;
}

//! Function to generate mutual approximation with impact parameter partials and associated scaler for single link end.
/*!
 *  Function to generate mutual approximation with impact parameter partials and associated scaler for all parameters that are to be estimated,
 *  for a single link ends.
 *  The set of parameters and bodies that are to be estimated, as well as the set of link ends
 *  (each of which must contain a transmitter, transmitter2 and receiver linkEndType) that are to be used.
 *  \param mutualApproximationLinkEnds Link ends (transmitter, transmitter2 and receiver) for which mutual approximation partials are to be
 *  calculated (i.e. for which mutual approximation observations are to be processed).
 *  \param bodyMap List of all bodies, for creating mutual approximation partials.
 *  \param parametersToEstimate Set of parameters that are to be estimated (in addition to initial states of
 *  requested bodies)
 *  \param lightTimeCorrections List of light time correction partials to be used (empty by default)
 *  \return Set of observation partials with associated indices in complete vector of parameters that are estimated,
 *  representing all  necessary mutual approximation partials of a single link end, and MutualApproximationWithImpactParameterScaling, object, used for
 *  scaling the position partial members of all MutualApproximationWithImpactParameterPartials in link end.
 */
template< typename ParameterType >
std::pair< std::map< std::pair< int, int >, std::shared_ptr< ObservationPartial< 2 > > >,
std::shared_ptr< PositionPartialScaling > >
createMutualApproximationWithImpactParameterPartials(
        const observation_models::LinkEnds mutualApproximationLinkEnds,
        const simulation_setup::NamedBodyMap& bodyMap,
        const std::shared_ptr< estimatable_parameters::EstimatableParameterSet< ParameterType > > parametersToEstimate,
        const std::vector< std::vector< std::shared_ptr< observation_models::LightTimeCorrection > > >& lightTimeCorrections =
        std::vector< std::vector< std::shared_ptr< observation_models::LightTimeCorrection > > >( ),
        const std::shared_ptr< propagators::DependentVariablesInterface > dependentVariablesInterface
                = std::shared_ptr< propagators::DependentVariablesInterface >( ) )

{
    std::vector< std::vector< std::shared_ptr< observation_partials::LightTimeCorrectionPartial > > > lightTimeCorrectionPartialObjects;
    if( lightTimeCorrections.size( ) > 0 )
    {
        if( lightTimeCorrections.size( ) != 2 )
        {
            throw std::runtime_error( "Error when making mutual approximation partials, light time corrections for "
                                      + std::to_string( lightTimeCorrections.size( ) ) + " links found, instead of 2.");
        }
        lightTimeCorrectionPartialObjects.push_back(
                    observation_partials::createLightTimeCorrectionPartials( lightTimeCorrections[ 0 ] ) );
        lightTimeCorrectionPartialObjects.push_back(
                    observation_partials::createLightTimeCorrectionPartials( lightTimeCorrections[ 1 ] ) );
    }

    // Create scaling object, to be used for all mutual approximation partials in current link end.
    std::shared_ptr< MutualApproximationWithImpactParameterScaling > mutualApproximationScaling
            = std::make_shared< MutualApproximationWithImpactParameterScaling >( dependentVariablesInterface );


    SingleLinkObservationTwoPartialList mutualApproximationPartials;


    // Initialize vector index variables.
    int currentIndex = 0;
    std::pair< int, int > currentPair = std::pair< int, int >( currentIndex, 1 );

    std::vector< std::shared_ptr< estimatable_parameters::EstimatableParameter<
            Eigen::Matrix< ParameterType, Eigen::Dynamic, 1 > > > > initialDynamicalParameters =
            parametersToEstimate->getEstimatedInitialStateParameters( );

    // Iterate over list of bodies of which the partials of the accelerations acting on them are required.
    for( unsigned int i = 0; i < initialDynamicalParameters.size( ); i++ )
    {

        std::string acceleratedBody;
        if( initialDynamicalParameters.at( i )->getParameterName( ).first == estimatable_parameters::initial_body_state )
        {
            acceleratedBody = initialDynamicalParameters.at( i )->getParameterName( ).second.first;
        }
        else if( initialDynamicalParameters.at( i )->getParameterName( ).first == estimatable_parameters::arc_wise_initial_body_state )
        {
            acceleratedBody = initialDynamicalParameters.at( i )->getParameterName( ).second.first;
        }
        else
        {
            throw std::runtime_error( "Error when making mutual approximation partials, could not identify parameter" );
        }

        // Create position mutual approximation partial for current body
        std::shared_ptr< MutualApproximationWithImpactParameterPartial > currentMutualApproximationPartial =
                createMutualApproximationWithImpactParameterPartialWrtBodyPosition(
                    mutualApproximationLinkEnds, bodyMap, acceleratedBody, mutualApproximationScaling,
                    lightTimeCorrectionPartialObjects );

        // Check if partial is non-nullptr (i.e. whether dependency exists between current mutual approximation and current body)
        if( currentMutualApproximationPartial != nullptr )
        {
            // Add partial to the list.
            currentPair = std::pair< int, int >( currentIndex, 6 );
            mutualApproximationPartials[ currentPair ] = currentMutualApproximationPartial;
        }

        // Increment current index by size of body initial state (6).
        currentIndex += 6;
    }

    // Iterate over all double parameters that are to be estimated.
    std::map< int, std::shared_ptr< estimatable_parameters::EstimatableParameter< double > > > doubleParametersToEstimate =
            parametersToEstimate->getDoubleParameters( );
    for( std::map< int, std::shared_ptr< estimatable_parameters::EstimatableParameter< double > > >::iterator
         parameterIterator = doubleParametersToEstimate.begin( );
         parameterIterator != doubleParametersToEstimate.end( ); parameterIterator++ )
    {
        // Create position mutual approximation partial for current parameter
        std::shared_ptr< MutualApproximationWithImpactParameterPartial > currentMutualApproximationPartial =
                createMutualApproximationWithImpactParameterPartialWrtParameter( mutualApproximationLinkEnds, bodyMap, parameterIterator->second,
                                                                                 mutualApproximationScaling, lightTimeCorrectionPartialObjects );

        if( currentMutualApproximationPartial != nullptr )
        {
            // Add partial to the list.
            currentPair = std::pair< int, int >( parameterIterator->first, 1 );
            mutualApproximationPartials[ currentPair ] = currentMutualApproximationPartial;
        }
    }

    // Iterate over all vector parameters that are to be estimated.
    std::map< int, std::shared_ptr< estimatable_parameters::EstimatableParameter< Eigen::VectorXd > > >
            vectorParametersToEstimate = parametersToEstimate->getVectorParameters( );
    for( std::map< int, std::shared_ptr< estimatable_parameters::EstimatableParameter< Eigen::VectorXd  > > >::iterator
         parameterIterator = vectorParametersToEstimate.begin( );
         parameterIterator != vectorParametersToEstimate.end( ); parameterIterator++ )
    {

        // Create position mutual approximation partial for current parameter
        std::shared_ptr< ObservationPartial< 2 > > currentMutualApproximationPartial;

        if( !isParameterObservationLinkProperty( parameterIterator->second->getParameterName( ).first )  )
        {
            currentMutualApproximationPartial = createMutualApproximationWithImpactParameterPartialWrtParameter(
                        mutualApproximationLinkEnds, bodyMap, parameterIterator->second, mutualApproximationScaling,
                        lightTimeCorrectionPartialObjects );
        }
        else
        {
            currentMutualApproximationPartial = createObservationPartialWrtLinkProperty< 2 >(
                        mutualApproximationLinkEnds, observation_models::mutual_approximation_with_impact_parameter, parameterIterator->second );
        }

        // Check if partial is non-nullptr (i.e. whether dependency exists between current observable and current parameter)
        if( currentMutualApproximationPartial != nullptr )
        {
            // Add partial to the list.
            currentPair = std::pair< int, int >( parameterIterator->first,
                                                 parameterIterator->second->getParameterSize( ) );
            mutualApproximationPartials[ currentPair ] = currentMutualApproximationPartial;
        }

    }
    return std::make_pair( mutualApproximationPartials, mutualApproximationScaling );
}

//! Function to generate mutual approximation with impact parameter partials for all parameters that are to be estimated, for all sets of link ends.
/*!
 *  Function to generate mutual approximation with impact parameter partials for all parameters that are to be estimated, for all sets of link ends.
 *  The mutual approximation partials are generated per set of link ends. The set of parameters and bodies that are to be
 *  estimated, as well as the set of link ends (each of which must contain a transmitter, transmitter2 and receiever linkEndType)
 *  that are to be used.
 *  \param linkEnds Vector of all link ends for which mutual approximation partials are to be calculated (i.e. for which one-way
 *  mutual approximation observations are  to be processed).
 *  \param bodyMap List of all bodies, for creating mutual approximation partials.
 *  \param parametersToEstimate Set of parameters that are to be estimated (in addition to initial states
 *  of requested bodies)
 *  \param lightTimeCorrections List of light time correction partials to be used (empty by default)
 *  \return Map of SingleLinkObservationTwoPartialList, representing all necessary mutual approximation partials of a single link
 *  end, and MutualApproximationScaling, object, used for scaling the position partial members of all MutualApproximationWithImpactParameterPartials in
 *  link end.
 */
template< typename ParameterType >
std::map< observation_models::LinkEnds, std::pair< SingleLinkObservationTwoPartialList,
std::shared_ptr< PositionPartialScaling > > >
createMutualApproximationWithImpactParameterPartials(
        const std::vector< observation_models::LinkEnds > linkEnds,
        const simulation_setup::NamedBodyMap& bodyMap,
        const std::shared_ptr< estimatable_parameters::EstimatableParameterSet< ParameterType > > parametersToEstimate,
        const std::map< observation_models::LinkEnds,
        std::vector< std::vector< std::shared_ptr< observation_models::LightTimeCorrection > > > >& lightTimeCorrections =
        std::map< observation_models::LinkEnds, std::vector< std::vector< std::shared_ptr< observation_models::LightTimeCorrection > > > >( ),
        const std::shared_ptr< propagators::DependentVariablesInterface > dependentVariablesInterface
                = std::shared_ptr< propagators::DependentVariablesInterface >( ) )
{
    // Declare return list.
    std::map< observation_models::LinkEnds, std::pair< SingleLinkObservationTwoPartialList,
            std::shared_ptr< PositionPartialScaling > > > mutualApproximationPartials;

    // Iterate over all link ends.
    for( unsigned int i = 0; i < linkEnds.size( ); i++ )
    {
        // Check if required link end types are present
        if( ( linkEnds[ i ].count( observation_models::receiver ) == 0 ) ||
                ( linkEnds[ i ].count( observation_models::transmitter ) == 0 ) ||
                ( linkEnds[ i ].count( observation_models::transmitter2 ) == 0 ) )
        {
            throw std::runtime_error( "Error when making mutual approximation with impact parameter partials, "
                                      "did not find both transmitter, transmitter2 and receiver in link ends" );

        }

        std::vector< std::vector< std::shared_ptr< observation_models::LightTimeCorrection > > > currentLightTimeCorrections;
        if( lightTimeCorrections.count( linkEnds.at( i ) ) > 0 )
        {
            if( lightTimeCorrections.at( linkEnds.at( i ) ).size( ) != 2 )
            {
                std::cerr << "Error when making mutual approximation with impact parameter partials, light time corrections for " <<
                           lightTimeCorrections.at( linkEnds.at( i ) ).size( ) << " links found, instead of 2." << std::endl;
            }
            currentLightTimeCorrections = lightTimeCorrections.at( linkEnds.at( i ) );
        }
        else
        {
            currentLightTimeCorrections.clear( );
        }

        // Create mutual approximation partials for current link ends
        mutualApproximationPartials[ linkEnds[ i ] ] = createMutualApproximationWithImpactParameterPartials(
                    linkEnds[ i ], bodyMap, parametersToEstimate, currentLightTimeCorrections, dependentVariablesInterface );
    }
    return mutualApproximationPartials;
}



//! Function to generate impact parameter (for mutual approximation) partial wrt a position of a body.
/*!
 *  Function to generate impact parameter (for mutual approximation) wrt a position of a body, for a single link ends (which must contain a
 *  transmitter and receiever  linkEndType).
 *  \param impactParameterLinkEnds Link ends (transmitter, transmitter2 and receiever) for which partials are to be calculated
 *  (i.e. for which impact parameter observations are to be processed).
 *  \param bodyMap List of all bodies, for creating impact parameter partial.
 *  \param bodyToEstimate Name of body wrt position of which a partial is to be created.
 *  \param impactParameterScaler Object scale position partials to impact parameter partials for current link ends.
 *  \param lightTimeCorrectionPartialObjects List of light time correction partials to be used (empty by default)
 *  \return Impact parameter (for mutual approximation) partial object wrt a current position of a body (is nullptr if no parameter dependency exists).
 */
std::shared_ptr< ImpactParameterMutualApproxPartial > createImpactParameterMutualApproxPartialWrtBodyPosition(
        const observation_models::LinkEnds impactParameterLinkEnds,
        const simulation_setup::NamedBodyMap& bodyMap,
        const std::string bodyToEstimate,
        const std::shared_ptr< ImpactParameterMutualApproxScaling > impactParameterScaler,
        const std::vector< std::vector< std::shared_ptr< observation_partials::LightTimeCorrectionPartial > > >&
        lightTimeCorrectionPartialObjects =
        std::vector< std::vector< std::shared_ptr< observation_partials::LightTimeCorrectionPartial > > >( ) );

//! Function to generate impact parameter (for mutual approximation) partial wrt a single  parameter.
/*!
 *  Function to generate impact parameter (for mutual approximation) partial wrt a single  parameter, for a single link ends (which must contain a
 *  transmitter, transmitter2 and receiever linkEndType).
 *  \tparam ParameterType Type of parameter (double for size 1, VectorXd for larger size).
 *  \param impactParameterLinkEnds Link ends (transmitter, transmitter2 and receiever) for which impact parameter partials are to be
 *  calculated (i.e. for which  impact parameter observations are to be processed).
 *  \param bodyMap List of all bodies, for creating impact parameter partial.
 *  \param parameterToEstimate Object of current parameter that is to be estimated.
 *  \param impactParameterScaler Object scale position partials to impact parameter partials for current link ends.
 *  \param lightTimeCorrectionPartialObjects List of light time correction partials to be used (empty by default)
 *  \return Impact parameter (for mutual approximation) partial object wrt a single parameter (is nullptr if no parameter dependency exists).
 */
template< typename ParameterType >
std::shared_ptr< ImpactParameterMutualApproxPartial > createImpactParameterMutualApproxPartialWrtParameter(
        const observation_models::LinkEnds impactParameterLinkEnds,
        const simulation_setup::NamedBodyMap& bodyMap,
        const std::shared_ptr< estimatable_parameters::EstimatableParameter< ParameterType > > parameterToEstimate,
        const std::shared_ptr< ImpactParameterMutualApproxScaling > impactParameterScaler,
        const std::vector< std::vector< std::shared_ptr< observation_partials::LightTimeCorrectionPartial > > >&
        lightTimeCorrectionPartialObjects =
        std::vector< std::vector< std::shared_ptr< observation_partials::LightTimeCorrectionPartial > > >( ) )
{
    std::shared_ptr< ImpactParameterMutualApproxPartial > impactParameterPartial;

    {
        std::map< observation_models::LinkEndType, std::shared_ptr< CartesianStatePartial > > positionPartials =
                createCartesianStatePartialsWrtParameter( impactParameterLinkEnds, bodyMap, parameterToEstimate );

        std::shared_ptr< ImpactParameterMutualApproxPartial > testImpactParameterPartial = std::make_shared< ImpactParameterMutualApproxPartial >(
                    impactParameterScaler, positionPartials, parameterToEstimate->getParameterName( ), lightTimeCorrectionPartialObjects );

        // Create mutual approximation partials if any position partials are created (i.e. if any dependency exists).
        if( positionPartials.size( ) > 0 || testImpactParameterPartial->getNumberOfLighTimeCorrectionPartialsFunctions( ) )
        {
            impactParameterPartial = testImpactParameterPartial;
        }
    }
    return impactParameterPartial;
}

//! Function to generate impact parameter (for mutual approximation) partials and associated scaler for single link end.
/*!
 *  Function to generate impact parameter (for mutual approximation) partials and associated scaler for all parameters that are to be estimated,
 *  for a single link ends.
 *  The set of parameters and bodies that are to be estimated, as well as the set of link ends
 *  (each of which must contain a transmitter, transmitter2 and receiver linkEndType) that are to be used.
 *  \param impactParameterLinkEnds Link ends (transmitter, transmitter2 and receiver) for which impact parameter partials are to be
 *  calculated (i.e. for which impact parameter observations are to be processed).
 *  \param bodyMap List of all bodies, for creating impact parameter partials.
 *  \param parametersToEstimate Set of parameters that are to be estimated (in addition to initial states of
 *  requested bodies)
 *  \param lightTimeCorrections List of light time correction partials to be used (empty by default)
 *  \return Set of observation partials with associated indices in complete vector of parameters that are estimated,
 *  representing all  necessary impact parameter partials of a single link end, and ImpactParameterMutualApproxScaling, object, used for
 *  scaling the position partial members of all ImpactParameterMutualApproxPartials in link end.
 */
template< typename ParameterType >
std::pair< std::map< std::pair< int, int >, std::shared_ptr< ObservationPartial< 1 > > >,
std::shared_ptr< PositionPartialScaling > >
createImpactParameterMutualApproxPartials(
        const observation_models::LinkEnds impactParameterLinkEnds,
        const simulation_setup::NamedBodyMap& bodyMap,
        const std::shared_ptr< estimatable_parameters::EstimatableParameterSet< ParameterType > > parametersToEstimate,
        const std::vector< std::vector< std::shared_ptr< observation_models::LightTimeCorrection > > >& lightTimeCorrections =
        std::vector< std::vector< std::shared_ptr< observation_models::LightTimeCorrection > > >( ),
        const std::shared_ptr< propagators::DependentVariablesInterface > dependentVariablesInterface
                = std::shared_ptr< propagators::DependentVariablesInterface >( ) )

{
    std::vector< std::vector< std::shared_ptr< observation_partials::LightTimeCorrectionPartial > > > lightTimeCorrectionPartialObjects;
    if( lightTimeCorrections.size( ) > 0 )
    {
        if( lightTimeCorrections.size( ) != 2 )
        {
            throw std::runtime_error( "Error when making impact parameter (for mutual approximation) partials, light time corrections for "
                                      + std::to_string( lightTimeCorrections.size( ) ) + " links found, instead of 2.");
        }
        lightTimeCorrectionPartialObjects.push_back(
                    observation_partials::createLightTimeCorrectionPartials( lightTimeCorrections[ 0 ] ) );
        lightTimeCorrectionPartialObjects.push_back(
                    observation_partials::createLightTimeCorrectionPartials( lightTimeCorrections[ 1 ] ) );
    }

    // Create scaling object, to be used for all impact parameter partials in current link end.
    std::shared_ptr< ImpactParameterMutualApproxScaling > impactParameterScaling = std::make_shared< ImpactParameterMutualApproxScaling >( dependentVariablesInterface );

    SingleLinkObservationPartialList impactParameterPartials;


    // Initialize vector index variables.
    int currentIndex = 0;
    std::pair< int, int > currentPair = std::pair< int, int >( currentIndex, 1 );

    std::vector< std::shared_ptr< estimatable_parameters::EstimatableParameter<
            Eigen::Matrix< ParameterType, Eigen::Dynamic, 1 > > > > initialDynamicalParameters =
            parametersToEstimate->getEstimatedInitialStateParameters( );

    // Iterate over list of bodies of which the partials of the accelerations acting on them are required.
    for( unsigned int i = 0; i < initialDynamicalParameters.size( ); i++ )
    {

        std::string acceleratedBody;
        if( initialDynamicalParameters.at( i )->getParameterName( ).first == estimatable_parameters::initial_body_state )
        {
            acceleratedBody = initialDynamicalParameters.at( i )->getParameterName( ).second.first;
        }
        else if( initialDynamicalParameters.at( i )->getParameterName( ).first == estimatable_parameters::arc_wise_initial_body_state )
        {
            acceleratedBody = initialDynamicalParameters.at( i )->getParameterName( ).second.first;
        }
        else
        {
            throw std::runtime_error( "Error when making impact parameter (for mutual approximation) partials, could not identify parameter" );
        }

        // Create position impact parameter (for mutual approximation) partial for current body
        std::shared_ptr< ImpactParameterMutualApproxPartial > currentImpactParameterPartial = createImpactParameterMutualApproxPartialWrtBodyPosition(
                    impactParameterLinkEnds, bodyMap, acceleratedBody, impactParameterScaling,
                    lightTimeCorrectionPartialObjects );

        // Check if partial is non-nullptr (i.e. whether dependency exists between current mutual approximation and current body)
        if( currentImpactParameterPartial != nullptr )
        {
            // Add partial to the list.
            currentPair = std::pair< int, int >( currentIndex, 6 );
            impactParameterPartials[ currentPair ] = currentImpactParameterPartial;
        }

        // Increment current index by size of body initial state (6).
        currentIndex += 6;
    }

    // Iterate over all double parameters that are to be estimated.
    std::map< int, std::shared_ptr< estimatable_parameters::EstimatableParameter< double > > > doubleParametersToEstimate =
            parametersToEstimate->getDoubleParameters( );
    for( std::map< int, std::shared_ptr< estimatable_parameters::EstimatableParameter< double > > >::iterator
         parameterIterator = doubleParametersToEstimate.begin( );
         parameterIterator != doubleParametersToEstimate.end( ); parameterIterator++ )
    {
        // Create position impact parameter (for mutual approximation) partial for current parameter
        std::shared_ptr< ImpactParameterMutualApproxPartial > currentImpactParameterMutualApproxPartial = createImpactParameterMutualApproxPartialWrtParameter(
                    impactParameterLinkEnds, bodyMap, parameterIterator->second, impactParameterScaling,
                    lightTimeCorrectionPartialObjects );

        if( currentImpactParameterMutualApproxPartial != nullptr )
        {
            // Add partial to the list.
            currentPair = std::pair< int, int >( parameterIterator->first, 1 );
            impactParameterPartials[ currentPair ] = currentImpactParameterMutualApproxPartial;
        }
    }

    // Iterate over all vector parameters that are to be estimated.
    std::map< int, std::shared_ptr< estimatable_parameters::EstimatableParameter< Eigen::VectorXd > > >
            vectorParametersToEstimate = parametersToEstimate->getVectorParameters( );
    for( std::map< int, std::shared_ptr< estimatable_parameters::EstimatableParameter< Eigen::VectorXd  > > >::iterator
         parameterIterator = vectorParametersToEstimate.begin( );
         parameterIterator != vectorParametersToEstimate.end( ); parameterIterator++ )
    {

        // Create position impact parameter (for mutual approximation) partial for current parameter
        std::shared_ptr< ObservationPartial< 1 > > currentImpactParameterMutualApproxPartial;

        if( !isParameterObservationLinkProperty( parameterIterator->second->getParameterName( ).first )  )
        {
            currentImpactParameterMutualApproxPartial = createImpactParameterMutualApproxPartialWrtParameter(
                        impactParameterLinkEnds, bodyMap, parameterIterator->second, impactParameterScaling,
                        lightTimeCorrectionPartialObjects );
        }
        else
        {
            currentImpactParameterMutualApproxPartial = createObservationPartialWrtLinkProperty< 1 >(
                        impactParameterLinkEnds, observation_models::impact_parameter_mutual_approx, parameterIterator->second );
        }

        // Check if partial is non-nullptr (i.e. whether dependency exists between current observable and current parameter)
        if( currentImpactParameterMutualApproxPartial != nullptr )
        {
            // Add partial to the list.
            currentPair = std::pair< int, int >( parameterIterator->first,
                                                 parameterIterator->second->getParameterSize( ) );
            impactParameterPartials[ currentPair ] = currentImpactParameterMutualApproxPartial;
        }

    }
    return std::make_pair( impactParameterPartials, impactParameterScaling );
}

//! Function to generate impact parameter (for mutual approximation) partials for all parameters that are to be estimated, for all sets of link ends.
/*!
 *  Function to generate impact parameter (for mutual approximation) partials for all parameters that are to be estimated, for all sets of link ends.
 *  The impact parameter partials are generated per set of link ends. The set of parameters and bodies that are to be
 *  estimated, as well as the set of link ends (each of which must contain a transmitter, transmitter2 and receiever linkEndType)
 *  that are to be used.
 *  \param linkEnds Vector of all link ends for which impact parameter partials are to be calculated (i.e. for which one-way
 *  impact parameter observations are  to be processed).
 *  \param bodyMap List of all bodies, for creating impact parameter partials.
 *  \param parametersToEstimate Set of parameters that are to be estimated (in addition to initial states
 *  of requested bodies)
 *  \param lightTimeCorrections List of light time correction partials to be used (empty by default)
 *  \return Map of SingleLinkObservationPartialList, representing all necessary impact parameter (for mutual approximation) partials of a single link
 *  end, and ImpactParameterMutualApproxScaling, object, used for scaling the position partial members of all ImpactParameterMutualApproxPartials in
 *  link end.
 */
template< typename ParameterType >
std::map< observation_models::LinkEnds, std::pair< SingleLinkObservationPartialList,
std::shared_ptr< PositionPartialScaling > > >
createImpactParameterMutualApproxPartials(
        const std::vector< observation_models::LinkEnds > linkEnds,
        const simulation_setup::NamedBodyMap& bodyMap,
        const std::shared_ptr< estimatable_parameters::EstimatableParameterSet< ParameterType > > parametersToEstimate,
        const std::map< observation_models::LinkEnds,
        std::vector< std::vector< std::shared_ptr< observation_models::LightTimeCorrection > > > >& lightTimeCorrections =
        std::map< observation_models::LinkEnds,
        std::vector< std::vector< std::shared_ptr< observation_models::LightTimeCorrection > > > >( ),
        const std::shared_ptr< propagators::DependentVariablesInterface > dependentVariablesInterface
                = std::shared_ptr< propagators::DependentVariablesInterface >( ) )
{
    // Declare return list.
    std::map< observation_models::LinkEnds, std::pair< SingleLinkObservationPartialList,
            std::shared_ptr< PositionPartialScaling > > > impactParameterPartials;

    // Iterate over all link ends.
    for( unsigned int i = 0; i < linkEnds.size( ); i++ )
    {
        // Check if required link end types are present
        if( ( linkEnds[ i ].count( observation_models::receiver ) == 0 ) ||
                ( linkEnds[ i ].count( observation_models::transmitter ) == 0 ) ||
                ( linkEnds[ i ].count( observation_models::transmitter2 ) == 0 ) )
        {
            throw std::runtime_error( "Error when making impact parameter (for mutual approximation) partials, did not find both transmitter, transmitter2 and receiver in link ends" );

        }

        std::vector< std::vector< std::shared_ptr< observation_models::LightTimeCorrection > > > currentLightTimeCorrections;
        if( lightTimeCorrections.count( linkEnds.at( i ) ) > 0 )
        {
            if( lightTimeCorrections.at( linkEnds.at( i ) ).size( ) != 2 )
            {
                std::cerr << "Error when making impact parameter (for mutual approximation) partials, light time corrections for " <<
                           lightTimeCorrections.at( linkEnds.at( i ) ).size( ) << " links found, instead of 2." << std::endl;
            }
            currentLightTimeCorrections = lightTimeCorrections.at( linkEnds.at( i ) );
        }
        else
        {
            currentLightTimeCorrections.clear( );
        }

        // Create impact parameter (for mutual approximation) partials for current link ends
        impactParameterPartials[ linkEnds[ i ] ] = createImpactParameterMutualApproxPartials(
                    linkEnds[ i ], bodyMap, parametersToEstimate, currentLightTimeCorrections, dependentVariablesInterface );
    }
    return impactParameterPartials;
}



}

}

#endif // TUDAT_CREATEMUTUALAPPROXIMATIONPARTIALS_H