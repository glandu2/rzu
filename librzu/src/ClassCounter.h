#ifndef CLASSCOUNTER_H
#define CLASSCOUNTER_H

#include "RappelzLib_global.h"

//Provide a counter for each classes so each instance has an unique name based on it's class name

//Contain class count as we instanciate them. Class which are not instanciated are not counted.
//This provide a way to convert a class type to an integer hash.
//this var is implemented in ClassCounter.cpp
extern unsigned int RAPPELZLIB_EXTERN ClassCounter__internal_object_engine_class_counter;

#define DECLARE_CLASSCOUNT_STATIC(classname) \
	unsigned long classname::ClassCounter::objectCount = 0; \
	unsigned long classname::ClassCounter::objectsCreated = 0; \
	const unsigned int classname::ClassCounter::classTypeHash = ClassCounter__internal_object_engine_class_counter++;

#endif // CLASSCOUNTER_H
