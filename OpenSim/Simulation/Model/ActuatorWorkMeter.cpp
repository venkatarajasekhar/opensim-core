// ActuatorWorkMeter.cpp
// Author: Frank C. Anderson, Ajay Seth
/*
 * Copyright (c)  2006, Stanford University. All rights reserved. 
* Use of the OpenSim software in source form is permitted provided that the following
* conditions are met:
* 	1. The software is used only for non-commercial research and education. It may not
*     be used in relation to any commercial activity.
* 	2. The software is not distributed or redistributed.  Software distribution is allowed 
*     only through https://simtk.org/home/opensim.
* 	3. Use of the OpenSim software or derivatives must be acknowledged in all publications,
*      presentations, or documents describing work in which OpenSim or derivatives are used.
* 	4. Credits to developers may not be removed from executables
*     created from modifications of the source.
* 	5. Modifications of source code must retain the above copyright notice, this list of
*     conditions and the following disclaimer. 
* 
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
*  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
*  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
*  SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
*  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
*  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR BUSINESS INTERRUPTION) OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
*  WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//=============================================================================
// INCLUDES
//=============================================================================
#include "ActuatorWorkMeter.h"
#include <OpenSim/Simulation/Model/Model.h>
#include <OpenSim/Simulation/Model/BodySet.h>

//=============================================================================
// STATICS
//=============================================================================
using namespace std;
//using namespace SimTK;
using namespace OpenSim;

static const string WORK_STATE_NAME = "work";

//=============================================================================
// CONSTRUCTOR(S) AND DESTRUCTOR
//=============================================================================
//_____________________________________________________________________________
/**
 * Default constructor.
 */
ActuatorWorkMeter::ActuatorWorkMeter() 
{
	setNull();
	setupProperties();
}

//_____________________________________________________________________________
/**
 * Convenience constructor.
 */
ActuatorWorkMeter::ActuatorWorkMeter(const Actuator &actuator, double initialWork)
{
	setNull();
	setupProperties();
	_actuatorNameProp.setValue(actuator.getName());
	_initialWorkProp.setValue(initialWork);
}

//_____________________________________________________________________________
/**
 * Destructor.
 */
ActuatorWorkMeter::~ActuatorWorkMeter()
{
}

//_____________________________________________________________________________
/**
 * Copy constructor.
 *
 * @param aActuatorWorkMeter ActuatorWorkMeter to be copied.
 */
ActuatorWorkMeter::ActuatorWorkMeter(const ActuatorWorkMeter &aActuatorWorkMeter) :
   ModelComponent(aActuatorWorkMeter)
{
	setNull();
	setupProperties();
}

//=============================================================================
// CONSTRUCTION METHODS
//=============================================================================


//_____________________________________________________________________________
/**
 * Set the data members of this ActuatorWorkMeter to their null values.
 */
void ActuatorWorkMeter::setNull(void)
{
	setType("ActuatorWorkMeter");
}

//_____________________________________________________________________________
/**
 * Connect properties to local pointers.
 */
void ActuatorWorkMeter::setupProperties(void)
{
	_actuatorNameProp.setComment("The actuator name whos work use will be calculated.");
	_actuatorNameProp.setName("actuator_name");
	_actuatorNameProp.setValue("Unassigned");
	_propertySet.append(&_actuatorNameProp);

	_initialWorkProp.setComment("The initial amount of work.");
	_initialWorkProp.setName("initial_actuator_work");
	_initialWorkProp.setValue(0.0);
	_propertySet.append(&_initialWorkProp);
}

Object* ActuatorWorkMeter::copy() const
{
	ActuatorWorkMeter *meter = new ActuatorWorkMeter(*this);
	return(meter);
}

//_____________________________________________________________________________
/**
 * Perform some set up functions that happen after the
 * object has been deserialized or copied.
 *
 * @param aModel OpenSim model containing this ActuatorWorkMeter.
 */
void ActuatorWorkMeter::setup(Model& aModel)
{
	const string& actName = _actuatorNameProp.getValueStr();
	ModelComponent::setup(aModel);
	int k = _model->getActuators().getIndex(actName);
	if( k >=0 )
		_actuator = &_model->getActuators().get(k);
	else{
		string errorMessage = "ActuatorWorkMeter: Invalid actuator '" + actName + "' specified in Actuator.";
		throw (Exception(errorMessage.c_str()));
	}
}

//=============================================================================
// Create the underlying system component(s)
//=============================================================================
void ActuatorWorkMeter::createSystem(SimTK::MultibodySystem& system) const
{
	ModelComponent::createSystem(system);
	
	ActuatorWorkMeter* mutableThis = const_cast<ActuatorWorkMeter *>(this);

	// Assign a name to the state variable to access the work value stored in the state
	Array<string> stateVariables(_actuator->getName()+"."+WORK_STATE_NAME, 1);

	// Add state variables to the underlying system
	mutableThis->addStateVariables(stateVariables);
}


//=============================================================================
// The state variable derivative (power) to be integrated
//=============================================================================
SimTK::Vector ActuatorWorkMeter::computeStateVariableDerivatives(const SimTK::State& s) const
{
	SimTK::Vector derivs(1, _actuator->getPower(s));
	double force = _actuator->getForce(s);
	double speed = _actuator->getSpeed(s);
	double power = derivs[0];

	return derivs;
}

 void ActuatorWorkMeter::initState( SimTK::State& s) const
{
	setStateVariable(s, getStateVariableNames()[0], _initialWorkProp.getValueDbl());
}

void ActuatorWorkMeter::setDefaultsFromState(const SimTK::State& state)
{
    _initialWorkProp.setValue(getWork(state));
}

//=============================================================================
// OPERATORS
//=============================================================================
//_____________________________________________________________________________
/**
 * Assignment operator.
 *
 * @return Reference to this object.
 */
ActuatorWorkMeter& ActuatorWorkMeter::operator=(const ActuatorWorkMeter &aActuatorWorkMeter)
{
	// BASE CLASS
	Object::operator=(aActuatorWorkMeter);
	return(*this);
}

//=============================================================================
// GET AND SET
//=============================================================================
//
// Computed work is part of the state
double ActuatorWorkMeter::getWork(const SimTK::State& state) const
{
	return getStateVariable(state, _actuator->getName()+"."+WORK_STATE_NAME);
}

