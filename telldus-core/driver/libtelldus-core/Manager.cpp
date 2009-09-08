//
// C++ Implementation: controller
//
// Description: 
//
//
// Author: Micke Prag <micke.prag@telldus.se>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Manager.h"
#include "Device.h"

#include "DeviceBrateck.h"
#include "DeviceGroup.h"
#include "DeviceNexa.h"
#include "DeviceRisingSun.h"
#include "DeviceWaveman.h"
#include "DeviceSartano.h"
#include "DeviceIkea.h"
#include "DeviceUndefined.h"
#include "DeviceUpm.h"
#include "DeviceX10.h"

#include "Controller.h"
#ifdef TELLSTICK_DUO
#include "TellStickDuo.h"
#endif
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WINDOWS
#define strcasecmp _stricmp
#endif


using namespace TelldusCore;

Manager *Manager::instance = 0;

Manager::Manager()
	: lastCallbackId(0)
{
#ifdef TELLSTICK_DUO
	Controller *controller = new TellStickDuo("TSQVB5HU");
	controllers[1] = controller;
#endif
}

Manager::~Manager() {
	// Clean up the device-map
	for (DeviceMap::iterator it = devices.begin(); it != devices.end(); ++it) {
		delete( it->second );
	}
	// Clean up the controller-map
	for (ControllerMap::iterator it = controllers.begin(); it != controllers.end(); ++it) {
		delete( it->second );
	}
}

/**
 * Get the requested device
 * Note that the Manager keeps ownership of the returned Device
 * and should not be deleted when not in use anymore.
 **/
Device *Manager::getDevice(int intDeviceId){
	Device* dev = NULL;
	
	DeviceMap::iterator iterator = devices.find(intDeviceId);
	if (iterator != devices.end()) {
		return iterator->second;
	}

	try{
		std::string protocol = settings.getProtocol(intDeviceId);
		std::string strModel = settings.getModel(intDeviceId);
		std::string strName = settings.getName(intDeviceId);
		
		//Strip anything after : if it is found
		size_t pos = strModel.find(":");
		if (pos != std::string::npos) {	
			strModel = strModel.substr(0, pos);
		}

		//each new brand must be added here
		if (strcasecmp(protocol.c_str(), "arctech") == 0){
			dev = new DeviceNexa(intDeviceId, strModel, strName);
			((DeviceNexa*)dev)->setHouse(settings.getDeviceParameter(intDeviceId, "house"));
			((DeviceNexa*)dev)->setUnit(settings.getDeviceParameter(intDeviceId, "unit"));
	
		} else if (strcasecmp(protocol.c_str(), "brateck") == 0) {
			dev = new DeviceBrateck(intDeviceId, strModel, strName);
			((DeviceBrateck*)dev)->setHouse(settings.getDeviceParameter(intDeviceId, "house"));

		} else if (strcasecmp(protocol.c_str(), "group") == 0) {
			dev = new DeviceGroup(intDeviceId, strModel, strName);
			((DeviceGroup*)dev)->setDevices(settings.getDeviceParameter(intDeviceId, "devices"));
			
		} else if (strcasecmp(protocol.c_str(), "risingsun") == 0) {
			dev = new DeviceRisingSun(intDeviceId, strModel, strName);
			((DeviceRisingSun*)dev)->setHouse(settings.getDeviceParameter(intDeviceId, "house"));
			((DeviceRisingSun*)dev)->setUnit(settings.getDeviceParameter(intDeviceId, "unit"));

		} else if (strcasecmp(protocol.c_str(), "Waveman") == 0) {
			dev = new DeviceWaveman(intDeviceId, strModel, strName);
			((DeviceWaveman*)dev)->setHouse(settings.getDeviceParameter(intDeviceId, "house"));
			((DeviceWaveman*)dev)->setUnit(settings.getDeviceParameter(intDeviceId, "unit"));

		} else if (strcasecmp(protocol.c_str(), "Sartano") == 0) {
			dev = new DeviceSartano(intDeviceId, strModel, strName);
			((DeviceSartano*)dev)->setCode(settings.getDeviceParameter(intDeviceId, "code"));

		} else if (strcasecmp(protocol.c_str(), "Ikea") == 0) {
			dev = new DeviceIkea(intDeviceId, strModel, strName);
			((DeviceIkea*)dev)->setSystem(settings.getDeviceParameter(intDeviceId, "system"));
			((DeviceIkea*)dev)->setUnits(settings.getDeviceParameter(intDeviceId, "units"));
			((DeviceIkea*)dev)->setFade(settings.getDeviceParameter(intDeviceId, "fade"));

		} else if (strcasecmp(protocol.c_str(), "upm") == 0) {
			dev = new DeviceUpm(intDeviceId, strModel, strName);
			((DeviceUpm*)dev)->setHouse(settings.getDeviceParameter(intDeviceId, "house"));
			((DeviceUpm*)dev)->setUnit(settings.getDeviceParameter(intDeviceId, "unit"));
		
		} else if (strcasecmp(protocol.c_str(), "x10") == 0) {
			dev = new DeviceX10(intDeviceId, strModel, strName);
			((DeviceX10*)dev)->setHouse(settings.getDeviceParameter(intDeviceId, "house"));
			((DeviceX10*)dev)->setUnit(settings.getDeviceParameter(intDeviceId, "unit"));
		
		} else {
			//This is a dummy device needed when the parameters isn't setup correctly.
			dev = new DeviceUndefined(intDeviceId, strModel, strName);
		}

#ifdef _LINUX
		dev->setDevice( settings.getSetting("deviceNode") );
#endif

	}
	catch(...){
		throw;
	}
	
	if (intDeviceId > 0) {
		devices[intDeviceId] = dev;
	}
	return dev;
}

int Manager::getNumberOfDevices(void) const {
	return settings.getNumberOfDevices();
}

int Manager::getDeviceId(int intDeviceIndex) const {
	return settings.getDeviceId(intDeviceIndex);
}

void Manager::loadAllDevices() {
	int numberOfDevices = getNumberOfDevices();
	for (int i = 0; i < numberOfDevices; ++i) {
		int id = settings.getDeviceId(i);
		if (!deviceLoaded(id)) {
			getDevice(id);
		}
	}
}

bool Manager::setDeviceProtocol(int intDeviceId, const std::string &strProtocol) {
	bool retval = settings.setProtocol( intDeviceId, strProtocol );
	
	// Delete the device to reload it when the protocol changes
	DeviceMap::iterator iterator = devices.find(intDeviceId);
	if (iterator != devices.end()) {
		Device *device = iterator->second;
		devices.erase( iterator );
		delete device;
	}
	
	return retval;
}

bool Manager::setDeviceModel(int intDeviceId, const std::string &strModel) {
	return settings.setModel(intDeviceId, strModel);
}

bool Manager::setDeviceState( int intDeviceId, int intDeviceState, const std::string &strDeviceStateValue ) {
	if (intDeviceState != TELLSTICK_BELL &&
	    intDeviceState != TELLSTICK_LEARN
		 ) {
		bool retval = settings.setDeviceState(intDeviceId, intDeviceState, strDeviceStateValue);
		for(CallbackList::const_iterator callback_it = callbacks.begin(); callback_it != callbacks.end(); ++callback_it) {
			(*callback_it).event(intDeviceId, intDeviceState, wrapStdString(strDeviceStateValue), (*callback_it).id, (*callback_it).context);
		}
		
		return retval;
	}
	return true;
}

int Manager::getDeviceState( int intDeviceId ) const {
	return settings.getDeviceState(intDeviceId);
}

std::string Manager::getDeviceStateValue( int intDeviceId ) const {
	return settings.getDeviceStateValue(intDeviceId);
}

bool Manager::deviceLoaded(int deviceId) const {
	DeviceMap::const_iterator iterator = devices.find(deviceId);
	if (iterator == devices.end()) {
		return false;
	}
	return true;
}

void Manager::parseMessage( const std::string &message ) {
	loadAllDevices(); //Make sure all devices is loaded before we iterator the list.
	
	std::map<std::string, std::string> parameters;
	std::string protocol;
	int method = 0;
	
	size_t prevPos = 0;
	size_t pos = message.find(";");
	while(pos != std::string::npos) {
		std::string param = message.substr(prevPos, pos-prevPos);
		prevPos = pos+1;
		size_t delim = param.find(":");
		if (delim == std::string::npos) {
			break;
		}
		if (param.substr(0, delim).compare("protocol") == 0) {
			protocol = param.substr(delim+1, param.length()-delim);
		} else if (param.substr(0, delim).compare("method") == 0) {
			method = Device::methodId(param.substr(delim+1, param.length()-delim));
		} else {
			parameters[param.substr(0, delim)] = param.substr(delim+1, param.length()-delim);
		}
		pos = message.find(";", pos+1);
	}
	for (DeviceMap::const_iterator it = devices.begin(); it != devices.end(); ++it) {
		if (it->second->getProtocol().compare(protocol) != 0) {
			continue;
		}
		if (! (it->second->methods() & method)) {
			continue;
		}
		bool found = true;
		for (std::map<std::string, std::string>::const_iterator p_it = parameters.begin(); p_it != parameters.end(); ++p_it) {
			if (!it->second->parameterMatches(p_it->first, p_it->second)) {
				found = false;
				break;
			}
		}
		if (found) {
			//First save the last sent command, this also triggers the callback to the client
			setDeviceState(it->first, method, "");
		}
	}

	for(RawCallbackList::const_iterator it = rawCallbacks.begin(); it != rawCallbacks.end(); ++it) {
		(*it).event(message.c_str(), (*it).id, (*it).context);
	}
}

int Manager::registerDeviceEvent( TDDeviceEvent eventFunction, void *context ) {
	int id = ++lastCallbackId;
	CallbackStruct callback = {eventFunction, id, context};
	callbacks.push_back(callback);
	return id;
}

int Manager::registerRawDeviceEvent( TDRawDeviceEvent eventFunction, void *context ) {
	int id = ++lastCallbackId;
	RawCallbackStruct callback = {eventFunction, id, context};
	rawCallbacks.push_back(callback);
	return id;
}

Manager *Manager::getInstance() {
	if (Manager::instance == 0) {
		Manager::instance = new Manager();
	}
	return Manager::instance;
}

void Manager::close() {
	if (Manager::instance != 0) {
		delete Manager::instance;
	}
}

bool TelldusCore::Manager::setDeviceParameter(int intDeviceId, const std::string & strName, const std::string & strValue) {
	return settings.setDeviceParameter(intDeviceId, strName, strValue);
}

std::string TelldusCore::Manager::getDeviceParameter(int intDeviceId, const std::string & strName) const {
	return settings.getDeviceParameter( intDeviceId, strName );
}

int TelldusCore::Manager::addDevice() {
	return settings.addDevice();
}

bool TelldusCore::Manager::removeDevice(int intDeviceId) {
	if (deviceLoaded(intDeviceId)) {
		DeviceMap::iterator iterator = devices.find(intDeviceId);
		if (iterator == devices.end()) { // Should not be possible since deviceLoaded() returned true
			return false;
		}
		Device *dev = iterator->second;
		devices.erase(iterator);
		delete dev;
	}
	
	return settings.removeDevice(intDeviceId);
}

bool TelldusCore::Manager::setDeviceName(int intDeviceId, const std::string & strNewName) {
	return settings.setName(intDeviceId, strNewName);
}

