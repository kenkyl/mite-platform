#include "SensorMessage.h"

SensorMessage::SensorMessage(){
}

SensorMessage::SensorMessage(char* idP, char* labelP, char* typeP, char* valueP){
	&id = idP;
	label = labelP;
	type = typeP;
	value = valueP; 
}

void SensorMessage::setId(char* idP){
	id = idP; 
}

void SensorMessage::setLabel(char* labelP){
	label = labelP; 
}

void SensorMessage::setType(char* typeP){
	type = typeP; 
}

void SensorMessage::setValue(char* valueP){
	value = valueP; 
}