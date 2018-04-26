/*    Copyright (c) 2010-2018, Delft University of Technology
 *    All rigths reserved
 *
 *    This file is part of the Tudat. Redistribution and use in source and
 *    binary forms, with or without modification, are permitted exclusively
 *    under the terms of the Modified BSD license. You should have received
 *    a copy of the license with this file. If not, please or visit:
 *    http://tudat.tudelft.nl/LICENSE.
 *
 */

#include <boost/make_shared.hpp>
#include <iostream>
#include "Tudat/InputOutput/matrixTextFileReader.h"

#include "Tudat/Astrodynamics/Aerodynamics/tabulatedAtmosphere.h"

namespace tudat
{
namespace aerodynamics
{

//! Initialize atmosphere table reader.
void TabulatedAtmosphere::initialize( const std::map< int, std::string >& atmosphereTableFile )
{
    // Locally store the atmosphere table file name.
    atmosphereTableFile_ = atmosphereTableFile;

    // Retrieve number of dependent variables from user.
    unsigned int numberOfDependentVariables = dependentVariables_.size( );
    // consistency with number of files is checked in readTabulatedAtmosphere function

    // Check input consistency
    if ( independentVariables_.size( ) != 1 )
    {
        if ( atmosphereTableFile_.size( ) != numberOfDependentVariables )
        {
            throw std::runtime_error( "Error when creating tabulated atmosphere from file, "
                                      "number of specified dependent variables differs from file." );
        }

        // Retrieve number of independent variables from file.
        numberOfIndependentVariables_ = input_output::getNumberOfIndependentVariablesInCoefficientFile(
                    atmosphereTableFile_.at( 0 ) );

        // Check number of independent variables
        if ( ( numberOfIndependentVariables_ < 1 ) || ( numberOfIndependentVariables_ > 4 ) )
        {
            throw std::runtime_error( "Error when reading tabulated atmosphere from file, found " +
                                      std::to_string( numberOfIndependentVariables_ ) +
                                      " independent variables, up to 4 currently supported." );
        }
        // Could also check to make sure that no duplicate (in)dependent variables are input

        // Check input consistency
        if ( static_cast< int >( independentVariables_.size( ) ) != numberOfIndependentVariables_ )
        {
            throw std::runtime_error( "Error when creating tabulated atmosphere from file, "
                                      "number of specified independent variables differs from file." );
        }
    }
    else
    {
        numberOfIndependentVariables_ = 1; // if only one independent variable is specified, only one file will
                                           // be provided, and it cannot be opened with the same function
    }

    // Get order of dependent variables
    for ( unsigned int i = 0; i < numberOfDependentVariables; i++ )
    {
        if ( i <= dependentVariableIndices_.size( ) )
        {
            dependentVariableIndices_.at( dependentVariables_.at( i ) ) = i;
            dependentVariablesDependency_.at( dependentVariables_.at( i ) ) = true;
        }
        else
        {
            std::string errorMessage = "Error, dependent variable " + std::to_string( dependentVariables_.at( i ) ) +
                    " not found in tabulated atmosphere.";
            throw std::runtime_error( errorMessage );
        }
    }

    // Check that density, pressure and temperature are present
    if ( !( dependentVariablesDependency_.at( 0 ) || dependentVariablesDependency_.at( 1 ) ||
            dependentVariablesDependency_.at( 2 ) ) )
    {
        throw std::runtime_error( "Error, tabulated atmosphere must be initialized with at least "
                                  "density, pressure and temperature." );
    }

    using namespace interpolators;

    // Create interpolators for variables requested by users, depending on the number of variables
    switch ( numberOfIndependentVariables_ )
    {
    case 1:
    {
        // Call approriate file reading function for 1 independent variables
        Eigen::MatrixXd tabulatedAtmosphereData = input_output::readMatrixFromFile(
                    atmosphereTableFile_.at( 0 ), " \t", "%" );
        unsigned int numberOfColumnsInFile = tabulatedAtmosphereData.cols( );
        unsigned int numberOfRowsInFile = tabulatedAtmosphereData.rows( );

        // Check whether data is present in the file.
        if ( numberOfRowsInFile < 1 || numberOfColumnsInFile < 1 )
        {
            std::string errorMessage = "The atmosphere table file " + atmosphereTableFile_.at( 0 ) + " is empty";
            throw std::runtime_error( errorMessage );
        }

        // Check
        if ( numberOfDependentVariables != ( numberOfColumnsInFile - 1 ) )
        {
            throw std::runtime_error( "Number of specified dependent variables does not match file." );
        }

        // Assign sizes to vectors
        independentVariablesData_.resize( numberOfIndependentVariables_ );
        std::vector< std::vector< double > > dependentVariablesData;
        dependentVariablesData.resize( numberOfDependentVariables );

        // Extract variables from file
        for ( unsigned int i = 0; i < numberOfRowsInFile; i++ )
        {
            independentVariablesData_.at( 0 ).push_back( tabulatedAtmosphereData( i, 0 ) );
            for ( unsigned int j = 0; j < dependentVariablesDependency_.size( ); j++ )
            {
                if ( dependentVariablesDependency_.at( j ) )
                {
                    dependentVariablesData.at( dependentVariableIndices_.at( j ) ).push_back(
                                tabulatedAtmosphereData( i, dependentVariableIndices_.at( j ) + 1 ) );
                }
            }
        }

        // Create interpolators for density, pressure and temperature
        interpolationForDensity_ = boost::make_shared< CubicSplineInterpolatorDouble >(
                    independentVariablesData_.at( 0 ), dependentVariablesData.at( dependentVariableIndices_.at( 0 ) ) );
        interpolationForPressure_ = boost::make_shared< CubicSplineInterpolatorDouble >(
                    independentVariablesData_.at( 0 ), dependentVariablesData.at( dependentVariableIndices_.at( 1 ) ) );
        interpolationForTemperature_ = boost::make_shared< CubicSplineInterpolatorDouble >(
                    independentVariablesData_.at( 0 ), dependentVariablesData.at( dependentVariableIndices_.at( 2 ) ) );

        // Create remaining interpolators, if requested by user
        if ( dependentVariablesDependency_.at( 3 ) )
        {
            interpolationForGasConstant_ = boost::make_shared< CubicSplineInterpolatorDouble >(
                        independentVariablesData_.at( 0 ), dependentVariablesData.at( 3 ) );
        }
        if ( dependentVariablesDependency_.at( 4 ) )
        {
            interpolationForSpecificHeatRatio_ = boost::make_shared< CubicSplineInterpolatorDouble >(
                        independentVariablesData_.at( 0 ), dependentVariablesData.at( 4 ) );
        }
        break;
    }
    case 2:
    {
        createMultiDimensionalAtmosphereInterpolators< 2 >( );
        break;
    }
    case 3:
    {
        createMultiDimensionalAtmosphereInterpolators< 3 >( );
        break;
    }
    case 4:
    {
        throw std::runtime_error( "Currently, only three independent variables are supported." );
        break;
    }
    }
}

//! Initialize atmosphere table reader.
template< int NumberOfIndependentVariables >
void TabulatedAtmosphere::createMultiDimensionalAtmosphereInterpolators( )
{
    using namespace interpolators;

    // Call approriate file reading function for N independent variables
    std::pair< std::vector< boost::multi_array< double, static_cast< size_t >( NumberOfIndependentVariables ) > >,
            std::vector< std::vector< double > > > tabulatedAtmosphereData;

    // Extract data
    tabulatedAtmosphereData = input_output::readTabulatedAtmosphere< NumberOfIndependentVariables >( atmosphereTableFile_ );

    // Assign independent variables
    independentVariablesData_ = tabulatedAtmosphereData.second;

    // Assign dependent variables
    interpolationForDensity_ =
            boost::make_shared< MultiLinearInterpolator< double, double, NumberOfIndependentVariables > >(
                independentVariablesData_, tabulatedAtmosphereData.first.at( dependentVariableIndices_.at( 0 ) ) );
    interpolationForPressure_ =
            boost::make_shared< MultiLinearInterpolator< double, double, NumberOfIndependentVariables > >(
                independentVariablesData_, tabulatedAtmosphereData.first.at( dependentVariableIndices_.at( 1 ) ) );
    interpolationForTemperature_ =
            boost::make_shared< MultiLinearInterpolator< double, double, NumberOfIndependentVariables > >(
                independentVariablesData_, tabulatedAtmosphereData.first.at( dependentVariableIndices_.at( 2 ) ) );
    if ( dependentVariablesDependency_.at( 3 ) )
    {
        interpolationForGasConstant_ =
                boost::make_shared< MultiLinearInterpolator< double, double, NumberOfIndependentVariables > >(
                    independentVariablesData_, tabulatedAtmosphereData.first.at( dependentVariableIndices_.at( 3 ) ) );
    }
    if ( dependentVariablesDependency_.at( 4 ) )
    {
        interpolationForSpecificHeatRatio_ =
                boost::make_shared< MultiLinearInterpolator< double, double, NumberOfIndependentVariables > >(
                    independentVariablesData_, tabulatedAtmosphereData.first.at( dependentVariableIndices_.at( 4 ) ) );
    }
}

} // namespace aerodynamics
} // namespace tudat
