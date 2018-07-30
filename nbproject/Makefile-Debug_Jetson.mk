#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug_Jetson
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/e1dda48/appsettings.o \
	${OBJECTDIR}/_ext/e1dda48/basicserial.o \
	${OBJECTDIR}/_ext/e1dda48/camera_base.o \
	${OBJECTDIR}/_ext/e1dda48/camera_file.o \
	${OBJECTDIR}/_ext/e1dda48/camera_usb.o \
	${OBJECTDIR}/_ext/e1dda48/error_base.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=`pkg-config --libs opencv` /home/nvidia/ml/darknet/darknet.so `pkg-config --libs libserialport` `pkg-config --libs jsoncpp`  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/videotracker

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/videotracker: /home/nvidia/ml/darknet/darknet.so

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/videotracker: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	g++ -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/videotracker ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/e1dda48/appsettings.o: ../libterraclear/src/appsettings.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/e1dda48
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/nvidia/ml `pkg-config --cflags opencv` `pkg-config --cflags libserialport` `pkg-config --cflags jsoncpp` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/e1dda48/appsettings.o ../libterraclear/src/appsettings.cpp

${OBJECTDIR}/_ext/e1dda48/basicserial.o: ../libterraclear/src/basicserial.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/e1dda48
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/nvidia/ml `pkg-config --cflags opencv` `pkg-config --cflags libserialport` `pkg-config --cflags jsoncpp` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/e1dda48/basicserial.o ../libterraclear/src/basicserial.cpp

${OBJECTDIR}/_ext/e1dda48/camera_base.o: ../libterraclear/src/camera_base.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/e1dda48
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/nvidia/ml `pkg-config --cflags opencv` `pkg-config --cflags libserialport` `pkg-config --cflags jsoncpp` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/e1dda48/camera_base.o ../libterraclear/src/camera_base.cpp

${OBJECTDIR}/_ext/e1dda48/camera_file.o: ../libterraclear/src/camera_file.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/e1dda48
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/nvidia/ml `pkg-config --cflags opencv` `pkg-config --cflags libserialport` `pkg-config --cflags jsoncpp` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/e1dda48/camera_file.o ../libterraclear/src/camera_file.cpp

${OBJECTDIR}/_ext/e1dda48/camera_usb.o: ../libterraclear/src/camera_usb.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/e1dda48
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/nvidia/ml `pkg-config --cflags opencv` `pkg-config --cflags libserialport` `pkg-config --cflags jsoncpp` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/e1dda48/camera_usb.o ../libterraclear/src/camera_usb.cpp

${OBJECTDIR}/_ext/e1dda48/error_base.o: ../libterraclear/src/error_base.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/e1dda48
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/nvidia/ml `pkg-config --cflags opencv` `pkg-config --cflags libserialport` `pkg-config --cflags jsoncpp` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/e1dda48/error_base.o ../libterraclear/src/error_base.cpp

${OBJECTDIR}/main.o: main.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/home/nvidia/ml `pkg-config --cflags opencv` `pkg-config --cflags libserialport` `pkg-config --cflags jsoncpp` -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} -r ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/darknet.so
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/videotracker

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
